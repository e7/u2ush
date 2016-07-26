cmake_minimum_required(VERSION 2.8)
project(u2ush)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})
SET(CMAKE_CXX_FLAGS -std=c++11)

AUX_SOURCE_DIRECTORY(src src_files)

include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(u2ush ${src_files})
target_link_libraries(u2ush ${QT_LIBRARIES})
