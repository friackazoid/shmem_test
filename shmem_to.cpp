#include "shmem_struct.hpp"

#include <iostream>
#include <thread>

#include <signal.h>

bool is_continue = true;

void signal_handler(int signal) {
    std::cout << "Signal: " << signal << std::endl;
    is_continue = false;
}

int main () {

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("Failed to register SIGINT handler");
        return 1;
    }

    SHMEM_ACCESS<FROM_R, TO_R> shmem_access;

    auto writer = [&shmem_access]() {
        while (is_continue) {
            FROM_R from = shmem_access.read_from();

            std::cout << "time: " << std::chrono::duration_cast<std::chrono::milliseconds>(from.time.time_since_epoch()).count()  << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    std::thread writer_thread(writer);

    if (writer_thread.joinable()) {
        writer_thread.join();
    }

    return 0;

}
