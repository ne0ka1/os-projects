#include <stdio.h>
#include <stdlib.h> 
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef struct file {
    int fd;
    off_t size;
} File;

typedef struct chunk {
    long long size;
    char *address;
} Chunk;

long long nchunks = 0;
sem_t mutex, empty, full;

void *compress(void *arg);

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
        if ((int fd = open(argv[i + 1], O_RDONLY)) == -1) {
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

    for (int i = 0; i < nthreads; i++) {
        pthread_create(&threads[i], NULL, compress, chunks);
    }

    // produce chunks by memory mapping files
    long long produce_ptr = 0;
    for (int i = 0; i < argc - 1; i++) {
        long long pos = 0;
        while (pos < files[i].size) {
            chunks[produce_ptr].size = page_size;
            if (pos + page_size > files[i].size) {
                chunks[produce_ptr].size = page_size;
            } else {
                chunks[produce_ptr].size = files[i].size - pos;
            }

            // TODO fill address

        }
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(files);
    free(chunks);
    exit(EXIT_SUCCESS);
}

void *compress(void *arg) {
    // do compress work here
}
