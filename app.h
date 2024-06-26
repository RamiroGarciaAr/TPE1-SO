#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <stdbool.h>

#define INITIAL_FILES_PER_SLAVE 2
#define MIN_ARGS_REQUIRED 2
#define FD_DIM 2
#define SLAVES 5

#define FORK_ERROR -1

#define LINE 100

#define READ 0
#define WRITE 1

typedef struct {
    int pidNum;
    int from_App_to_Slave_Pipe[FD_DIM];
    int from_Slave_to_App_Pipe[FD_DIM];
} SlaveData;

void distributeFiles(SlaveData slaves[], char *argv[], int total_files, int numSlaves, int files_per_slave, int slaveSet);

void close_descriptors(SlaveData slave[], size_t slaves);

void safe_dup2(int old_fd, int new_fd);


#endif
