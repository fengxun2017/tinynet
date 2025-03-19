cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_FLAGS "-O0 -g -std=c++17")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for gcov
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")

message(STATUS "in utest show CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")

# 引入 FetchContent 模块
include(FetchContent)
# 配置 Google Test
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.11.0.zip
)
message(STATUS "Fetching Google Test...")
# 下载并添加 Google Test
FetchContent_MakeAvailable(googletest)
message(STATUS "Google Test fetched successfully!")


file(GLOB COMM_SOURCE_FILES 
  "../src/*.cpp" 
  "../src/poller/*.cpp" 
  "../src/socket/*.cpp" )
# 将头文件路径添加到包含路径
include_directories(${CMAKE_CURRENT_LIST_DIR}/../inc)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../inc/poller)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../inc/socket)

if(NOT TARGET loglib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../tinylog ${CMAKE_CURRENT_BINARY_DIR}/tinylog)
endif()


# 启用测试
enable_testing()

# 批量添加目标,链接 Google Test 库
set(TARGETS test_event_loop
            test_io_channel
            test_io_poller
            test_wakeup_poller
            test_io_socket
            )
foreach(target ${TARGETS})
    add_executable(${target} ${target}.cpp ${COMM_SOURCE_FILES})  # 假设每个目标的源文件与目标同名
    target_link_libraries(${target} gtest gmock gtest_main loglib)

    # 添加测试
    add_test(NAME ${target} COMMAND ${target})
endforeach()







