
cmake_minimum_required(VERSION 3.11)

project(postgres CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(myApp src/main.cpp
                     src/employee.h
                     src/generators.h)
target_link_libraries(myApp PRIVATE CONAN_PKG::libpqxx)

