cmake_minimum_required(VERSION 3.10)

project(SharedMemoryExample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g")  # Enable debug symbols in Debug mode

add_executable(writer writer.cpp)
add_executable(reader reader.cpp)
add_executable(shmem_from shmem_from.cpp)
add_executable(shmem_to shmem_to.cpp)

# Link with the real-time library
target_link_libraries(writer rt)
target_link_libraries(reader rt)
target_link_libraries(shmem_from rt)
target_link_libraries(shmem_to rt)

