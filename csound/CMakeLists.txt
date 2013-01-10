cmake_minimum_required(VERSION 2.6)
project(mmop)

set(source_files metamorph_opcode.cpp)
include_directories(/usr/local/include/simpl
                    /usr/local/include/metamorph
                    /usr/local/include/csound)
set(libs metamorph simpl m)
add_definitions(-DUSE_DOUBLE)

if(APPLE)
    include_directories(
        /Library/Frameworks/CsoundLib.framework/Versions/Current/Headers
        /Library/Frameworks/CsoundLib64.framework/Versions/Current/Headers
    )
endif(APPLE)

add_library(mmop SHARED ${source_files})
target_link_libraries(mmop ${libs})
