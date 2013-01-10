cmake_minimum_required(VERSION 2.6)
project(metamorph)

set(source_files src/fx.cpp
                 src/transformations.cpp
                 src/time_scale.cpp
                 src/spec_env.cpp)
set(include_files src/metamorph.h
                  src/fx.h
                  src/transformations.h
                  src/time_scale.h
                  src/spec_env.h)
set(libs m gsl fftw3 simpl modal notesegmentation)

include_directories(src)
add_library(metamorph SHARED ${source_files})
target_link_libraries(metamorph ${libs})

install(TARGETS metamorph LIBRARY DESTINATION lib)
install(FILES ${include_files} DESTINATION include/metamorph)

# ----------------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
    set(test_src ${source_files} tests/test_fx.cpp)
    list(APPEND libs cppunit sndfile)
    add_executable(test_fx ${test_src})
    target_link_libraries(test_fx ${libs})
else()
    message("Not building tests. To change run CMake with -D BUILD_TESTS=yes")
endif()
