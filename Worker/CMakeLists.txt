cmake_minimum_required(VERSION 3.5)
project (Worker)
set(CMAKE_CXX_STANDARD 17)
include_directories(../Net)
file(GLOB PROJECT_SOURCE_FILES "*.cpp")
add_library(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

