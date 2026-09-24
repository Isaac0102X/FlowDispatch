cmake_minimum_required(VERSION 3.16)
project(core_manager CXX)

set(CMAKE_CXX_STANDARD 20)

# 查找系统安装的 gRPC 和 Protobuf
find_package(gRPC CONFIG REQUIRED)

# 获取 gRPC C++ 插件路径（备用方案）
get_target_property(GRPC_CPP_PLUGIN gRPC::grpc_cpp_plugin LOCATION)
#                   ^^^^^^^^^^^^^^^ ^^^^^^^^^^^^^^^^^^^^^^ ^^^^^^^^
#                   参数1: 输出变量名   参数2: 目标名        参数3: 属性名



# 生成 proto 源文件
set(PROTO_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../proto/libcore.proto")
set(PROTO_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
#在CMake配置阶段mkdir
file(MAKE_DIRECTORY ${PROTO_GENERATED_DIR})

add_custom_command(
    #OUTPUT用于声明一个命令将要生成的文件（或文件列表）。
    #它告诉 CMake 构建系统：“运行这个命令后，会产出这些文件
    #如果这些文件缺失或过时，就重新运行命令。”
    OUTPUT "${PROTO_GENERATED_DIR}/libcore.pb.cc"
           "${PROTO_GENERATED_DIR}/libcore.pb.h"
           "${PROTO_GENERATED_DIR}/libcore.grpc.pb.cc"
           "${PROTO_GENERATED_DIR}/libcore.grpc.pb.h"
    #COMMAND在构建阶段执行，不是配置阶段
    #如果add.proto文件被修改了会重新执行它
    COMMAND protobuf::protoc
        --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/../proto
        --cpp_out=${PROTO_GENERATED_DIR}
        --grpc_out=${PROTO_GENERATED_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        ${PROTO_SRC}
    DEPENDS ${PROTO_SRC}
    #COMMENT的效果与message相似，前者在make构建阶段，后者在cmake ../配置阶段
    COMMENT "Generating gRPC C++ sources..."
)

add_library(nekocore_lib
    ${PROTO_GENERATED_DIR}/libcore.pb.cc
    ${PROTO_GENERATED_DIR}/libcore.grpc.pb.cc
)

#target_include_directories相当于windows下图形界面vs设置项目的Include目录
#main.cpp中的#include "add.grpc.pb.h"就是通过它找到地方的
target_include_directories(nekocore_lib PUBLIC ${PROTO_GENERATED_DIR})

target_link_libraries(nekocore_lib
    gRPC::grpc++
    protobuf::libprotobuf
)

# 客户端可执行文件
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} nekocore_lib)

# 把运行需要的资源文件拷贝到和可执行文件同一个输出目录
set(NKR_ASSETS
    ${CMAKE_SOURCE_DIR}/assets/geoip.db
    ${CMAKE_SOURCE_DIR}/assets/geosite.db
    ${CMAKE_SOURCE_DIR}/assets/geoip.dat
    ${CMAKE_SOURCE_DIR}/assets/geosite.dat
    ${CMAKE_SOURCE_DIR}/assets/nekobox_core
    ${CMAKE_SOURCE_DIR}/assets/test_node.json
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${NKR_ASSETS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copying nekobox_core runtime assets to build output directory"
)
