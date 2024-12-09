#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The shell_info struct stores settings and current state of the shell */
typedef struct
{
    int interactive;            /* whether the shell is running in interactive mode (1) or batch mode (0) */
    FILE * stream;              /* file pointer to the input strea, may be stdin or batch file */
} shell_info

shell_info shell;

void initialize_shell(void)
{
    shell.interactive = 1;
    shell.stream = stdin;
}

int main(int argtc, char *argv[])
{

    initialize_shell();

    if (argc == 2){
    } else if (argc == 2){
        shell.interactive = 0;
        if ((shell.stream = fopen(argv[1], 'r')) == NULL){
            fprintf(stderr, "wish: %s cannot be opened.\n", argv[1]);
            exit(1);
        }
    } else {
        fprintf(stderr, "wish: can only have zero or one argument.\n");
        exit(1);
    }

    while (1){
        if (shell.interactive) printf("wish > ");

        // getline();
    }
}
