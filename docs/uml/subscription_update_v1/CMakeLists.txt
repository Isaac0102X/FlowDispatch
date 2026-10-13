cmake_minimum_required(VERSION 3.21)
project(nekoray_subscription_update_v1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(nlohmann_json REQUIRED)

add_library(subscription_update_v1
    src/node.cpp
    src/node_controller.cpp
    src/uuid_factory_qt.cpp
)

target_include_directories(subscription_update_v1 PUBLIC include)
target_link_libraries(subscription_update_v1 PUBLIC Qt6::Core nlohmann_json::nlohmann_json)
target_compile_features(subscription_update_v1 PUBLIC cxx_std_20)
