cmake_minimum_required(VERSION 3.0)
project(main)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
#设置需要编译的源文件列表
set(SRC_LIST ./muduo_server.cpp)
#设置可执行文件最终存储路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#生成可执行文件server,由SRC_LIST编译而来
add_executable(server ${SRC_LIST})
#server这个目标程序需要链接三个库文件
target_link_libraries(server muduo_net muduo_base pthread)