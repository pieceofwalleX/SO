#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <pwd.h>

#define COMMAND_ERROR_1 "Command not found!"

typedef struct
{
    int found;
    const char *operator;
    int argc_cmd1;
    char **argv_cmd1;
    int argc_cmd2;
    char **argv_cmd2;
} comands;

comands parse(char *argv[], int argc)
{
    comands result;

    result.found = 0;
    result.operator= NULL;
    result.argc_cmd1 = 0;
    result.argc_cmd2 = 0;
    result.argv_cmd1 = NULL;
    result.argv_cmd2 = NULL;

    int found = 0;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], "<") == 0 || strcmp(argv[i], "|") == 0)
        {
            found = 1;
            result.found = 1;
            result.operator= argv[i];
            continue;
        }

        if (!found)
        {
            // Adiciona argumentos no contexto do cmd1
            result.argv_cmd1 = realloc(result.argv_cmd1, (result.argc_cmd1 + 1) * sizeof(char *));
            result.argv_cmd1[result.argc_cmd1] = strdup(argv[i]);
            result.argc_cmd1++;
        }
        else
        {
            // Adiciona argumentos no contexto do cmd2
            result.argv_cmd2 = realloc(result.argv_cmd2, (result.argc_cmd2 + 1) * sizeof(char *));
            result.argv_cmd2[result.argc_cmd2] = strdup(argv[i]);
            result.argc_cmd2++;
        }
    }
    result.argv_cmd1[result.argc_cmd1] = NULL;
    if (found)
        result.argv_cmd2[result.argc_cmd2] = NULL;

    return result;
}

/*
    Verificar se um tecla foi precionada
*/

int key_pressed()
{
    struct timeval tv;
    fd_set fds;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) == 1;
}

/*
    Funcao usada para criar ficheiro
*/
void create_file(comands input)
{   
    char** subarguments;


    int pipe_controles[2];

    pid_t child;

    if(pipe(pipe_controles) == -1){
        perror("Falha ao criar o pipe");
        return;
    }

    child = fork();

    if(child == -1){
        perror("Falha ao criar Processo");
        return;
    }

    if(child == 0){
        close(pipe_controles[0]);
        dup2(pipe_controles[1],STDOUT_FILENO);
        close(pipe_controles[1]); 

        subarguments = input.argv_cmd1 + 1;

        execvp(input.argv_cmd1[1],subarguments);
    }else{
        close(pipe_controles[1]);
        //dup2(pipe_controles[0],STDIN_FILENO);


        int output_file = open(input.argv_cmd2[0],O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if(output_file == -1){
            perror("Falha ao abrir ficheiro");
            return;
        }

        char buffer[4096];
        ssize_t bytesRead = read(pipe_controles[0],buffer,sizeof(buffer)); //Bytes lidos do pipe

        write(output_file,buffer,bytesRead);//Escrever no ficheiro

        close(pipe_controles[0]);

    }

}

void read_file(comands input){

       char** subarguments;


    int pipe_controles[2];

    pid_t child;

    if(pipe(pipe_controles) == -1){
        perror("Falha ao criar o pipe");
        return;
    }

    child = fork();

    if(child == -1){
        perror("Falha ao criar Processo");
        return;
    }

    if(child == 0){
        close(pipe_controles[0]);
        dup2(pipe_controles[1],STDOUT_FILENO);
        close(pipe_controles[1]);

        int output_file = open(input.argv_cmd2[0],O_RDONLY);

        if(output_file == -1){
            perror("Falha ao abrir ficheiro");
            return;
        }

        char buffer[4096];
        ssize_t bytesRead = read(output_file,buffer,sizeof(buffer)); //Bytes lidos do pipe

        write(STDOUT_FILENO,buffer,bytesRead);//Escrever no ficheiro

        close(output_file); 
        
    }else{
        close(pipe_controles[1]);
        dup2(pipe_controles[0],STDIN_FILENO);
        close(pipe_controles[0]);

        subarguments = input.argv_cmd1 + 1;

        execvp(input.argv_cmd1[1],subarguments);

    }

}

/*
    Funcao usada para ler o input
*/
void read_command(int argc, char *argv[])
{

    // Caso o 1 argumento seja top vamos redirecionalo
    if (!strcmp(argv[1], "top"))
    {
        command_top();
    }
    else
    {
        execute_command(parse(argv, argc));
    }

    return;
}
/*
    Funcao usada para executar o comando desejado pelo utilizador
*/
int execute_command(comands input)
{
    char **subarguments;

    if (!input.found)
    {
        subarguments = input.argv_cmd1 + 1;
        execvp(input.argv_cmd1[1], subarguments);
    }
    else
    {

        execute_multicommands(input);

        // subarguments = input.argv_cmd2 + 2;
        // execvp(input.argv_cmd2[1],subarguments);
    }
    return 0;
}

int execute_multicommands(comands input)
{
    char **subarguments;
    /*
        Aqui basicamente vai se criar um processo filho para executar
        os argumentos antes dos operadores >,< ou |
        Quando o processo filho finalizar o processo pai
        executa os argumentos depois dos operadores
    */
    int pipe_controles[2];
    pid_t child;

    if (pipe(pipe_controles) == -1)
    {
        fprintf(stderr, "Falha ao criar o PIPE");
        return 1;
    }

    child = fork();

    if (child == -1)
    {
        fprintf(stderr, "Falha ao criar Processo");
        return 1;
    }
    if (!strcmp(input.operator, "|"))
    {
        {
            if (child == 0)
            {
                /*
                    Fechar a leitura de dados no processo filho
                */
                close(pipe_controles[0]);
                dup2(pipe_controles[1], STDOUT_FILENO);
                close(pipe_controles[1]);

                subarguments = input.argv_cmd1 + 1;

                execvp(input.argv_cmd1[1], subarguments);
            }
            else
            {
                /*
                    Fechar a escrita de dados no processo pai
                */
                close(pipe_controles[1]);
                dup2(pipe_controles[0], STDIN_FILENO);
                close(pipe_controles[0]);

                // Di   gitar --color=always para aparecer a vermelho

                execvp(input.argv_cmd2[0], input.argv_cmd2);
            }
        }
    }else if(!strcmp(input.operator,">")){
        create_file(input);
    }else if(!strcmp(input.operator,"<")){
        read_file(input);
    }
}

int is_process(const char *name)
{
    for (size_t i = 0; i < strlen(name); i++)
    {
        if (name[i] < '0' || name[i] > '9')
        {
            return 0;
        }
    }
    return 1;
}

int number_of_process()
{

    DIR *dir;
    struct dirent *entry;

    dir = opendir("/proc");

    if (dir == NULL)
    {
        fprintf(stderr, "Falha ao abir /proc");
        return 1;
    }

    int count = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (is_process(entry->d_name))
        {
            count++;
        }
    }
    return count;
}

