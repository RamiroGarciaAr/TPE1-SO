#ifndef SHARE_MEMORY_H
#define SHARE_MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>

#define SEM_MODE 0666
#define SHM_MODE 0666

#define CONTENT_SEM_NAME "/flag_semaphore"
#define READ_WRITE_SEM_NAME "/read_write_semaphore"
#define SHM_NAME "/share_memory"
#define LINE 100

typedef struct{
    
    char *shm_name;
    char *shm_ptr;
    int fd;
    
    int file_size;

    sem_t *content_sem;

}ShareMemory;

ShareMemory CreateSHM(int file_size);
ShareMemory ConnectSHM(int file_size);
sem_t * safe_sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
void safe_ftruncate(int files, off_t length, sem_t * sem);
int safe_shm_open(const char *name, int oflag, mode_t mode, sem_t * sem);
void * safe_mmap(void * addr, size_t length, int prot, int flags, int fd, off_t offset, sem_t * sem);
void shm_write(ShareMemory shm_data, char * bufferIn);
void shm_read(ShareMemory shm_data);
void safe_munmap(void *addr, size_t len, int fd, sem_t * sem);
void close_shm(ShareMemory shm_data);
void destroy_shm(ShareMemory shm_data);


#endif