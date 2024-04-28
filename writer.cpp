#include <iostream>
#include <chrono>
#include <thread>

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

using namespace std::chrono_literals;

int main() {
    const char* shm_name = "/my_shared_memory";
    const char* semaphore_name = "/my_semaphore";
    const size_t shm_size = 1024; // Adjust the size as needed

    // Create or open the shared memory object
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Resize the shared memory object
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate");
        return 1;
    }

    // Map the shared memory into the process address space
    int* shm_ptr = static_cast<int*>(mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    sem_t* semaphore = sem_open(semaphore_name, O_CREAT, 0666, 0);

    srand(time(NULL));

    while (true) {
        int random = rand() % 100;
        std::cout << "Random number: " << random << std::endl;

        if (random == 0) {
            break;
        }

        
        *shm_ptr = random;
        sem_post(semaphore);

        std::cout << "Writer: " << random << std::endl;
        std::this_thread::sleep_for(1s);
    }


    // Unmap and close the shared memory object
    munmap(shm_ptr, shm_size);
    close(shm_fd);
    sem_close(semaphore);

    return 0;
}

