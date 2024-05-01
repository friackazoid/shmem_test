#include "shmem_struct.hpp"

#include <iostream>
#include <thread>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

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



    auto writer = []() {
        ShmemWrite<FROM_R> shmem_access("/my_from_r");

        FROM_R from;
        for (int i = 0; i < 7; ++i) {
            from.a.x[i] = i;
            from.a.y[i] = i;
        }
        for (int i = 0; i < 32; ++i) {
            from.b.x[i] = i % 2 == 0;
            from.b.y[i] = i % 2 == 0;
        }

        while (is_continue) {
            from.time = std::chrono::steady_clock::now();

            shmem_access.write(from);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };


    auto reader = []() {
        ShmemRead<TO_R> shmem_access("/my_to_r");
        while (is_continue) {
            TO_R to = shmem_access.read();

            std::cout << "TO_Robot time: " << std::chrono::duration_cast<std::chrono::milliseconds>(to.time.time_since_epoch()).count()  << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    std::thread writer_thread(writer);
    std::thread reader_thread(reader);

    if (writer_thread.joinable()) {
        writer_thread.join();
    }

    if (reader_thread.joinable()) {
        reader_thread.join();
    }

    return 0;
}
