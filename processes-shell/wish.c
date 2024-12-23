#include <stdio.h>  // fopen, fclose, getline
#include <stdlib.h> // exit
#include <string.h>
#include <unistd.h>
#include <sys/wait.h> // waitpid

#define INTERACTIVE_MODE 1
#define BATCH_MODE 0
#define BUFF_SIZE 256

struct Shell;
int execute_builtin(struct Shell *shell);
int search_path(char path[], struct Shell *shell);
void execute_command(struct Shell *shell);
void print_error();
void clean(struct Shell *shell);

// The Shell struct stores settings and current state of the shell
struct Shell {
    int mode;               // the mode of shell (interactive or batch)
    FILE* input;            // file pointer to the input stream, may be stdin or batch file
    char* line;             // current line read from pointer
    char* paths[BUFF_SIZE]; // arrays of path name
    int argc;               // argc of current command
    char* argv[BUFF_SIZE];  // argv of current command
};


int main(int argc, char *argv[])
{
    // initialize a shell
    struct Shell shell;
    shell.input = stdin;
    shell.line = NULL;
    shell.paths[0] = "/bin";
    shell.paths[1] = NULL;

    size_t linecap = 0;

    // set input stream
    if (argc < 2){
        shell.mode = INTERACTIVE_MODE;
    } else if (argc == 2){
        shell.mode = BATCH_MODE;
        if ((shell.input = fopen(argv[1], "r")) == NULL){
            print_error();
            exit(EXIT_FAILURE);
        }
    } else {
        print_error();
        exit(EXIT_FAILURE);
    }

    while (1){
        if (shell.mode == INTERACTIVE_MODE) {
            printf("wish > ");
        }

        if (getline(&(shell.line), &linecap, shell.input) == -1){
            print_error();
        }

        if (strcmp(shell.line, "\n") == 0) {
            continue;
        }

        // populate shell.argc and shell.argv
        // parse_line(line);
        shell.argc = 1;
        shell.argv[0] = "ls";
        shell.argv[1] = NULL;

        if (execute_builtin(&shell) == -1){
            execute_command(&shell);
        }
    }
    clean(&shell);
    exit(EXIT_SUCCESS);
}

// execute builtin commands: exit, cd, path
int execute_builtin(struct Shell *shell){
    if (strcmp(shell->argv[0], "exit") == 0){
        if (shell->argc != 1){
            print_error();
        } else {
            clean(shell);
            exit(EXIT_SUCCESS);
        }
    } else if (strcmp(shell->argv[0], "cd") == 0){
        if (shell->argc != 2) {
           print_error();
        }
        else if (chdir(shell->argv[1]) == -1) {
           print_error();
        }
    } else if (strcmp(shell->argv[0], "path") == 0){
        int i = 0;
        shell->paths[0] = NULL;
        for (; i < shell->argc - 1; i++){
            shell->paths[i] = strdup(shell->argv[i + 1]);
        }
        shell->paths[i + 1] = NULL;
    } else { // not built-in command
        return -1;
    }
    return 0;
}

// search executable file in paths, store the result in path
int search_path(char path[], struct Shell *shell) {
    int i = 0;
    while (shell->paths[i] != NULL){
        snprintf(path, BUFF_SIZE, "%s/%s", shell->paths[i], shell->argv[0]);
        if (access(path, X_OK) == 0){
            return 0;
            i++;
        }
        return -1;
    }
}

void execute_command(struct Shell *shell) {
    char path[BUFF_SIZE];
    if (search_path(path, shell) == 0) {
        pid_t pid = fork();
        if (pid == -1) {
            print_error();
        } else if (pid == 0) {
            // child process
            // redirect(out);
            if (execv(path, shell->argv) == -1){
                printf("execv = -1\n");
                print_error();
            } 
        } else {
            // parent process
            waitpid(pid, NULL, 0);
        } 
    } else {
        print_error(); // not in path
    }
}

void print_error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
}

void clean(struct Shell *shell) {
    free(shell->line);
    if (shell->mode == INTERACTIVE_MODE) {
        fclose(shell->input);
    }
}
