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

    auto reader = []() {
        ShmemRead<FROM_R> shmem_access("/my_from_r");
        while (is_continue) {
            FROM_R from = shmem_access.read();

            std::cout << "From Robot time: " << std::chrono::duration_cast<std::chrono::milliseconds>(from.time.time_since_epoch()).count()  << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    auto writer = []() {
        ShmemWrite<TO_R> shmem_access("/my_to_r");

        TO_R to;
        
        while(is_continue) {
            to.time = std::chrono::steady_clock::now();

            shmem_access.write(to);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    std::thread reader_thread(reader);
    std::thread writer_thread(writer);

    if (reader_thread.joinable()) {
        reader_thread.join();
    }

    if (writer_thread.joinable()) {
        writer_thread.join();
    }

    return 0;

}
