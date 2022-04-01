cmake_minimum_required(VERSION 3.10)
project(CrosswordHelper)

set(CMAKE_VERBOSE_MAKEFILE on)

# NDK APIs
find_library( android-lib android )
find_library( log-lib log )

# Configure sources
add_library(native-lib SHARED
            src/main/cpp/native-lib.cpp)

target_link_libraries( native-lib ${log-lib} ${android-lib} )
