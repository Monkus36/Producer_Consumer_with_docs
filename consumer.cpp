/*
g++ producer.cpp -pthread -o producer
g++ consumer.cpp -pthread -o consumer
./producer
./consumer

g++ producer.cpp -pthread -o producer
g++ consumer.cpp -pthread -o consumer
./producer & ./consumer
*/

#include <iostream>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>

struct Table {
    int items[2]; 
};

sem_t *empty, *full, *mutex;

void openSemaphores() {
    empty = sem_open("empty", 0);
    full = sem_open("full", 0);
    mutex = sem_open("mutex", 0);
    
    if (empty == SEM_FAILED || full == SEM_FAILED || mutex == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }
}

Table *table;

void accessSharedMemory() {
    // open shared memory
    int fileDescriptor = shm_open("/myshm", O_RDWR, 0666);
    if (fileDescriptor == -1) {
        std::cerr << "Failed to open shared memory object" << std::endl;
        exit(1);
    }

    // map shared memory
    table = static_cast<Table*>(mmap(nullptr, sizeof(Table), PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0));
    if (table == MAP_FAILED) {
        std::cerr << "Failed to map the shared memory object" << std::endl;
        close(fileDescriptor);
        exit(1);
    }

    close(fileDescriptor);
}

void consumer() {
    // tells the consumer to wait until there is at least one item
    sem_wait(full);
    // ensures mutual exclusion
    sem_wait(mutex);

    //critical section
    for (int i = 0; i < 2; i++) {
        if (table->items[i] != 0) {
            std::cout << "Consumed item: " << table->items[i] << std::endl;
            table->items[i] = 0;
        }
    }
    // increment mutex semaphore; release lock
    sem_post(mutex);
    // signal to the producer that items have been consumed
    sem_post(empty);
}

void printTable() {
    for (int i = 0; i < 2; i++) {
        std::cout << std::endl << "table->items[" << i << "]: "<< table->items[i];
    }
    std::cout << std::endl;
}

void cleanup() {
    // unmap the shared memory
    munmap(table, sizeof(Table));

    // unlink the shared memory object
    shm_unlink("/myshm");

    // close and unlink semaphores
    sem_close(empty);
    sem_close(full);
    sem_close(mutex);

    sem_unlink("empty");
    sem_unlink("full");
    sem_unlink("mutex");
}

int main() {
    openSemaphores();
    accessSharedMemory();

    std::thread consumerThread(consumer);
    consumerThread.join();

    printTable();
    cleanup();
    return 0;
}
