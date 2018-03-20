cmake_minimum_required(VERSION 3.6)
project(warshall_no_path)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp network.cpp network.h router.cpp router.h reachability.h reachability.cpp rulebased.h rulebased.cpp)

set(CMAKE_BUILD_TYPE Debug)

link_directories(/usr/local/lib)
include_directories(/usr/local/include)

add_executable(all_pair ${SOURCE_FILES})

target_link_libraries(all_pair jsoncpp bdd.a)