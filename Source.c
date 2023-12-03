#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define COMMAND_ERROR_1 "Command not found!"
#define COMMAND_ERROR_2 "Sub Argument not found!"

/*
    Funcao usada para criar ficheiro
*/
void create_file(){

}

/*
    Funcao usada para ler o input
*/
void read_command(int argc,char *argv[]){

    //Caso o 1 argumento seja top vamos redirecionalo
    if(!strcmp(argv[1],"top")){
        if(argc > 2){
            fprintf(stderr,"%s\n",COMMAND_ERROR_1);
            fprintf(stderr,"Command top format: mycmd top\n");
            exit(EXIT_FAILURE);
        }
        command_top();
        exit(EXIT_SUCCESS);
    }

    command_verify(argc,argv);

}
/*
    Funcao usada para executar o comando desejado pelo utilizador
*/
int execute_command(int argc,char *argv[],char *args[]){

    if(!strcmp(argv[1],"ls")){
        command_ls(argc,argv,args);
    }
}
/*
    Funcao para verificar se existe o comando introduzido
*/
void command_verify(int argc,char *argv[]){
    char *command_list[3] = {"ls","grep","ps"};
    char *subcommand_list[8] = {"-l","-a","-h","-r","-t","-R","-F","--color"};
    char *args[8] = {};
     
    for(int i = 0;i < 3;i++){
        //Verifica se o comando existe na lista de comandos
        if(!strcmp(command_list[i],argv[1])){
            //Se existir verificar se temos mais argumentos
            if(argc < 2){
                //Caso nao tenha mais argumentos, Mandar executar o commando
                execute_command(argc,argv,args);
                exit(EXIT_SUCCESS);
            }else{
                //Caso tenha mais subargumentos verificar se existem, Caso existam colocar em um array
                for(int r = 2;r < argc;r++){
                    int subcommand_found = 0;
                    for(int t = 0;t < 8;t++){
                        if(!strcmp(argv[r],subcommand_list[t])){
                            subcommand_found = 1;
                        }
                        args[r-2] = argv[r];
                        
                    }
                    if(!subcommand_found){
                        fprintf(stderr,"%s\n",COMMAND_ERROR_2);
                        exit(EXIT_FAILURE);  
                    }
                }
                execute_command(argc,argv,args);
                exit(EXIT_SUCCESS);
            }
        }
    }
    fprintf(stderr,"%s\n",COMMAND_ERROR_1);
    exit(EXIT_FAILURE);
}
/*
    Funcao usada quando o argumento top e escolhido
*/
void command_top(){

    double cpuload[3];

    if(getloadavg(cpuload,3) == -1){
        fprintf(stderr,"Error getting cpu load");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout,"CPU LOAD: %.2f %.2f %.2f\n",cpuload[0],cpuload[1],cpuload[2]);
    exit(EXIT_SUCCESS);
}

void command_ls(int argc,char *argv[],char *args){
    int i = 0;
    DIR *dir;
    struct dirent *entry;
    struct stat file_info;
    dir = opendir("."); // Diretorio atual

    if(!dir){
        fprintf(stderr,"Error opening directory");
        exit(EXIT_FAILURE);
    }

    if(stat(".",&file_info) == -1){
        fprintf(stderr,"Erro ao obter informacoes do ficheiro");
        exit(EXIT_FAILURE);
    }

    while((entry = readdir(dir)) != NULL){
        if((!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")) && exists(argc,argv,"-a",0) == 1){
            continue;
        }
        if(i % 5 == 0){
            
            fprintf(stdout,"\n");
            i=0;
        }
        if(exists(argc,argv,"-l",0) == 1){
            if(S_ISDIR(file_info.st_mode)){
                fprintf(stdout,"\033[1;34m%s\033[0m \t\t",entry->d_name);
            }else{
                fprintf(stdout,"\033[0;37m%s\033[0m \t\t",entry->d_name);
            }
        }
        i++;
    }
    fprintf(stdout,"\n");
}
/*
    Esta funcao serve para verificar se um argumento existe no array argv 
    Em vez de se adicionar um loop em todos as funcoes basta chamar esta funcao
    O type pode ter X valores dependendo do command_list, usando sempre a mesma ordem
*/
int exists(int argc,char *argv[],char *string,int type){
    
    if(type == 0){
        for(int i = 2; i < argc;i++){
            if(!strcmp(argv[i],string)){
                return 0;
            }
        }
    }

    return 1;
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
