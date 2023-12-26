cmake_minimum_required(VERSION 3.5)
project (t33 C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/include/t33/*.h")

add_library(${PROJECT_NAME} ${SRC})