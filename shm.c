// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "shm.h"
//Creamos wrappers para garantizar la seguridad
ShareMemory CreateSHM(size_t file_size) {

    ShareMemory shm_data;

    // Crear e inicializar el semaforos
    // Me indica si hay contenido
    sem_t * content_sem = safe_sem_open(CONTENT_SEM_NAME, O_CREAT, SEM_MODE, 0);
    
    int fd = safe_shm_open(SHM_NAME, O_RDWR | O_CREAT, SHM_MODE, content_sem);

    // Truncar el archivo al tamano deseado
    safe_ftruncate(fd, file_size, content_sem);

    // Mapear el archivo a la memoria
    char *ptr = safe_mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0, content_sem);

    shm_data.shm_ptr = ptr;
    shm_data.fd = fd;
    shm_data.content_sem = content_sem;
    shm_data.file_size = file_size;

    return shm_data;
}

ShareMemory ConnectSHM(size_t file_size) {

    ShareMemory shm_data;

    // Conexión al semáforo existente
    sem_t *content_sem = sem_open(CONTENT_SEM_NAME, 0); // Abre el semáforo existente
    if (content_sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Conexión a la memoria compartida existente
    int fd = shm_open(SHM_NAME, O_RDWR, SHM_MODE);
    if (fd == -1) {
        perror("shm_open");
        sem_close(content_sem);
        exit(EXIT_FAILURE);
    }

    // Mapear el archivo a la memoria
    char *ptr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        safe_close(fd);
        sem_close(content_sem);
        exit(EXIT_FAILURE);
    }

    shm_data.content_sem = content_sem;
    shm_data.file_size = file_size;
    shm_data.fd = fd;
    shm_data.shm_ptr = ptr;

    return shm_data;
}

void shm_write(ShareMemory shm_data, char * bufferIn){
    
    write(shm_data.fd, bufferIn, strlen(bufferIn));
    write(shm_data.fd, "\n", 1);
    msync(shm_data.shm_ptr,shm_data.file_size, MS_SYNC);
    
    // Libera el semaforo de aviso de contenido
    sem_post(shm_data.content_sem);    

    return;
}

void shm_read(ShareMemory shm_data){

    char bufferIn[LINE];
    ssize_t bytes_read;
    ssize_t total_bytes_read = 0;

    sem_wait(shm_data.content_sem);

    while (total_bytes_read < LINE - 1) {
        bytes_read = read(shm_data.fd, bufferIn + total_bytes_read, 1);
        if (bytes_read <= 0) {
            // Si no se leyó nada o ocurrió un error, terminar la lectura
            break;
        }
        total_bytes_read += bytes_read;

        // Verificar si se ha encontrado un salto de línea
        if (bufferIn[total_bytes_read - 1] == '\n') {
            // Se ha encontrado un salto de línea, terminar la lectura
            break;
        }
    }

    // Terminar la cadena con un carácter nulo
    bufferIn[total_bytes_read] = '\0';

    // Imprimir el contenido leído hasta el salto de línea
    printf("%s\n", bufferIn);
}

sem_t * safe_sem_open(const char *name, int oflag, mode_t mode, unsigned int value){
    sem_t * sem = sem_open(name, oflag, mode, value);
    
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    return sem;
}

void safe_ftruncate(int files, off_t length, sem_t * sem){

    if (ftruncate(files, length) == -1) {
        perror("ftruncate");
        safe_close(files);
        sem_close(sem);
        sem_unlink(CONTENT_SEM_NAME);
        exit(EXIT_FAILURE);
    }

    return;
}

int safe_shm_open(const char *name, int oflag, mode_t mode, sem_t * sem){

    int fd = shm_open(name, oflag, mode);

    if (fd == -1) {
        perror("open");
        sem_close(sem);
        sem_unlink(CONTENT_SEM_NAME);
        exit(EXIT_FAILURE);
    }

    return fd;
}


void * safe_mmap(void * addr, size_t length, int prot, int flags, int fd, off_t offset, sem_t * sem){
    void * ptr = mmap(addr, length, prot, flags, fd, offset);
    
    if (ptr == MAP_FAILED) {
        perror("mmap");
        safe_close(fd);
        sem_close(sem);
        sem_unlink(CONTENT_SEM_NAME);
        exit(EXIT_FAILURE);
    }
    
    return ptr;
}

void safe_munmap(void *addr, size_t len, int fd, sem_t *sem){

    if (munmap(addr, len) == -1) {
        perror("munmap");
        safe_close(fd);
        sem_close(sem);
        sem_unlink(CONTENT_SEM_NAME);
        exit(EXIT_FAILURE);
    }
    
}

void close_shm(ShareMemory shm_data){
    safe_munmap(shm_data.shm_ptr, shm_data.file_size, shm_data.fd, shm_data.content_sem);
    sem_close(shm_data.content_sem);
    close(shm_data.fd);
}

void destroy_shm(ShareMemory shm_data){
    safe_munmap(shm_data.shm_ptr, shm_data.file_size, shm_data.fd, shm_data.content_sem);
    sem_close(shm_data.content_sem);
    sem_unlink(CONTENT_SEM_NAME);
    //close(shm_data.fd);
    shm_unlink(SHM_NAME);
}

void safe_close(int fd){

    if(close(fd) == -1){
        perror("close");
        exit(EXIT_FAILURE);
    }
}