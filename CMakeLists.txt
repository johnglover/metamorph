cmake_minimum_required(VERSION 2.6)
project(metamorph)

set(source_files src/fx.cpp src/time_scale.cpp)

set(include_files src/metamorph.h
                  src/fx.h
                  src/time_scale.h)

include_directories(src)
add_library(metamorph SHARED ${source_files})
target_link_libraries(metamorph notesegmentation modal simpl m)

install(TARGETS metamorph LIBRARY DESTINATION lib)
install(FILES ${include_files} DESTINATION include/metamorph)
