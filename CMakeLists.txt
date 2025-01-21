cmake_minimum_required(VERSION 3.30)
project(http_server_c C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "-Wall -Wno-pointer-sign -pedantic -fuse-ld=mold -fsanitize=address")
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin")

file(GLOB source_files CONFIGURE_DEPENDS "src/*.c" "include/*.h")

add_executable(http_server_c ${source_files})

target_include_directories(http_server_c PUBLIC "include" "/opt/local/include")
target_link_directories(http_server_c PUBLIC "/opt/local/lib")
target_link_options(http_server_c PUBLIC -fsanitize=address)
target_link_libraries(http_server_c magic)
