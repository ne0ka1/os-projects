#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef struct files {
    int fd;
    off_t size;
} Files;

typedef struct targs {
    // TODO targs implementation
} Targs;

void *compress(void *arg);

int main(int argc, char *argv[])
{
    if (argc == 1){
        fprintf(stdout, "pzip: file1 [file2 ...]\n");
        exit(EXIT_FAILURE);
    }

    int nthreads = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t *threads = malloc(sizeof(pthread_t) * nthreads);
    if (threads == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    Files *files = malloc(sizeof(Files) * (argc - 1));
    if (files == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < nthreads; i++) {
        // TODO targs is arguments for compress
        pthread_create(&threads[i], NULL, compress, targs);
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(files);
    exit(EXIT_SUCCESS);
}

void *compress(void *arg) {
    // do compress work here
}
