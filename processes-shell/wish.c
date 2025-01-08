#include <stdio.h>  // fopen, fclose, getline
#include <stdlib.h> // exit
#include <string.h> // strcep, strtok, strcmp
#include <unistd.h> // fileno
#include <sys/types.h> // size_t
#include <sys/wait.h> // waitpid

#define INTERACTIVE_MODE 1
#define BATCH_MODE 0
#define BUFF_SIZE 256

typedef struct shell Shell;
void parse_line(Shell *shell);
int execute_builtin(Shell *shell);
int search_path(char path[], Shell *shell);
void redirect(Shell *shell);
void execute_command(Shell *shell);
void print_error();
void clean(Shell *shell);

// The Shell struct stores settings and current state of the shell
typedef struct shell {
    int mode;               // the mode of shell (interactive or batch)
    FILE* input;            // file pointer to the input stream
    FILE* output;           // file pointer to the output stream
    char* line;             // current line read from pointer
    char* paths[BUFF_SIZE]; // arrays of path name
    int argc;               // argc of current command
    char* argv[BUFF_SIZE];  // argv of current command
} Shell;


int main(int argc, char *argv[])
{
    // initialize a shell
    Shell shell;
    shell.input = stdin;
    // shell.output is reset for every command
    shell.line = NULL;
    shell.paths[0] = "/bin";
    shell.paths[1] = NULL;

    size_t linecap = 0;
    ssize_t nread;

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

        if ((nread = getline(&(shell.line), &linecap, shell.input)) > 0){

            // remove newline character
            if (shell.line[nread - 1] == '\n'){
                shell.line[nread - 1] = '\0';
            }
            if (shell.line[0] == '\n') {
                continue;
            } 

            parse_line(&shell);

            if (execute_builtin(&shell) == -1){
                execute_command(&shell);
            }

        } else if (feof(shell.input) != 0) {
            clean(&shell);
            exit(EXIT_SUCCESS);
        }
    }
}

// tokenizes line based on whitespace characters
void parse_line(Shell *shell){
    shell->output = stdout;
    char *command = strsep(&shell->line, ">");

    if (shell->line != NULL){
        // needs redirection
        char *saveptr0;
        char *out = strtok_r(shell->line, " \t\n", &saveptr0);
        if (!out){
            // no output file specified
            print_error();
            return;
        }
        char *more_out = strtok_r(NULL, " \t\n", &saveptr0);
        if (more_out != NULL) {
            // multiple output files
            print_error();
            return;
        } else {
            // set the output stream
            if ((shell->output = fopen(out, "w")) == NULL){
                print_error();
                return;
            }
        }
    }

    // parse command
    char *saveptr;
    char *arg = strtok_r(command, " \t\n", &saveptr);
    int i = 0;

    while (arg != NULL) {
        shell->argv[i] = arg;
        arg = strtok_r(NULL, " \t\n", &saveptr);
        i++;
    }

    shell->argv[i] = NULL;
    shell->argc = i;
}

// execute builtin commands: exit, cd, path
int execute_builtin(Shell *shell){
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
int search_path(char path[], Shell *shell) {
    int i = 0;
    while (shell->paths[i] != NULL){
        snprintf(path, BUFF_SIZE, "%s/%s", shell->paths[i], shell->argv[0]);
        if (access(path, X_OK) == 0){
            return 0;
            i++;
        }
        return -1;
    }
    return 0;
}

// redirect output to shell->output
void redirect(Shell *shell) {
    int out_fileno;
    if ((out_fileno = fileno (shell->output)) == -1) {
        print_error();
        return;
    }
    if (out_fileno != STDOUT_FILENO) {
        if (dup2(out_fileno, STDOUT_FILENO) == -1) {
            print_error();
            return;
        }
        if (dup2(out_fileno, STDERR_FILENO) == -1) {
            print_error();
            return;
        }
        fclose(shell->output);
    }
}

// execute command in path
void execute_command(Shell *shell) {
    char path[BUFF_SIZE];
    if (search_path(path, shell) == 0) {
        pid_t pid = fork();
        if (pid == -1) {
            print_error();
        } else if (pid == 0) {
            // child process
            redirect(shell);
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

void clean(Shell *shell) {
    free(shell->line);
    if (shell->mode == INTERACTIVE_MODE) {
        fclose(shell->input);
    }
}
