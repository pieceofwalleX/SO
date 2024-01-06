#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>

#define COMMAND_ERROR_1 "Command not found!"
#define COMMAND_ERROR_2 "Sub Argument not found!"


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

        if (!found) {
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
    }

    execute_command(argc,argv);
    return 0;
    }
/*
    Funcao usada para executar o comando desejado pelo utilizador
*/
int execute_command(int argc,char *argv[]){

    comands input = parse(argc,argv);

    execlp(input.argv_cmd2,NULL);
}

/*
    Funcao usada quando o argumento top e escolhido
*/
void command_top(){

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
