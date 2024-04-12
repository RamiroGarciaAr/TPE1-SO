#include "shm.h"
#include "view.h"

int main(int argc, char* argv[]){

    int file_size, file_num, sem_value;
    // Cuando se pasa por argumento la información
    if(argc == 2){ 
        file_size = atoi(argv[1]);
    }

    // Cuando se pasa por pipe la información
    else if(argc == 1) { 

        // Leer el peso de la memoria compartida
        char input[INPUT_SIZE] = {0};
        
        fgets(input, sizeof(input), stdin);
        // Parsear la entrada para obtener el número de archivos
        if (sscanf(input, "%d", &file_size) != 1) {
            perror("Entrada incorrecta.");
            exit(1);
        }
    }
    else{
        // Proceso cuando no se pasa la información requerida
    }

    file_num = file_size / LINE;

    ShareMemory shm_data = ConnectSHM(file_size);

    sem_t * content_sem = shm_data.content_sem;
    

    do{
        shm_read(shm_data);
        sem_getvalue(content_sem, &sem_value);
        file_num--;

    } while(sem_value > 0 || file_num > 0);

    close_shm(shm_data);

    return 0;
}

