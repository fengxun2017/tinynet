cmake_minimum_required(VERSION 3.10)

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


file(GLOB COMM_SOURCE_FILES "../src/*.cpp")
# 将头文件路径添加到包含路径
include_directories(${CMAKE_CURRENT_LIST_DIR}/../inc)


# 添加可执行文件
add_executable(my_tests test_tinynet_util.cpp ${COMM_SOURCE_FILES} )

# 链接 Google Test 库
target_link_libraries(my_tests gtest gmock gtest_main loglib)

# 启用测试
enable_testing()

# 添加测试
add_test(NAME MyTest COMMAND my_tests)



