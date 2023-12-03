#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define COMMAND_ERROR_1 "Command not found!"

/*
    Funcao usada para criar
*/
void create_file(){

}

/*
    Funcao usada para ler o input
*/
int read_command(int argc,char *argv[]){
    //Caso o 1 argumento seja top vamos redirecionalo
    if(!strcmp(argv[1],"top")){
        if(argc > 2){
            fprintf(stderr,"%s\n",COMMAND_ERROR_1);
            fprintf(stderr,"Command top format: mycmd top\n");
            return EXIT_FAILURE;
        }

        double cpuload[3];

        if(getloadavg(cpuload,3) == -1){
            fprintf(stderr,"Error getting cpu load");
            return EXIT_FAILURE;
        }

        fprintf(stdout,"CPU LOAD: %.2f %.2f %.2f\n",cpuload[0],cpuload[1],cpuload[2]);
    }
}
/*
    Funcao usada para executar o comando desejado pelo utilizador
*/
int execute_command(char *argv[]){

}
/*
    Funcao usada quando o argumento top e escolhido
*/

int main(int argc, char *argv[]){


    if(argc < 2 ){
        fprintf(stderr,"Format: mycmd <...>\n");
        fprintf(stderr,"\nmycmd --help\n");
        return EXIT_FAILURE;
    }

    read_command(argc,argv);

    return EXIT_SUCCESS;
}
