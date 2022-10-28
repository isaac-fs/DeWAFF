cmake_minimum_required(VERSION 3.10)
project(DeWAFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include") 
add_executable(DeWAFF   src/main.cpp
                        src/DeWAFF.cpp
                        src/NonAdaptiveUSM.cpp
                        src/ParallelDeWAFF.cpp
                        src/Timer.cpp
                        src/Tools.cpp)
target_link_libraries(DeWAFF ${OpenCV_LIBS})