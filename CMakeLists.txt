cmake_minimum_required(VERSION 3.10)
project(DeWAFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -Wall -pedantic-errors -Wextra -Wsign-conversion")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
add_executable(DeWAFF   src/Main.cpp
                        src/DeWAFF.cpp
                        src/Filters.cpp
                        src/ProgramInterface.cpp
                        src/Utils.cpp)
target_link_libraries(DeWAFF ${OpenCV_LIBS})

# Parallel flags
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()