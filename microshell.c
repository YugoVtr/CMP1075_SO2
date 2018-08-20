/*
    Comandos 
        fork
        exec 
        wait
        exit_code 
    gcc microshell.c -o micro && ./micro
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_command(char *command, char *parameters[])
{
    int cnt, len, i;
    gets(command);
    cnt = 0;
    parameters[0] = command;
    len = strlen(command);
    for(i=0;i<len;i++){
        if(command[i] == ' '){
                command[i] = 0;
                parameters[++cnt] = command+i+1;
            }
        }
    parameters[++cnt] = NULL;
    return cnt;
}

int main() { 
    while( 1 ) { 
        type_prompt();                     //Mostra o prompt na tela 
        char *command;
        char *parameters[5];
        read_command(command, parameters); //Le a linha de entrada do teclado

        pid_t pid = fork();                //Cria um processo filho
        if( pid ) { 
            wait( NULL );                   //Pai espera o filho
        } else { 
            execve(command, parameters, 0); //Filho traz o trabalho 
        }
    }
}

