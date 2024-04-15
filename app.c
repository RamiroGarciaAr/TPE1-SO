#include "app.h"
#include "shm.h"

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
    size_t files_read = 0;

    //Contiene todos los hash de los archivos
    size_t results_dim = LINE * total_files;
    char resultLine[LINE];

    SlaveData slave[slaves];

    fd_set readFromSlaves;
    FD_ZERO(&readFromSlaves);

    int maxFd = 0;

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
            close(slave[i].from_Slave_to_App_Pipe[WRITE]);
            dup2(slave[i].from_App_to_Slave_Pipe[READ], STDIN_FILENO);
            close(slave[i].from_App_to_Slave_Pipe[READ]);
            execve("./slave", argv, NULL);

            //En caso de que no se haya realizado el exceve por error
            perror("Execve");
            exit(EXIT_FAILURE);
        }

        //Caso: Es el padre
        else{
            close(slave[i].from_App_to_Slave_Pipe[READ]);
            close(slave[i].from_Slave_to_App_Pipe[WRITE]);
        }


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
    // Esperar 2 segundos a que se conecte
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
            if (FD_ISSET(slave[i].from_Slave_to_App_Pipe[0], &readSet)) {
                int bytesRead = read(slave[i].from_Slave_to_App_Pipe[0], resultLine, sizeof(resultLine));
                if (resultLine[bytesRead - 1] == '\n') {
                    resultLine[bytesRead - 1] = '\0';
                }
                if (bytesRead > 0) {
                    fprintf(result_file, "%s\n", resultLine);
                    shm_write(shm_data, resultLine);
                    //shm_read(shm_data);
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
        write(slaves[slaveSet].from_App_to_Slave_Pipe[1], argv[filesSend + 1], strlen(argv[filesSend + 1]));
        write(slaves[slaveSet].from_App_to_Slave_Pipe[1], "\n", 1);
        filesSend++;
    }

}

void close_descriptors(SlaveData slave[], size_t slaves){

    for(int i = 0; i < slaves; i++) {
        for(int j = 0; j < FD_DIM; j++) {
            if(!(j == 0)){
                if(!(i>0 && j==1))
                close(slave[i].from_App_to_Slave_Pipe[j]);
            }
            close(slave[i].from_Slave_to_App_Pipe[j]);
        }
    }
    return;
}
