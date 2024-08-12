cmake_minimum_required(VERSION 3.5.0)
project(snow-ball VERSION 0.1.0 LANGUAGES C CXX)
aux_source_directory(src sources)
set(CMAKE_CXX_STANDARD 17)
aux_source_directory(src/server sources)
aux_source_directory(src/client sources)
add_executable(snow-ball main.cpp ${sources})

