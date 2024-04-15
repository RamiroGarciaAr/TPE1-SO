#ifndef SLAVE_H
#define SLAVE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define INPUT_BUFFER_SIZE 4096
#define MD5_RESULT_SIZE 100
#define COMMAND_SIZE 300
#define BUFFER_SIZE 560
#define PID_SIZE 10

int calculateMd5(char *filePath, char *ansBuffer);

#endif