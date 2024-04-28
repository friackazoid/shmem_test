#include <iostream>
#include <chrono>
#include <thread>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

using namespace std::chrono_literals;

int main() {
    const char* shm_name = "/my_shared_memory";
    const char* semaphore_name = "/my_semaphore";
    const size_t shm_size = 1024; // Adjust the size as needed

    // Open the shared memory object
    int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Map the shared memory into the process address space
    int* shm_ptr = static_cast<int*>( mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_fd, 0) );
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    sem_t* semaphore = sem_open(semaphore_name, O_CREAT, 0666, 0); 

    while (true) {

        sem_wait(semaphore);

        // Read and display the message from the shared memory
        std::cout << "Message read from shared memory: " << *(static_cast<int*>(shm_ptr)) << std::endl;
        std::this_thread::sleep_for(500ms);
    }


    // Unmap and close the shared memory object
    munmap(shm_ptr, shm_size);
    close(shm_fd);

    // Unlink the shared memory object
    shm_unlink(shm_name);
    sem_close(semaphore);

    return 0;
}