int number_of_process_running(int print)
{
    DIR *dir = opendir("/proc/");
    struct dirent *entry;
    struct stat file_info;
    int count = 1, pid,uid;
    char nome[50], state[50], user[50];

    if (dir == NULL)
    {
        perror("Falha ao abrir /proc/");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        char path[300];
        snprintf(path, sizeof(path), "/proc/%s", entry->d_name);

        if (!stat(path, &file_info) && S_ISDIR(file_info.st_mode))
        {
            if (count == 19)
            {
                break;
            }
            pid = atoi(entry->d_name);
            if (pid > 0)
            {   
                char cmdline[300];
                snprintf(cmdline, sizeof(cmdline), "/proc/%d/cmdline", pid);
                FILE *fp = fopen(cmdline, "r");

                if(fp == NULL){
                    perror("Falha ao abir o ficheiro");
                    return;
                }
                char cmdline_content[400];
                size_t bytesRead = fread(cmdline_content,1,sizeof(cmdline_content),fp);

                char cmdline_string[400];
                strncpy(cmdline_string,cmdline_content,bytesRead);
                fclose(fp);

                char status[300];
                snprintf(status, sizeof(status), "/proc/%d/status", pid);
                fp = fopen(status, "r");

                if (fp != NULL)
                {
                    char line[300];
                    while (fgets(line, sizeof(line), fp))
                    {
                        if (strstr(line, "State:"))
                        {
                            sscanf(line, "State: %[^\n]", state);
                        }
                        else if (strstr(line, "Uid:"))
                        {
                            sscanf(line, "Uid: %*d", uid);
                        }
                    }
                    fclose(fp);
                }
                
                // fp = open("/etc/passwd","r");
                // // fseek(fp,0,SEEK_SET);
                // // char passwd_line[300];
                // // while(fgets(passwd_line,sizeof(passwd_line),fp) != NULL){
                // //     int passwd_uid;
                // //     sscanf(passwd_line,"%*s %*s %d",&passwd_uid);

                // //     if(passwd_uid == uid){
                // //         sscanf(passwd_line,"%[^:]",user);
                // //         break;
                // //     }
                // // }
                // close(fp);

                struct passwd *pwd = getpwuid(uid);
                if(!pwd){
                    return;
                }

                if (!strcmp(state, "R (running)"))
                {
                    if (print == 1)
                    {
                        fprintf(stdout, "%d\t\b\b|", count);
                        fprintf(stdout, "\t%d\t\b\b|", pid);
                        fprintf(stdout, "\t%s\t\b\b|\t", state);
                        fprintf(stdout, "\t%s\t\b\b|\t", pwd->pw_name);
                        //fprintf(stdout, "\t/proc/%d/status%s\n", pid, cmdline_string);
                        for(size_t i = 0; i < bytesRead;i++){
                        char cur = cmdline_content[i];
                        if(cur == '\0'){
                            fprintf(stdout,"");
                        }else{
                            fprintf(stdout,"%c",cur);
                        }
                    }
                    fprintf(stdout,"\n");
                    }
                    count++;
                }
            }
        }
    }
    return count;
}

