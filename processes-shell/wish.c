#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ARG_MAX 100 /* maximum number of arguments */

/* The shell_info struct stores settings and current state of the shell */
struct shell_info {
    int interactive;            /* whether the shell is running in interactive mode (1) or batch mode (0) */
    FILE* stream;              /* file pointer to the input strea, may be stdin or batch file */
    int argc;
    char* argv[ARG_MAX];
};

struct shell_info shell;

/* The command_entry struct stores the name and function of a command */
struct command_entry {
    char* name;
    int (*functionp) (int argc, char* argv[]);
};

/* cmd_exit exits; accept no argument */
int cmd_exit(int argc, char* argv[]) {
    if (argc != 1) {
        fprintf(stderr, "%s: cannot have argument.\n", argv[0]);
        return 1;
    }
    exit(0);
    return 0;
}

/* cmd_cd to be implemented */
int cmd_cd(int argc, char* argv[]){
    return 0;
}

/* cmd_path to be implemented */
int cmd_path(int argc, char* argv[]){
    return 0;
}

/* builtin commands */
struct command_entry builtin[] = {{"exit", cmd_exit},
                                  {"cd", cmd_cd},
                                  {"path", cmd_path}
};

/* initialize shell */

void initialize_shell(void)
{
    shell.argc = -1;
    shell.interactive = 1;
    shell.stream = stdin;
}

int main(int argc, char *argv[])
{
    char* line = NULL;
    size_t linecap = 0;

    initialize_shell();

    if (argc == 2){ // batch mode
        shell.interactive = 0;
        if ((shell.stream = fopen(argv[1], "r")) == NULL){
            fprintf(stderr, "wish: %s cannot be opened.\n", argv[1]);
            exit(1);
        }
    } else if (argc > 2){
        fprintf(stderr, "wish: can only have zero or one argument.\n");
        exit(1);
    }

    while (1){
        if (shell.interactive) printf("wish > ");

        if (getline(&line, &linecap, shell.stream) == -1){
            fprintf(stderr, "wish: getline failed.\n");
        }
        // replace the end newline with the null character
        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        // populate shell.argc and shell.argv
        // parse_line(line);
        shell.argc = 1;
        shell.argv[0] = "exit";

        // check builtin
        for (int i = 0; i < sizeof(builtin) / sizeof(builtin[0]); i++){
            if (strcmp(shell.argv[0], builtin[i].name) == 0){
                builtin[i].functionp(shell.argc, shell.argv);
            }
        }
    }

    free(line);
    return 0;
}
