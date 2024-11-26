#include <stdio.h>
#include <stdlib.h>

#define MAXLINE 250

int main(int argc, char *argv[])
{
    if (argc == 1){
        exit(0);
    }

    FILE *fp;
    int cnt = 0;
    char line[MAXLINE];
    
    while (--argc > 0){
        if ((fp = fopen(argv[++cnt], "r")) == NULL){
            printf("wcat: cannot open file\n");
            exit(1);
        }
        
        while (fgets(line, MAXLINE, fp) != NULL){
            printf("%s", line);
        }

        fclose(fp);
    }

    exit(0);
}
