cmake_minimum_required(VERSION 3.30)
project(http_server_c C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB source_files CONFIGURE_DEPENDS "src/*.c" "include/*.h")

add_executable(http_server_c ${source_files})

target_include_directories(http_server_c PUBLIC "include")
