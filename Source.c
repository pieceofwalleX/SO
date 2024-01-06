#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <linux/limits.h>

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
void create_file()
{
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

                // Digitar --color=always para aparecer a vermelho

                execvp(input.argv_cmd2[0], input.argv_cmd2);
            }
        }
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
    int count = 0, pid;
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
            if(count == 18){
                break;
            }
            pid = atoi(entry->d_name);
            if (pid > 0)
            {

                char status[300];
                snprintf(status, sizeof(status), "/proc/%d/status", pid);
                FILE *fp = fopen(status, "r");

                if (fp != NULL)
                {
                    char line[300];
                    while (fgets(line, sizeof(line), fp))
                    {
                        if (strstr(line, "Name:"))
                        {
                            sscanf(line, "Name: %[^\n]", nome);
                        }
                        else if (strstr(line, "State:"))
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
            }
            if (!strcmp(state, "R (running)"))
            {
                if (print == 1)
                {
                    fprintf(stdout, "%d", pid);
                    fprintf(stdout, "\t\t\t%s", state);
                    fprintf(stdout, "\t\t\t/proc/%d/status%s\n", pid, nome);
                }
                count++;
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

    int process = 0, processRunning = number_of_process_running(0);

    if (dir == NULL)
    {
        fprintf(stderr, "Falha ao abir /proc");
        return 1;
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
                if (process + processRunning == 18)
                {
                    break;
                }
                process++;
                char status[300];
                snprintf(status, sizeof(status), "/proc/%d/status", pid);

                FILE *fp = fopen(status, "r");

                if (fp != NULL)
                {
                    char line[300];
                    while (fgets(line, sizeof(line), fp))
                    {
                        if (strstr(line, "Name:"))
                        {
                            sscanf(line, "Name: %[^\n]", nome);
                        }
                        else if (strstr(line, "State:"))
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
                    fprintf(stdout, "%d", pid);
                    fprintf(stdout, "\t\t\t%s", state);
                    fprintf(stdout, "\t\t\t/proc/%d/status%s\n", pid, nome);
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
        if (key_pressed())
        {
            c = getchar();
            if (c == 'q' || c == 'Q')
            {
                break;
                ;
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

        fprintf(stdout, "Processes : %d\n", number_of_process());
        fprintf(stdout, "Processes Running: %d\n", number_of_process_running(0));

        fprintf(stdout, "####################################################################################\n");
        fprintf(stdout, "@ID\t\t\t@STATUS\t\t\t\t@NAME\n");
        fprintf(stdout, "####################################################################################\n\n");


        number_of_process_running(1);//O 1 server para dar Print aos Processos
        list_process();

        sleep(10);
        system("clear");
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
