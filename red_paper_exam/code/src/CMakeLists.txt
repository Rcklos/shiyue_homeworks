cmake_minimum_required(VERSION 3.2)
project(red_paper_exam)

set(LIBFISH_VERSION 0.1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../build/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../build/lib)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAAGS} -O3 -pthread -g")

add_definitions(-DMSGPACK_NO_BOOST)

# 包含路径
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/../thirdparty/rest_rpc/include
  ${PROJECT_SOURCE_DIR}/../thirdparty/msgpack-c/include
  ${PROJECT_SOURCE_DIR}/../thirdparty/asio
)

set(libfish cofish)

# 源代码
file(GLOB source_files ${PROJECT_SOURCE_DIR}/*.cpp)
add_library(lrp STATIC ${source_files})
target_link_libraries(lrp ${libfish})


# 测试代码
file(GLOB test_files ${PROJECT_SOURCE_DIR}/../test/*.cpp)
foreach(test_file ${test_files})
  STRING(REGEX REPLACE ".*test_(.+)\\..+$" "\\1" target ${test_file})
  add_executable(${target} ${test_file})
  target_link_libraries(${target} lrp)
endforeach(test_file)
