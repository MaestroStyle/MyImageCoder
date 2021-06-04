cmake_minimum_required(VERSION 3.5)

project(data_compression LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")

find_package(OpenCV REQUIRED PATHS F:/tools/opencv/build)

# Additional Include Directories
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories("include/")

add_executable(data_compression ${SOURCES} ${HEADERS})

target_link_libraries(data_compression ${OpenCV_LIBS})