void list_process()
{
    DIR *dir = opendir("/proc/");
    struct dirent *entry;
    struct stat file_info;
    char nome[50], state[50], user[50];

    int process = 1, processRunning = number_of_process_running(0) - 1;

    if (dir == NULL)
    {
        fprintf(stderr, "Falha ao abir /proc");
        return;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        // Verificar quantos processo estao a ser mostrados

        char path[300];
        snprintf(path, sizeof(path), "/proc/%s", entry->d_name);

        // Verificar se e um diretorio
        if (!stat(path, &file_info) && S_ISDIR(file_info.st_mode))
        {
            int pid = atoi(entry->d_name);

            if (pid > 0)
            {
                if (process + processRunning - 1 == 20)
                {
                    break;
                }

                char cmdline[300];
                snprintf(cmdline, sizeof(cmdline), "/proc/%d/cmdline", pid);
                FILE *fp = fopen(cmdline, "r");

                if(fp == NULL){
                    perror("Falha ao abir o ficheiro");
                    return;
                }
                char cmdline_content[400];
                size_t bytesRead = fread(cmdline_content,1,sizeof(cmdline_content),fp);

                fclose(fp);

                cmdline_content[bytesRead] = '\0';


                char status[300];
                snprintf(status, sizeof(status), "/proc/%d/status", pid);

                fp = fopen(status, "r");

                if (fp != NULL)
                {
                    char line[300];
                    while (fgets(line, sizeof(line), fp))
                    {
                         if (strstr(line, "State:"))
                        {
                            sscanf(line, "State: %[^\n]", state);
                        }
                        else if (strstr(line, "User:"))
                        {
                            sscanf(line, "Uid: %*d %50s", user);
                        }
                    }
                    fclose(fp);
                }
                else
                {
                    // continue;
                }
                if (strcmp(state, "R (running)") != 0)
                {
                    fprintf(stdout, "%d\t\b\b|", process + processRunning);
                    fprintf(stdout, "\t%d\t\b\b|", pid);
                    fprintf(stdout, "\t%s\t\b\b|\t", state);
                    fprintf(stdout, "\t%s\t\b\b|\t", user);
                    //fprintf(stdout, "\t/proc/%d/status%s\n", pid, cmdline_content);

                    for(size_t i = 0; i < bytesRead;i++){
                        char cur = cmdline_content[i];
                        if(cur == '\0'){
                            fprintf(stdout,"");
                        }else{
                            fprintf(stdout,"%c",cur);
                        }
                    }
                    fprintf(stdout,"\n");

                    process++;
                }
            }
        }
    }
    closedir(dir);
}
/*
    Funcao usada quando o argumento top e escolhido
*/
int command_top()
{
    char c;
    while (1)
    {
        system("clear");
        if (key_pressed())
        {
            c = getchar();
            if (c == 'q' || c == 'Q')
            {
                return;
            }
        }
        double cpuload[3];
        FILE *load = fopen("/proc/loadavg", "r");

        if (load != NULL)
        {
            if ((fscanf(load, "%lf %lf %lf", &cpuload[0], &cpuload[1], &cpuload[2])) != 3)
            {
                fprintf(stderr, "Error getting cpu load");
                return 1;
            }

            fprintf(stdout, "CPU LOAD: %.2f %.2f %.2f\n", cpuload[0], cpuload[1], cpuload[2]);
        }
        else
        {
            fprintf(stderr, "Error opening /proc/loadavg\n");
            return 1;
        }
        /*
            Tiramos 1 aos processos porque para listar as linhas
            o inicio do counter foi setado a 1 e nao a 0
        */
        int count_process = number_of_process() - 1;
        int count_process_running = number_of_process_running(0) - 1;
        fprintf(stdout, "Processes : %d\n", count_process + count_process_running);
        fprintf(stdout, "Processes Running: %d\n", count_process_running);

        fprintf(stdout, "####################################################################################\n");
        fprintf(stdout, "[LINE]|\t[ID]\t\b\b|\t[STATUS]\t\b\b|\t[NAME]\n");
        fprintf(stdout, "####################################################################################\n\n");

        number_of_process_running(1); // O 1 server para dar Print aos Processos
        list_process();

        sleep(8);
        fprintf(stdout, "UPDATING PAGE\n");
        sleep(2);
    }
    return 0;
}

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        fprintf(stderr, "Format: mycmd <...>\n");
        fprintf(stderr, "\nmycmd --help\n");
        return EXIT_FAILURE;
    }

    read_command(argc, argv);

    return EXIT_SUCCESS;
}
