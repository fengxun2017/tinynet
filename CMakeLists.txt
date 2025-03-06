cmake_minimum_required(VERSION 3.10)

message(STATUS "in netlib show PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

# 搜索源文件
file(GLOB SOURCES "src/*.cpp" "http/*.cpp" "websocket/*.cpp" "secure/*.cpp" "can/*.cpp")

# 创建静态库
add_library(netlib STATIC ${SOURCES})

# 设置包含目录
message(STATUS "in netlib show CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")

target_include_directories(netlib PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc)
target_include_directories(netlib PUBLIC ${CMAKE_CURRENT_LIST_DIR}/http)
target_include_directories(netlib PUBLIC ${CMAKE_CURRENT_LIST_DIR}/websocket)
target_include_directories(netlib PUBLIC ${CMAKE_CURRENT_LIST_DIR}/secure)
target_include_directories(netlib PUBLIC ${CMAKE_CURRENT_LIST_DIR}/can)

if(NOT TARGET loglib)
add_subdirectory(${CMAKE_SOURCE_DIR}/../tinylog ${CMAKE_CURRENT_BINARY_DIR}/tinylog)
endif()

# 链接 loglib 库
target_link_libraries(netlib PUBLIC loglib)
target_compile_definitions(netlib PRIVATE TINYNET_DEBUG)
# target_compile_definitions(netlib PRIVATE DISABLE_LOG)

# 设置库输出目录
# set_target_properties(netlib PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
