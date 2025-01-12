#include <stdio.h>
#include <stdlib.h> 
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

#include <pthread.h>
#include <semaphore.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef struct file {
    int fd;
    off_t size;
} File;

typedef struct output {
    struct output *next;
    int count;
    char character;
} Output;

typedef struct chunk {
    long long size;
    char *addr;
    Output *outputs;
} Chunk;

long long nchunks = 0;
long long consumer_ptr = 0;
sem_t mutex, empty, full;

void *compress(void *arg);
void process_outputs(Chunk *chunks);

int main(int argc, char *argv[])
{
    if (argc == 1){
        fprintf(stdout, "pzip: file1 [file2 ...]\n");
        exit(EXIT_FAILURE);
    }

    // allocate threads and files
    long page_size = sysconf(_SC_PAGE_SIZE);
    int nthreads = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t *threads = malloc(sizeof(pthread_t) * nthreads);
    if (threads == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    File *files = malloc(sizeof(File) * (argc - 1));
    if (files == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // open files and get nchunks
    for (int i = 0; i < argc - 1; i++) {
        int fd = open(argv[i + 1], O_RDONLY);
        if (fd == -1) {
            perror("open");
            continue;
        }
        files[i].fd = fd;

        struct stat st;
        if (fstat(fd, &st) == -1) {
            perror("fstat");
            close(fd);
            continue;
        }
        files[i].size = st.st_size;

        nchunks += (st.st_size / page_size + 1);
    }

    // initialize chunks
    Chunk *chunks = malloc(sizeof(Chunk) * nchunks);
    if (chunks == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // init semaphores
    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, 1);
    sem_init(&full, 0, 0);

    // create threads
    for (int i = 0; i < nthreads; i++) {
        pthread_create(&threads[i], NULL, compress, chunks);
    }

    // produce chunks by memory mapping files
    long long producer_ptr = 0;
    for (int i = 0; i < argc - 1; i++) {
        long long pos = 0;
        while (pos < files[i].size) {
            sem_wait(&empty);
            sem_wait(&mutex);

            chunks[producer_ptr].size = page_size;
            if (pos + page_size > files[i].size) {
                chunks[producer_ptr].size = page_size;
            } else {
                chunks[producer_ptr].size = files[i].size - pos;
            }

            char *addr = mmap(NULL, (size_t)chunks[producer_ptr].size, PROT_READ, MAP_PRIVATE, files[i].fd, pos);
            if (addr == MAP_FAILED) {
                perror("mmap");
                exit(EXIT_FAILURE);
            }

            pos += page_size;
            chunks[producer_ptr].addr = addr;
            chunks[producer_ptr].outputs = NULL;
            producer_ptr = (producer_ptr + 1) % nchunks;

            sem_post(&mutex);
            sem_post(&full);
        }
        close(files[i].fd);
    }

    // check compressions done
    sem_wait(&empty);
    sem_wait(&mutex);

    for (int i = 0; i < nthreads; i++) {
        pthread_cancel(threads[i]);
        pthread_join(threads[i], NULL);
    }

    // write outputs
    void process_outputs(Chunk* chunks);
    sem_post(&mutex);

    // clean up 
    free(threads);
    free(files);
    free(chunks);
    sem_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty);

    exit(EXIT_SUCCESS);
}

Output* make_output(int count, char character) {
    Output *output = malloc(sizeof(output));
    if (output == NULL) {
        perror("malloc output");
        exit(EXIT_FAILURE);
    }
    output->count = count;
    output->character = character;
    output->next = NULL;
    return output;
}

void *compress(void *arg) {
    Chunk *chunks = (Chunk *)arg;

    while (1){
        sem_wait(&full);
        sem_wait(&mutex);

        Chunk *curr_chunk = &chunks[consumer_ptr];
        consumer_ptr = (consumer_ptr + 1) & nchunks;

        Output *head = NULL;
        Output *prev_output = NULL;
        char prev_character = '\0';
        int prev_count = 0;

        for (long long i = 0; i < curr_chunk->size; i++) {
            char character = curr_chunk->addr[i];
            if (character == prev_character) {
                prev_count++;
            } else {
                if (prev_count != 0) {
                    Output *last_output = make_output(prev_count, prev_character);
                    if (prev_output != NULL) {
                        prev_output->next = last_output;
                    }
                    prev_output = last_output;
                    if (head == NULL) {
                        head = prev_output;
                    }
                }
                prev_count = 1;
                prev_character = character;
            }
        }

        if (head == NULL) {
            // same character all through in the chunk
            curr_chunk->outputs = make_output(prev_count, prev_character);
        } else {
            curr_chunk->outputs = head;
            prev_output->next = make_output(prev_count, prev_character);
        }

        sem_post(&mutex);
        sem_post(&empty);
    }
}

void write_file(int count, char *character) {
    fwrite(&count, sizeof(int), 1, stdout);
    fwrite(character, sizeof(char), 1, stdout);
}

void process_first_output(Output *curr_output, int *last_count, char *last_character) {
    if (curr_output->character == *last_character) {
        // same as last character
        write_file(curr_output->count + *last_count, &curr_output->character);
    } else {
        // not same as last characte
        if (*last_count > 0) {
            write_file(*last_count, last_character);
        }
        write_file(curr_output->count, &curr_output->character);
    }
}

void process_last_output(Output *curr_output, int *last_count, char *last_character, Chunk *chunks, int index) {
    if (curr_output != chunks[index].outputs) {
        // not first output
        if (index != nchunks - 1) {
            // not last chunk, update last_count and last_character
            *last_count = curr_output->count;
            *last_character = curr_output->character;
        } else {
            // last chunk, just write
            write_file(curr_output->count, &curr_output->character);
        }
    } else {
        // first output
        if (curr_output->character == *last_character) {
            // same as last character
            if (index != nchunks - 1) {
                *last_count += curr_output->count;
            } else {
                write_file(curr_output->count + *last_count, &curr_output->character);
            }    
        } else {
            // not same as last character
            if (*last_count > 0) {
                write_file(*last_count, last_character);
            }
            if (index != nchunks - 1) {
                *last_character = curr_output->character;
                *last_count = curr_output->count;
            } else {
                write_file(curr_output->count, &curr_output->character);
            }
        }
    }
}


void process_outputs(Chunk *chunks){
    int *last_count = 0;
    char *last_character = '\0';

    for (long long i = 0; i < nchunks; i++) {
        Output *curr_output = chunks[i].outputs;

        while (curr_output != NULL) {
            if (curr_output == chunks[i].outputs && curr_output->next != NULL) {
                process_first_output(curr_output, last_count, last_character);
            } else if (curr_output->next == NULL) {
                process_last_output(curr_output, last_count, last_character, chunks, i);
            } else {
                write_file(curr_output->count, &curr_output->character);
            }

            Output *temp = curr_output;
            curr_output = curr_output->next;
            free(temp);
        }

        if (munmap(chunks[i].addr, (size_t)chunks[i].size) != 0) {
            perror("mumnap");
            exit(EXIT_FAILURE);
        }
    }
}
