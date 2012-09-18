cmake_minimum_required(VERSION 2.6)
project(mmop)

set(source_files metamorph_opcode.cpp)
include_directories(/usr/local/include/simpl
                    /usr/local/include/metamorph
                    /usr/local/include/csound)
add_definitions(-DUSE_DOUBLE)

add_library(mmop SHARED ${source_files})
target_link_libraries(mmop metamorph simpl m)
