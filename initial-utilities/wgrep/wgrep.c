#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* search term in file *fp, and print the result to stdout */
int search(FILE *fp, char *term);

int main(int argc, char *argv[])
{
    if (argc == 1){
        printf("wgrep: searchterm [file ...]\n");
        exit(1);
    }

    char *term = argv[1];

    if (argc == 2){
        search(stdin, term);
    } else {
        FILE *fp;
        int cnt = 1;
        while (--argc > 1){
            if ((fp = fopen(argv[++cnt], "r")) == NULL){
                printf("wgrep: cannot open file\n");
                exit(1);
            }
            search(fp, term);
            fclose(fp);
        }
    }

    exit(0);
}

int search(FILE *fp, char *term){
    char *line = NULL;
    size_t len = 0;
    size_t nread;

    while ((nread = getline(&line, &len, fp)) != -1){
        if (strstr(line, term) != NULL){
            printf("%s", line);
        }
    }

    free(line);
    return 0;
}
