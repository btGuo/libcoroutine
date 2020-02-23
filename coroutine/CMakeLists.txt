cmake_minimum_required(VERSION 3.0)
project(coroutine)
aux_source_directory(. SRC_LIST)
add_library(coroutine ${SRC_LIST})

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(CMAKE_INSTALL_PREFIX /usr/local)
install(TARGETS coroutine 
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/coroutine
    DESTINATION include
)
