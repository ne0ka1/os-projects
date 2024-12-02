#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc == 1){
        printf("wunzip: file1 [file2 ...]\n");
        exit(1);
    }

    FILE *fp;
    int count = 0; // character count
    char ch;

    for (int i = 1; i < argc; i++){
        if ((fp = fopen(argv[i], "r")) == NULL){
            printf("wunzip: cannot open file\n");
            exit(1);
        }

        while (fread(&count, sizeof(int), 1, fp) == 1 &&
               fread(&ch, sizeof(char), 1, fp) == 1) {
            for (int j = 0; j < count; j++){
                putchar(ch);
            }
        }

        fclose(fp);
    }

    exit(0);
}
