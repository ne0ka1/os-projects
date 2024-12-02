#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc == 1){
        printf("wzip: file1 [file2 ...]\n");
        exit(1);
    }

    FILE *fp;
    int count = 0; // character count
    int curr, prev = EOF; // current and previous character

    for (int i = 1; i < argc; i++){
        if ((fp = fopen(argv[i], "r")) == NULL){
            printf("wzip: cannot open file\n");
            exit(1);
        }
        
        while ((curr = fgetc(fp)) != EOF) {
            if (curr != prev && prev != EOF) {
                fwrite(&count, sizeof(int), 1, stdout);
                fwrite(&prev, sizeof(char), 1, stdout);
                count = 0;
            }
            prev = curr;
            count++;
        }

        fclose(fp);
    }

    if (count > 0) {
        fwrite(&count, sizeof(int), 1, stdout);
        fwrite(&prev, sizeof(char), 1, stdout);
    }

    exit(0);
}
