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

    SHMEM_ACCESS<FROM_R, TO_R> shmem_access;

    auto writer = [&shmem_access]() {
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
            std::cout << "????: " << std::chrono::duration_cast<std::chrono::milliseconds>(from.time.time_since_epoch()).count()  << std::endl;

            shmem_access.write(from);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    std::thread writer_thread(writer);

    if (writer_thread.joinable()) {
        writer_thread.join();
    }

    return 0;
}
