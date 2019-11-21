# CMakeList.txt: hcnet2webrtc 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required(VERSION 3.8)

project(hcnet2webrtc VERSION 0.0.1 LANGUAGES CXX)

message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
message("PROJECT_NAME = ${PROJECT_NAME}")
message("CMAKE_SYSTEM = ${CMAKE_SYSTEM}")
message("CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
message("CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR}")

# 添加头文件路径
set(INC_DIR
	include
	3rdparty/hc_net_sdk/${CMAKE_SYSTEM_NAME}/inc
)
# 添加库文件路径
set(LINK_DIR
	3rdparty/hc_net_sdk/${CMAKE_SYSTEM_NAME}/lib
	3rdparty/hc_net_sdk/${CMAKE_SYSTEM_NAME}/lib/HCNetSDKCom
)

# 添加源文件
set(SOURCE_FILES
	src/main.cpp
)

# 添加头文件路径
include_directories(${INC_DIR})

# 添加链接库路径
LINK_DIRECTORIES(${LINK_DIR})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_library(hcnetsdk SHARED IMPORTED)
	set_target_properties(hcnetsdk PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/3rdparty/hc_net_sdk/${CMAKE_SYSTEM_NAME}/lib/libhcnetsdk.so)
else()
	add_library(hcnetsdk STATIC IMPORTED)
	set_target_properties(hcnetsdk PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/3rdparty/hc_net_sdk/${CMAKE_SYSTEM_NAME}/lib/HCNetSDK.lib)
endif()

# 将源代码添加到此项目的可执行文件。
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
	hcnetsdk
)

# TODO: 如有需要，请添加测试并安装目标。
