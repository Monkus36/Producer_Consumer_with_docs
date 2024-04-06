#include <iostream>
#include <thread>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>

struct Table {
    int items[2];
};

sem_t *empty, *full, *mutex;
Table *table;
int fileDescriptor;

void initializeSemaphores() {
    empty = sem_open("empty", O_CREAT, 0666, 2);
    full = sem_open("full", O_CREAT, 0666, 0);
    mutex = sem_open("mutex", O_CREAT, 0666, 1);
    
    if (empty == SEM_FAILED || full == SEM_FAILED || mutex == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }
}

void setupSharedMemory() {
    // create shared memory object
    fileDescriptor = shm_open("/myshm", O_CREAT | O_RDWR, 0666);
    if (fileDescriptor == -1) {
        std::cerr << "Error opening shared memory object" << std::endl;
        exit(1);
    }

    // size the shared memory
    if (ftruncate(fileDescriptor, sizeof(Table)) == -1) {
        std::cerr << "Failed to size the shared memory object" << std::endl;
        close(fileDescriptor);
        exit(1);
    }

    // map the shared memory
    table = static_cast<Table*>(mmap(nullptr, sizeof(Table), PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0));
    if (table == MAP_FAILED) {
        std::cerr << "Failed to map the shared memory object" << std::endl;
        close(fileDescriptor);
        exit(1);
    }

    for (int i = 0; i < 2; ++i) {
        table->items[i] = 0;
    }
}

void producer() {
    //ensures that the producer only adds items to the buffer if there is space available
    sem_wait(empty);
    //ensures that only one thread or process can modify the shared resource at a time
    sem_wait(mutex); // enter critical section
    
    for (int i = 1; i < 3; i++) {
        if (table->items[i-1] == 0) { // find first empty slot
            table->items[i-1] = i;
            std::cout << "Successfully produced " << i << " and placed it into the shared memory table." << std::endl;
        }
    }

    //allows other threads/processes to access the shared resource safely
    sem_post(mutex); // leave critical section
    //signals to the consumer that there is new data available
    sem_post(full);
}

// close/unmap/unlink resources
void cleanup() {
    munmap(table, sizeof(Table));

    close(fileDescriptor);

    shm_unlink("/myshm");

    sem_close(empty);
    sem_close(full);
    sem_close(mutex);

    sem_unlink("empty");
    sem_unlink("full");
    sem_unlink("mutex");
}

void printTable() {
    for (int i = 0; i < 2; i++) {
        std::cout << std::endl << "table->items[" << i << "]: "<< table->items[i];
    }
}

int main() {
    cleanup();
    initializeSemaphores();
    setupSharedMemory();
    
    std::thread producerThread(producer);
    producerThread.join();

    printTable();
    // cleanup();
    std::cout << std::endl << "Producer finished execution" << std::endl << std::endl;
    return 0;
}
