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

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MAX_STR 256

void type_prompt(){
    printf(ANSI_COLOR_RED "\nxD " ANSI_COLOR_MAGENTA "$ " ANSI_COLOR_RESET);
}

int read_command(char *command, char *parameters[])
{
    char line[MAX_STR*MAX_STR]; 
    fgets (line, MAX_STR*MAX_STR, stdin);

    char *p = strtok(line," "); 
    strcpy(command,p); 
    int i = 0 ; 
    while ( parameters[i] = strtok(NULL, " ") ) {
        *(parameters[i] - 1) = '\0';
        i++; 
    }
    return i + 1; 
}

int main() { 
    while( 1 ) { 
        type_prompt();                                  //Mostra o prompt na tela 
        char command[MAX_STR];
        char *parameters[MAX_STR];
        int size = read_command(command, parameters);   //Le a linha de entrada do teclado
        printf("%s",parameters[size-2]); 

        pid_t pid = fork();                             //Cria um processo filho
        if( pid ) { 
            wait( NULL );                               //Pai espera o filho
        } else { 
            char *env[]= {getenv("PATH"),NULL}; 
            execve(command,parameters,env) ;            //Filho traz o trabalho
        }
    }
}

