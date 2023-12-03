#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

/*
    Funcao usada para criar
*/
void create_file(){

}

/*
    Funcao usada para ler o input
*/
void read_command(char **argv[]){

}
/*
    Funcao usada para executar o comando desejado pelo utilizador
*/
void execute_command(char** argv[]){

}

int main(int argc, char *argv[]){


    if(argc < 2 ){
        fprintf(stderr,"Format: mycmd <...>\n");
        fprintf(stderr,"\nmycmd --help\n");
        exit(EXIT_FAILURE);
    }

    read_command(argv);

    exit(EXIT_SUCCESS);
}
