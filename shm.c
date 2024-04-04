#include "shm.h"

ShareMemory CreateSHM(size_t file_size) {

    ShareMemory shm_data;

    // Crear e inicializar el semaforos
    // Me indica si hay contenido
    sem_t * content_sem = safe_sem_open(CONTENT_SEM_NAME, O_CREAT, SEM_MODE, 1);
    sem_init(content_sem, 1, 0);
    //Me indica si se puede leer o escribir
    sem_t * read_write_sem = safe_sem_open(READ_WRITE_SEM_NAME, O_CREAT, SEM_MODE, 1);
    
    int fd = safe_shm_open(SHM_NAME, O_RDWR | O_CREAT, SHM_MODE, content_sem, read_write_sem);

    // Truncar el archivo al tamano deseado
    safe_ftruncate(fd, file_size,content_sem, read_write_sem);

    // Mapear el archivo a la memoria
    char *ptr = safe_mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0, content_sem, read_write_sem);

    shm_data.shm_ptr = ptr;
    shm_data.content_sem = content_sem;
    shm_data.read_write_sem = read_write_sem;
    shm_data.file_size = file_size;

    return shm_data;
}

void shm_write(ShareMemory shm_data, char * bufferIn){
    sem_wait(shm_data.read_write_sem);  // Espera el semaforo de lectura/escritura
    
    lseek(shm_data.fd, 0, SEEK_END);    // Posicionar al final del archivo
    write(shm_data.fd, bufferIn, strlen(bufferIn));
    write(shm_data.fd, "\n", 1);
    
    sem_post(shm_data.read_write_sem);  // Libera el semaforo de lectura/escritura
    sem_post(shm_data.content_sem);     // Libera el semaforo de aviso de contenido

    return;
}

void shm_read(ShareMemory shm_data){
    sem_wait(shm_data.content_sem);     // Espera el semaforo de aviso de contenido
    sem_wait(shm_data.read_write_sem);  // Espera el semaforo de lectura/escritura
    
    printf("Contenido del archivo:\n%s\n", shm_data.shm_ptr);
    
    sem_post(shm_data.read_write_sem);  // Libera el semaforo de lectura/escritura
    return;
}

sem_t * safe_sem_open(const char *name, int oflag, mode_t mode, unsigned int value){
    sem_t * sem = sem_open(name, oflag, mode, value);
    
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    return sem;
}

void safe_ftruncate(int files, off_t length, sem_t * sem1, sem_t * sem2){

    if (ftruncate(files, length) == -1) {
        perror("ftruncate");
        close(files);
        sem_close(sem1);
        sem_close(sem2);
        sem_unlink(CONTENT_SEM_NAME);
        sem_unlink(READ_WRITE_SEM_NAME);
        exit(EXIT_FAILURE);
    }

    return;
}

int safe_shm_open(const char *name, int oflag, mode_t mode, sem_t * sem1, sem_t * sem2){

    int fd = shm_open(name, oflag, mode);

    if (fd == -1) {
        perror("open");
        sem_close(sem1);
        sem_close(sem2);
        sem_unlink(CONTENT_SEM_NAME);
        sem_unlink(READ_WRITE_SEM_NAME);
        exit(EXIT_FAILURE);
    }

    return fd;
}


void * safe_mmap(void * addr, size_t length, int prot, int flags, int fd, off_t offset, sem_t * sem1, sem_t * sem2){
    void * ptr = mmap(addr, length, prot, flags, fd, offset);
    
    if (ptr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        sem_close(sem1);
        sem_close(sem2);
        sem_unlink(CONTENT_SEM_NAME);
        sem_unlink(READ_WRITE_SEM_NAME);
        exit(EXIT_FAILURE);
    }
    
    return ptr;
}

void safe_munmap(void *addr, size_t len, int fd, sem_t *sem1, sem_t *sem2){

    if (munmap(addr, len) == -1) {
        perror("munmap");
        close(fd);
        sem_close(sem1);
        sem_close(sem2);
        sem_unlink(CONTENT_SEM_NAME);
        sem_unlink(READ_WRITE_SEM_NAME);
        exit(EXIT_FAILURE);
    }
    
}

void close_shm(ShareMemory shm_data){
    safe_munmap(shm_data.shm_ptr, shm_data.file_size, shm_data.fd, shm_data.content_sem, shm_data.read_write_sem);
    sem_close(shm_data.content_sem);
    sem_close(shm_data.read_write_sem);
    sem_unlink(CONTENT_SEM_NAME);
    sem_unlink(READ_WRITE_SEM_NAME);
    shm_unlink(SHM_NAME);
}