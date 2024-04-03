#include "slave.h"

int main() {
    
    ssize_t nbytes;
    char inputBuffer[4096];  // Tamaño suficientemente grande para almacenar la entrada

    while ((nbytes = read(STDIN_FILENO, inputBuffer, sizeof(inputBuffer))) > 0) {
        // Reemplazamos el salto de línea final con un carácter nulo
        if (inputBuffer[nbytes - 1] == '\n') {
            inputBuffer[nbytes - 1] = '\0';
        }

        char * token = strtok(inputBuffer, "\n");  // Dividir la entrada en tokens por espacios en blanco

        while (token != NULL) {
            char ansBuffer[256] = "";  // Buffer para almacenar el resultado MD5
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
    char mypid[10];
    sprintf(mypid, "%d", pid);

    char command[300];
    snprintf(command, sizeof(command), "md5sum %s", filePath);
    // printf("command: %s\n", command);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Md5 ERROR!");
        return 1;
    }

    char buffer[560];
    size_t bytesRead = 0;

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Append the MD5 and filename to the ansBuffer
        strcat(ansBuffer, mypid);
        strcat(ansBuffer, "-");
        strcat(ansBuffer, buffer);

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