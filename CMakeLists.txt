cmake_minimum_required(VERSION 3.24)
project(PROJET_LP25 C)

set(CMAKE_C_STANDARD 99)

add_executable(PROJET_LP25 main.c configuration.c configuration.h defines.c defines.h file-properties.c file-properties.h files-list.c files-list.h messages.c messages.h processes.c processes.h sync.c sync.h utility.c utility.h)
