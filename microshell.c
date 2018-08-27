/*
    Comandos 
        fork
        exec 
        wait
        exit_code 
    gcc microshell.c -o micro && ./micro
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MAX_STR 256
#define EXIT "exit"

/* Funcao para tratar Ctrl-C */
void getOut() { 
    char c; 
    printf("\nDeseja sair? (y/n): ");
    scanf("%c", &c);
    if ( c == 'y' || c == 'Y' ) { 
        exit(EXIT_SUCCESS);
    } 
}

/* Imprime o prompt */
void type_prompt(){
    printf(ANSI_COLOR_RED "\nxD " ANSI_COLOR_CYAN "$ " ANSI_COLOR_RESET);
}

/* Le o comando e os parametros */
int read_command(char *command, char *parameters[])
{
    char line[MAX_STR*MAX_STR]; 
    fgets (line, MAX_STR*MAX_STR, stdin);

    if( !line ) { 
        return 0;
    }

    char *p = strtok(line," "); 
    strcpy(command,p); 
    int i = 0 ; 
    while ( parameters[i] = strtok(NULL, " ") ) {
        i++; 
    }
    return i + 1; 
}

int main(int argc, char const *argv[]) 
{ 
    /* Capturar sinal de Ctrl-C */
    struct sigaction act;
    act.sa_handler = getOut;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);                             //Ctrl+C
    sigaction(SIGTSTP, &act, 0);                            //Ctrl+Z

    /* Execucao do shell */
    while ( 1 ) { 
        type_prompt();                                      //Mostra o prompt na tela 
        char command[MAX_STR];
        char *parameters[MAX_STR];
        int size = read_command(command, parameters);       //Le a linha de entrada do teclado
 
        if (strncmp(EXIT, command, 4) == 0) {               //Comando de saida
            exit (EXIT_SUCCESS);  
        }

        pid_t pid = fork();                                 //Cria um processo filho  
        char *env[]= {getenv("PATH"),NULL};
        int stat_val; 
        switch (pid) {
            case -1:
                exit (EXIT_FAILURE);
                break;
            case 0: 
                execve(command, parameters, env) ;          //Filho traz o trabalho
                perror("execve");
                exit (EXIT_FAILURE);
                break; 
            default:
                wait( &stat_val );                          //Pai espera o filho
                break;
        }
    }
}

