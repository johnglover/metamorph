cmake_minimum_required(VERSION 2.6)
project(metamorph_examples)

set(source_files spec_env_interp.cpp)
include_directories(/usr/local/include/simpl /usr/local/include/metamorph)
add_definitions(-DUSE_DOUBLE)

add_executable(spec_env_interp ${source_files})
target_link_libraries(spec_env_interp sndfile metamorph simpl m)
