#pragma once

#include <chrono>
#include <array>
#include <string>
#include <stdexcept>
#include <iostream>

#include <cstring>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>


struct A {
    double x[7];
    double y[7];
};

struct B {
    std::array<bool, 32> x;
    std::array<bool, 32> y;
};

struct FROM_R {
    std::chrono::time_point<std::chrono::steady_clock> time;
    A a;
    B b;
};

struct C {
    double x[7];
    double y[7];
};

struct TO_R {
    std::chrono::time_point<std::chrono::steady_clock> time;
    C a;
};


template <typename FROM, typename TO>
class SHMEM_ACCESS {
    public:
        SHMEM_ACCESS() {
            shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
            if (shm_fd == -1) {
                perror("shm_open");
                throw std::runtime_error("shm_open");
            }

            int shm_size = sizeof(DATA); 
            if (ftruncate(shm_fd, shm_size) == -1) {
                perror("ftruncate");
                throw std::runtime_error("ftruncate");
            }

            // Map the shared memory into the process address space
            data = static_cast<DATA*>(mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
            if (data == MAP_FAILED) {
                perror("mmap");
                throw std::runtime_error("mmap");
            }

            std::cout << "FROM: " << std::chrono::duration_cast<std::chrono::milliseconds>(data->from.time.time_since_epoch()).count()  << std::endl;
            std::cout << "TO: " << std::chrono::duration_cast<std::chrono::milliseconds>(data->to.time.time_since_epoch()).count()  << std::endl;

        }

        ~SHMEM_ACCESS() {
            munmap(data, sizeof(DATA));
            close(shm_fd);
        }

        void write(const FROM& from) {
            std::memcpy(&data->from, &from, sizeof(FROM));
            std::cout << "!!!!: " << std::chrono::duration_cast<std::chrono::milliseconds>(data->from.time.time_since_epoch()).count()  << std::endl;
        }

        void write(const TO& to) {
            std::memcpy(&data->to, &to, sizeof(TO));
            std::cout << "!!!!: " << std::chrono::duration_cast<std::chrono::milliseconds>(data->to.time.time_since_epoch()).count()  << std::endl;
        }

        TO read() {
            std::cout << "!!!!: " << std::chrono::duration_cast<std::chrono::milliseconds>(data->to.time.time_since_epoch()).count()  << std::endl;
            return static_cast<TO>(data->to);
        }

        FROM read_from() {
            std::cout << "!!!!: " << std::chrono::duration_cast<std::chrono::milliseconds>(data->from.time.time_since_epoch()).count()  << std::endl;
            return static_cast<FROM>(data->from);
        }

    private:
        std::string shm_name{"/my_shared_memory"};

        struct DATA {
            FROM from;
            TO to;
        } __attribute__((aligned(4)));

        DATA* data;
        int shm_fd;
};
