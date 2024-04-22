// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "slave.h"

int main() {
    
    ssize_t nbytes;
    char inputBuffer[INPUT_BUFFER_SIZE];  // Tamaño suficientemente grande para almacenar la entrada

    while ((nbytes = read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer))) > 0) {
        // Reemplazamos el salto de línea final con un carácter nulo
        if (inputBuffer[nbytes - 1] == '\n') {
            inputBuffer[nbytes - 1] = '\0';
        }

        char * token = strtok(inputBuffer, "\n");  

        while (token != NULL) {
            char ansBuffer[MD5_RESULT_SIZE] = "";  
            if (calculateMd5(token, ansBuffer) == 0) {
                size_t output_dim = write(STDOUT_FILENO, ansBuffer, strlen(ansBuffer));
                if (output_dim == -1) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            } else {
                fprintf(stderr, "Error calculating MD5 for file: %s\n", token);
            }
            token = strtok(NULL, "\n");
        }
    }

    return 0;
}

int calculateMd5(char *filePath, char *ansBuffer) {

    int pid = getpid();
    char mypid[PID_SIZE];
    sprintf(mypid, "%d", pid);

    char command[COMMAND_SIZE];
    snprintf(command, sizeof(command), "md5sum %s", filePath);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Md5 ERROR!");
        return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead = 0;

    while (fgets(buffer, sizeof(buffer), fp) != NULL) { // Vamos leyendo linea por linea
        
        // Append the MD5 and filename to the ansBuffer
        strcat(ansBuffer, mypid);
        strcat(ansBuffer, "-");
        strcat(ansBuffer, buffer); //Hash y dir

        // pid - 12809182389173978918 Textos/texto1.txt

        // Reset the buffer
        memset(buffer, 0, sizeof(buffer));

        bytesRead++;
    }

    // Close the stream and check the exit status
    int status = pclose(fp);
    if (status == -1) {
        perror("pclose");
        return 1;
    } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        // The command exited with an error status
        fprintf(stderr, "Command failed with exit status %d\n", WEXITSTATUS(status));
        return 1;
    }

    return 0;
}