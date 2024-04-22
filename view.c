// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "shm.h"
#include "view.h"

int main(int argc, char* argv[]){

    size_t file_size, file_num;
    int sem_value;

    // Cuando se pasa por argumento la información
    if(argc == 2){ 
        file_num = strtoul(argv[1],NULL,10);
    }

    // Cuando se pasa por pipe la información
    else if(argc == 1) { 

        // Leer el peso de la memoria compartida
        char input[INPUT_SIZE] = {0};
        
        read(STDIN_FILENO, input, sizeof(input));
        file_num = strtoul(input,NULL,10);
    }

    else{
        // Proceso cuando no se pasa la información requerida
        fprintf(stderr, "Wrong Information");
        exit(EXIT_FAILURE);
    }

    file_size = file_num * LINE;

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

