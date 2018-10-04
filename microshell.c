
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);
int sh_exec(char **args);

int is_wait = 1; 

char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "exec"
};

int (*builtin_func[]) (char **) = {
    &sh_cd,
    &sh_help,
    &sh_exit,
    &sh_exec
};

int sh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int sh_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "sh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("sh");
        }
    }
    return 1;
}

int sh_help(char **args)
{
    int i;
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < sh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int sh_exit(char **args)
{
    return 0;
}

int sh_launch(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("sh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("sh");
    } else {
        // Parent process
        do {
            if(is_wait) { 
                waitpid(pid, &status, WUNTRACED);
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int sh_exec(char **args) { 
    if (args[1] == NULL) {
        fprintf(stderr, "sh: expected argument to \"exec\"\n");
    } else {
        args++; 
        sh_launch(args); 
    }
    return 0;
}

int sh_execute(char **args)
{
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < sh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return sh_launch(args);
}

#define sh_RL_BUFSIZE 1024
char *sh_read_line(void) {
    int bufsize = sh_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        if (c == EOF) {
            exit(EXIT_SUCCESS);
        } else if (c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        if (position >= bufsize) {
            bufsize += sh_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "sh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define sh_TOK_BUFSIZE 64
#define sh_TOK_DELIM " \t\r\n\a"


char **sh_split_line(char *line)
{
    int bufsize = sh_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, **tokens_backup;

    if (!tokens) {
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, sh_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += sh_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
		        free(tokens_backup);
                fprintf(stderr, "sh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, sh_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

void sh_get_out() { 
    char c; 
    printf("\nAre you to exit ? (y/n): ");
    scanf("%c", &c);
    if ( c == 'y' || c == 'Y' ) { 
        exit(EXIT_SUCCESS);
    } 
}

void sh_type_prompt(void){
    int bufsize = sh_RL_BUFSIZE;
    char *buffer = malloc(sizeof(char) * bufsize);
    getlogin_r(buffer,bufsize); 
    
    char cwd[sh_RL_BUFSIZE];
    getcwd(cwd, sizeof(cwd));

    printf(CYAN "%s:" GREEN " %s:" RESET "$  ", buffer,cwd);
}

void sh_signal(void) { 
    struct sigaction act;
    act.sa_handler = sh_get_out;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);                             //Ctrl+C
    sigaction(SIGTSTP, &act, 0);                            //Ctrl+Z
}

void sh_loop(void) {
    char *line;
    char **args;
    int status;
    
    do {

        sh_type_prompt();
        line = sh_read_line();
        args = sh_split_line(line);
        status = sh_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv)
{
    sh_signal(); 
    sh_loop();
    return EXIT_SUCCESS;
}

