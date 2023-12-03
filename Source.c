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
        command_top();
        return EXIT_SUCCESS;
    }

    command_verify(argc,argv);

}
/*
    Funcao usada para executar o comando desejado pelo utilizador
*/
int execute_command(int argc,char *argv[]){

}
/*
    Funcao para verificar se existe o comando introduzido
*/
int command_verify(int argc,char *argv[]){
    char *command_list[3] = {"ls","grep","ps"};
    char *subcommand_list[8] = {"-l","-a","-h","-r","-t","-R","-F","--color"};
     
    for(int i = 0;i < 3;i++){
        //Verifica se o comando existe na lista de comandos
        if(!strcmp(command_list[i],argv[1])){
            //Se existir verificar se temos mais argumentos
            if(argc < 3){
                //Caso nao tenha mais argumentos, Mandar executar o commando
                execute_command(argc,argv);
                return EXIT_SUCCESS;
            }else{
                //Caso tenha mais subargumentos verificar se existem
                for(int r = 2;r < argc;r++){
                    for(int t = 0;t < 8;t++){
                        if(strcmp(argv[r],subcommand_list[t]) != 0){
                            return EXIT_FAILURE;
                        }
                    }
                }
                execute_command(argc,argv);
            }
        }
    }
    fprintf(stderr,"%s\n",COMMAND_ERROR_1);
    return EXIT_FAILURE;
}
/*
    Funcao usada quando o argumento top e escolhido
*/
int command_top(){

    double cpuload[3];

    if(getloadavg(cpuload,3) == -1){
        fprintf(stderr,"Error getting cpu load");
        return EXIT_FAILURE;
    }

    fprintf(stdout,"CPU LOAD: %.2f %.2f %.2f\n",cpuload[0],cpuload[1],cpuload[2]);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]){


    if(argc < 2 ){
        fprintf(stderr,"Format: mycmd <...>\n");
        fprintf(stderr,"\nmycmd --help\n");
        return EXIT_FAILURE;
    }

    read_command(argc,argv);

    return EXIT_SUCCESS;
}
