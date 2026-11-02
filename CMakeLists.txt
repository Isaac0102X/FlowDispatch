cmake_minimum_required(VERSION 3.16)
project(FlowDispatch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/Include")

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Svg)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

#cd to ${CMAKE_BINARY_DIR}, then cmake --build . --target copy_config
set(CONFIG_DIR "${CMAKE_BINARY_DIR}")
add_custom_target(copy_config
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/config"
            "${CONFIG_DIR}/Config"
    COMMENT "Copying config files to ${CONFIG_DIR}/Config"
)

#cd to ${CMAKE_BINARY_DIR}, then cmake --install .
# install(
#     DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/config/"
#     DESTINATION "${CMAKE_BINARY_DIR}/Config"
# )

add_subdirectory(src)

