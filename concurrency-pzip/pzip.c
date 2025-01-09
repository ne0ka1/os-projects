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
long long consume_ptr = 0;
sem_t mutex, empty, full;

void *compress(void *arg);
Output* make_output(int count, char character);

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
    long long produce_ptr = 0;
    for (int i = 0; i < argc - 1; i++) {
        long long pos = 0;
        while (pos < files[i].size) {
            sem_wait(&empty);
            sem_wait(&mutex);

            chunks[produce_ptr].size = page_size;
            if (pos + page_size > files[i].size) {
                chunks[produce_ptr].size = page_size;
            } else {
                chunks[produce_ptr].size = files[i].size - pos;
            }

            char *addr = mmap(NULL, (size_t)chunks[produce_ptr].size, PROT_READ, MAP_PRIVATE, files[i].fd, pos);
            if (addr == MAP_FAILED) {
                perror("mmap");
                exit(EXIT_FAILURE);
            }

            pos += page_size;
            chunks[produce_ptr].addr = addr;
            chunks[produce_ptr].outputs = NULL;
            produce_ptr = (produce_ptr + 1) % nchunks;

            sem_post(&mutex);
            sem_post(&full);
        }
        close(files[i].fd);
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }


    free(threads);
    free(files);
    free(chunks);
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

        Chunk *curr_chunk = &chunks[consume_ptr];
        consume_ptr = (consume_ptr + 1) & nchunks;

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
