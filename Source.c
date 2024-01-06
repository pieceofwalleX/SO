#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>

#define COMMAND_ERROR_1 "Command not found!"

typedef struct {
    int found;
    const char *operator;
    int argc_cmd1;
    char **argv_cmd1;
    int argc_cmd2;
    char **argv_cmd2;
} comands;

comands parse(char *argv[], int argc) {
    comands result;

    result.found = 0; 
    result.operator = NULL;
    result.argc_cmd1 = 0;
    result.argc_cmd2 = 0;
    result.argv_cmd1 = NULL;
    result.argv_cmd2 = NULL;

    int found = 0;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], "<") == 0 || strcmp(argv[i], "|") == 0) {
            found = 1;
            result.found = 1;
	    result.operator =  argv[i];
            continue;
        }

        if(!found) {
            // Adiciona argumentos no contexto do cmd1
            result.argv_cmd1 = realloc(result.argv_cmd1, (result.argc_cmd1 + 1) * sizeof(char *));
            result.argv_cmd1[result.argc_cmd1] = strdup(argv[i]);
            result.argc_cmd1++;
        } else {
            // Adiciona argumentos no contexto do cmd2 
            result.argv_cmd2 = realloc(result.argv_cmd2, (result.argc_cmd2 + 1) * sizeof(char *));
            result.argv_cmd2[result.argc_cmd2] = strdup(argv[i]);
            result.argc_cmd2++;
        }
    }
    result.argv_cmd1[result.argc_cmd1] = NULL;
    if (found) result.argv_cmd2[result.argc_cmd2] = NULL;


    return result;
}
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
        command_top();
    }else{
        execute_command(parse(argv,argc));
    }

    return;
}
/*
    Funcao usada para executar o comando desejado pelo utilizador
*/
int execute_command(comands input){
    char ** subarguments;

    if(!input.found){
        subarguments = input.argv_cmd1 + 1;
        execvp(input.argv_cmd1[1],subarguments);
    }else{

        execute_multicommands(input);

        // subarguments = input.argv_cmd2 + 2;
        // execvp(input.argv_cmd2[1],subarguments);
    }
    return 0;
}

int execute_multicommands(comands input){
    char ** subarguments;
    /*
        Aqui basicamente vai se criar um processo filho para executar
        os argumentos antes dos operadores >,< ou |
        Quando o processo filho finalizar o processo pai
        executa os argumentos depois dos operadores
    */
    int pipe_controles[2];
    pid_t child;

    if(pipe(pipe_controles) == -1){
        fprintf(stderr,"Falha ao criar o PIPE");
        return 1;
    }

    child = fork();

    if(child == -1){
        fprintf(stderr,"Falha ao criar Processo");
        return 1;
    }

    if(!strcmp(input.operator,"|")){
        if(child == 0){
            /*
                Fechar a leitura de dados no processo filho
            */
            close(pipe_controles[0]);
            dup2(pipe_controles[1],STDOUT_FILENO);
            close(pipe_controles[1]);

            subarguments = input.argv_cmd1 + 1;

            execvp(input.argv_cmd1[1],subarguments);
        }else{
            /*
                Fechar a escrita de dados no processo pai
            */
            close(pipe_controles[0]);
            dup2(pipe_controles[0],STDIN_FILENO);
            close(pipe_controles[0]);
            subarguments = input.argv_cmd2 + 1;
            execvp(input.argv_cmd2[1],subarguments);
        }
    }
}

/*
    Funcao usada quando o argumento top e escolhido
*/
int command_top(){

    double cpuload[3];
    FILE *file = fopen("/proc/loadavg","r");
    

    if(file != NULL){
        if((fscanf(file,"%lf %lf %lf",&cpuload[0],&cpuload[1],&cpuload[2]) )!= 3){
            fprintf(stderr,"Error getting cpu load");
            return 1;
        }

        fprintf(stdout,"CPU LOAD: %.2f %.2f %.2f\n",cpuload[0],cpuload[1],cpuload[2]);
        return 0;
    }else{
        fprintf(stderr,"Error opening /proc/loadavg\n");
        return 1;
    }
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
