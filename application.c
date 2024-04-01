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

typedef struct {
    int pidNum;
    int toSlavePipe[2];
    int fromSlavePipe[2];
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

    SlaveData Slave[slaves];
    
    




    return 0;
}