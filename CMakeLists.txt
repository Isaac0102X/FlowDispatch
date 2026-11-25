cmake_minimum_required(VERSION 3.16)
project(FlowDispatch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/Include")

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Svg)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_subdirectory(src)

