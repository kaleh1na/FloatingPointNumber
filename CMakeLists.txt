cmake_minimum_required(VERSION 3.12)

project(
    lab1
    VERSION 1.0
    DESCRIPTION "lab1"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE fixedpoint)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
