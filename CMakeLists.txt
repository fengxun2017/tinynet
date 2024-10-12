
message(STATUS "in netlib show PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

# 搜索源文件
file(GLOB SOURCES "src/*.cpp")

# 创建静态库
add_library(netlib STATIC ${SOURCES})

# 设置包含目录
message(STATUS "in netlib show CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")

target_include_directories(netlib PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc)

# 链接 loglib 库
target_link_libraries(netlib PUBLIC loglib)

# 设置库输出目录
set_target_properties(netlib PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
