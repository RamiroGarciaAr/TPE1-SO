// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "app.h"
#include "shm.h"

int main(int argc, char * argv[]){

    if (argc < MIN_ARGS_REQUIRED) {
        fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //Calculamos cantidad de archivos por esclavo y archivos restantes INICIALES
    size_t total_files = argc - 1;
    size_t slaves = (SLAVES > total_files) ? total_files : SLAVES; //(I)
    size_t files_per_slave = ((slaves * INITIAL_FILES_PER_SLAVE) > total_files) ? total_files/slaves : INITIAL_FILES_PER_SLAVE;
    size_t reminding_files = total_files - (slaves * files_per_slave);
    size_t files_read = 0;

    //Contiene todos los hash de los archivos 
    size_t results_dim = LINE * total_files;  // Tamaño de buffer total para la share memory
    char resultLine[LINE];                    // (HASH - PID - DIR)

    SlaveData slave[slaves];                  // Vector de esclavos (Tamaño dado por I)

    fd_set readFromSlaves;                    // Definimos los FD (File descriptors)
    FD_ZERO(&readFromSlaves);                 // Seteamos los FD

    int maxFd = 0;

    for(int i = 0; i < slaves; i++){

        pipe(slave[i].from_App_to_Slave_Pipe); // Creamos los Pipes para la comunicacion de la App al Slave
        pipe(slave[i].from_Slave_to_App_Pipe); // Creamos los Pipes para la comunicacion de la Slave al App
        


        int fd_aux = fork();      
        slave[i].pidNum = fd_aux;              

        //Caso: No se pudo crear el hijo
        if(fd_aux == -1){
            fprintf(stderr, "Child could not be created\n");
            exit(FORK_ERROR); 
        }

        //Caso: Es el hijo (Slave)
        //lee lo que le manda el app
        //El hijo procesa la informacion y devuelve el hash a la app
        else if(fd_aux == 0){
            
            //Slave lee datos de la app
            //Cerramos file descriptors acumulados
            for(int j = 0; j < i; j++)
            {
                safe_close(slave[j].from_Slave_to_App_Pipe[READ]);
                safe_close(slave[j].from_App_to_Slave_Pipe[WRITE]);
            }

            //Tanto el extremo de escritura del from_App_to_Slave_Pipe
            //Como el extremo de lectura de from_Slave_to_App_Pipe, no se usan
            safe_close(slave[i].from_App_to_Slave_Pipe[WRITE]); //Cerramos los FD inecesarios
            safe_close(slave[i].from_Slave_to_App_Pipe[READ]);

            //Redirigimos la salida
            safe_dup2(slave[i].from_Slave_to_App_Pipe[WRITE], STDOUT_FILENO);
            safe_close(slave[i].from_Slave_to_App_Pipe[WRITE]);
            safe_dup2(slave[i].from_App_to_Slave_Pipe[READ], STDIN_FILENO);
            safe_close(slave[i].from_App_to_Slave_Pipe[READ]);
            
            execve("./slave", argv, NULL);

            //En caso de que no se haya realizado el exceve por error
            perror("Execve");
            exit(EXIT_FAILURE);
        }

        //Caso: Es el padre
        else{
            safe_close(slave[i].from_App_to_Slave_Pipe[READ]);
            safe_close(slave[i].from_Slave_to_App_Pipe[WRITE]);
        }

        //Acumulamos los file descriptors de lectura que se van a utilizar para el select
        //Seteamos el maximo fd xq se ocupa del select
        FD_SET(slave[i].from_Slave_to_App_Pipe[READ], &readFromSlaves);
            if (slave[i].from_Slave_to_App_Pipe[READ] > maxFd) {
                maxFd = slave[i].from_Slave_to_App_Pipe[READ];
            }
    }

    FILE * result_file = fopen("resultado.txt", "w");

    if (result_file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    ShareMemory shm_data = CreateSHM(results_dim);

    // Imprime el peso de la Shm que necesita view para conectarse
    printf("%zu", total_files);
    fflush(stdout);
    // Esperar 2 segundos a que se conecte el proceso vista
    sleep(2);

    // Primera distribución de archivos
    distributeFiles(slave, argv, total_files, slaves, files_per_slave, 0);
   

    while (files_read < total_files) {
        fd_set readSet = readFromSlaves;
        int selectResult = select(maxFd + 1, &readSet, NULL, NULL, NULL);
        if (selectResult == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < slaves; i++) {
            if (FD_ISSET(slave[i].from_Slave_to_App_Pipe[READ], &readSet)) {
                int bytesRead = read(slave[i].from_Slave_to_App_Pipe[READ], resultLine, sizeof(resultLine));
                if (bytesRead > 0 && resultLine[bytesRead - 1] == '\n') {
                    resultLine[bytesRead - 1] = '\0';
                }
                if (bytesRead > 0) {
                    fprintf(result_file, "%s\n", resultLine);
                    shm_write(shm_data, resultLine);
                    files_read++;
                    if (reminding_files > 0) {
                        distributeFiles(slave, argv, total_files, slaves, files_per_slave, i);
                        reminding_files--;
                    }
                }
            }
        }
    }

    fclose(result_file);
    close_descriptors(slave, slaves);
    destroy_shm(shm_data);
    return 0;
}

void distributeFiles(SlaveData slaves[],char *argv[], int total_files, int numSlaves, int files_per_slave, int slaveSet){

    static int filesSend = 0;

    if(filesSend == 0){
        for (int i = 0; i < numSlaves; i++) {
            for (int j = 0; j < files_per_slave; j++) {
                // Todos los archivos han sido distribuidos
                if (filesSend >= total_files) {
                    return;
                }
                write(slaves[i].from_App_to_Slave_Pipe[WRITE], argv[filesSend + 1], strlen(argv[filesSend + 1]));
                write(slaves[i].from_App_to_Slave_Pipe[WRITE], "\n", 1);
                filesSend++;
            }
        }
    }

    else{
        write(slaves[slaveSet].from_App_to_Slave_Pipe[WRITE], argv[filesSend + 1], strlen(argv[filesSend + 1]));
        write(slaves[slaveSet].from_App_to_Slave_Pipe[WRITE], "\n", 1);
        filesSend++;
    }

}

void close_descriptors(SlaveData slave[], size_t slaves){

    for(int i = 0; i < slaves; i++) {
        for(int j = 0; j < FD_DIM; j++) {
            if(i <= 0 && j != 0 ){
                safe_close(slave[i].from_App_to_Slave_Pipe[j]);
            }
            safe_close(slave[i].from_Slave_to_App_Pipe[j]);
        }
    }
    return;
}

void safe_dup2(int old_fd, int new_fd){

    if(dup2(old_fd, new_fd) == -1){
        perror("dup2");
        exit(EXIT_FAILURE);
    }
}