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


template <typename TData>
class ShmemWrite {
    public: 
       explicit ShmemWrite( std::string const& shm_name ) : shm_name(shm_name){
            shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
            if (shm_fd == -1) {
                perror("shm_open");
                throw std::runtime_error("shm_open");
            }

            int shm_size = sizeof(TData); 
            if (ftruncate(shm_fd, shm_size) == -1) {
                perror("ftruncate");
                throw std::runtime_error("ftruncate");
            }

            // Map the shared memory into the process address space
            data = static_cast<TData*>(mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
            if (data == MAP_FAILED) {
                perror("mmap");
                throw std::runtime_error("mmap");
            }
        }

        ~ShmemWrite() {
            munmap(data, sizeof(TData));
            close(shm_fd);
        }

        void write(const TData& data) {
            std::memcpy(this->data, &data, sizeof(TData));
        }
    private:
        std::string shm_name{""};

        TData* data;
        int shm_fd;

};


template <typename TData>
class ShmemRead {
    public: 
       explicit ShmemRead( std::string const& shm_name) : shm_name(shm_name){
            shm_fd = shm_open(shm_name.c_str(), O_RDONLY, 0666);
            if (shm_fd == -1) {
                perror("shm_open");
                throw std::runtime_error("shm_open");
            }

            // Map the shared memory into the process address space
            data = static_cast<TData*>(mmap(0, sizeof(TData), PROT_READ , MAP_SHARED, shm_fd, 0));
            if (data == MAP_FAILED) {
                perror("mmap");
                throw std::runtime_error("mmap");
            }
        }

        ~ShmemRead() {
            munmap(data, sizeof(TData));
            close(shm_fd);
        }

        TData read() {
            return static_cast<TData>(*data);
        }

    private:
        std::string shm_name{};

        TData* data;
        int shm_fd;
};


template <typename TData>
class ShmemAccesor {
    public:
        explicit ShmemAccesor(std::string const& shm_name) : reader(shm_name), writer(shm_name) { }

    public:

        ShmemRead <TData> reader;
        ShmemWrite <TData> writer;
};


