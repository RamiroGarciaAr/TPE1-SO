#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>

#define MIN_ARGS_REQUIRED 2
#define SLAVES 5
#define INITIAL_FILES_PER_SLAVE 2 
#define MD5_HASH 33
#define FORK_ERROR -1

#define READ 0
#define WRITE 1

typedef struct {
    int pidNum;
    int from_App_to_Slave_Pipe[2];
    int from_Slave_to_App_Pipe[2];
} SlaveData;

int main(int argc, char * argv[]){

    if (argc < MIN_ARGS_REQUIRED) {
        fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //Calculamos cantidad de archivos por esclavo y archivos restantes INICIALES
    size_t total_files = argc - 1;
    size_t slaves = (SLAVES > total_files) ? total_files : SLAVES;
    size_t files_per_slave = ((slaves * INITIAL_FILES_PER_SLAVE) > total_files) ? total_files/slaves : INITIAL_FILES_PER_SLAVE;
    size_t reminding_files = total_files - (slaves * files_per_slave);
    
    //Contiene todos los hash de los archivos
    char results[MD5_HASH * total_files];

    SlaveData slave[slaves];
    
    for(int i = 0; i < slaves; i++){

        pipe(slave[i].from_App_to_Slave_Pipe);
        pipe(slave[i].from_Slave_to_App_Pipe);

        int fd_aux = fork();
        slave[i].pidNum = fd_aux;   

        //Caso: No se pudo crear el hijo
        if(fd_aux == -1){
            fprintf(stderr, "Child could not be created\n");
            exit(FORK_ERROR);
        }
        //Caso: Es el hijo (Slave)
        else if(fd_aux == 0){
            //Tanto el extremo de escritura del from_App_to_Slave_Pipe
            //Como el extremo de lectura de from_Slave_to_App_Pipe, no se usan 
            close(slave[i].from_App_to_Slave_Pipe[WRITE]);
            close(slave[i].from_Slave_to_App_Pipe[READ]);
            
            //Redirigimos la salida
            dup2(slave[i].from_Slave_to_App_Pipe[WRITE], STDOUT_FILENO);
            dup2(slave[i].from_App_to_Slave_Pipe[READ], STDIN_FILENO);
            
            execve("./slave", argv, NULL);

            //En caso de que no se haya realizado el exceve por error
            perror("Execve");
            exit(EXIT_FAILURE);
        }
        //Caso: Es el padre
        else{
            close(slave[i].from_App_to_Slave_Pipe[READ]);
            close(slave[i].from_Slave_to_App_Pipe[WRITE]);

            dup2(slave[i].from_App_to_Slave_Pipe[WRITE], STDOUT_FILENO);
            dup2(slave[i].from_Slave_to_App_Pipe[READ], STDIN_FILENO);
        }
    }
    
    



    return 0;
}

