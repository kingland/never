cmake_minimum_required(VERSION 2.8)
project(repl)
enable_language(C)

# repl
set(SRC main.c repl.c)
include_directories("../include")

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} nev)
