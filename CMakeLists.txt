cmake_minimum_required(VERSION 3.9)
project(cinq C)

set(CMAKE_C_STANDARD 99)

add_library(cinq cinq.h iteratordef.h iterator.h iterator.c iterators.h iterators.c)