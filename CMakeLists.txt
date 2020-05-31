# CMakeList.txt: hcnet_example 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required(VERSION 3.8)

project(hcnet_example VERSION 0.0.1 LANGUAGES CXX)

message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
message("PROJECT_NAME = ${PROJECT_NAME}")
message("CMAKE_SYSTEM = ${CMAKE_SYSTEM}")
message("CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
message("CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR}")

set(CMAKE_CXX_FLAGS "-g -fexec-charset=GBK")

# 添加头文件路径
set(INC_DIR
        include
        3rdParty/${CMAKE_SYSTEM_NAME}/hc_net_sdk/inc
        3rdParty/${CMAKE_SYSTEM_NAME}/spdlog/inc
        3rdParty/${CMAKE_SYSTEM_NAME}/tinyxml2/inc
        )
# 添加库文件路径
set(LINK_DIR
        3rdParty/${CMAKE_SYSTEM_NAME}/hc_net_sdk/lib
        3rdParty/${CMAKE_SYSTEM_NAME}/hc_net_sdk/lib/HCNetSDKCom
        3rdParty/${CMAKE_SYSTEM_NAME}/spdlog/lib
        3rdParty/${CMAKE_SYSTEM_NAME}/tinyxml2/lib
        )

# 添加源文件
set(SOURCE_FILES
        source/hc_net.cpp
        source/main.cpp
        )

# 添加头文件路径
include_directories(${INC_DIR})

# 添加链接库路径
link_directories(${LINK_DIR})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_library(hcnetsdk SHARED IMPORTED)
    set_target_properties(hcnetsdk PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/3rdParty/${CMAKE_SYSTEM_NAME}/hc_net_sdk/lib/libhcnetsdk.so)
    add_library(spdlog STATIC IMPORTED)
    set_target_properties(spdlog PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/3rdParty/${CMAKE_SYSTEM_NAME}/spdlog/lib/libspdlog.a)
    add_library(tinyxml2 STATIC IMPORTED)
    set_target_properties(tinyxml2 PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/3rdParty/${CMAKE_SYSTEM_NAME}/tinyxml2/lib/libtinyxml2.a)
else ()
    add_library(hcnetsdk STATIC IMPORTED)
    set_target_properties(hcnetsdk PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/3rdParty/${CMAKE_SYSTEM_NAME}/hc_net_sdk/lib/HCNetSDK.lib)
    add_library(spdlog STATIC IMPORTED)
    set_target_properties(spdlog PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/3rdParty/${CMAKE_SYSTEM_NAME}/spdlog/lib/libspdlog.a)
    add_library(tinyxml2 STATIC IMPORTED)
    set_target_properties(tinyxml2 PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/3rdParty/${CMAKE_SYSTEM_NAME}/tinyxml2/lib/libtinyxml2.a)
endif ()

# 将源代码添加到此项目的可执行文件。
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
        hcnetsdk
        spdlog
        tinyxml2
        )

# TODO: 如有需要，请添加测试并安装目标。
