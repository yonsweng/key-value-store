cmake_minimum_required(VERSION 3.16)
project(cloud_computing C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(cloud_computing
        kvs.c
        kvs.h
        list.c
        list.h
        test.c)
