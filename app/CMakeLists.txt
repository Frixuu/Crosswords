cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

find_library( log-lib log )
find_library( android-lib android )

add_library( native-lib SHARED src/main/cpp/native-lib.cpp )
add_library( app-glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c )

target_link_libraries( native-lib app-glue ${log-lib} ${android-lib} )