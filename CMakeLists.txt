cmake_minimum_required(VERSION 2.8)

project(RealTimeObjectTracker)

find_package(OpenCV REQUIRED)

file(GLOB SOURCES "src/*.cpp")
add_executable(RealTimeObjectTracker ${SOURCES})

target_link_libraries(RealTimeObjectTracker ${OpenCV_LIBS})