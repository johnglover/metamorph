cmake_minimum_required(VERSION 2.6)
project(metamorph)

set(source_files src/fx.cpp)

set(include_files src/metamorph/fx.h)

include_directories(src)
add_library(metamorph SHARED ${source_files})
target_link_libraries(metamorph notesegmentation modal simpl m)

install(TARGETS metamoprh LIBRARY DESTINATION lib)
install(FILES ${include_files} DESTINATION include/metamorph)
