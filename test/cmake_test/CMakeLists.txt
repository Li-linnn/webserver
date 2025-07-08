//最低版本
cmake_minimum_required(VERSION 3.14)

project(CODE)

//添加c++11版本
set(CMAKE_CXX_STANDARD 11)

//头文件：头文件和源文件不在一个目录下会找不到路径，所以指定搜索头文件路径
include_directories(./cmaketest ./include)//文件与文件之间用空格分开

//将多个源文件打包加入列表
file(GLOB_RECURSE SOURCES "src1/*.cpp" "src2/*.cpp" "func01.cpp" "func02.cpp")

//生成静态库
set(LIB1_SOURCES ./src1/func03.cpp)
add_library(lib1 STATIC ${LIB1_SOURCES})//动态库STATIC改成SHARED

//指定静态库生成路径和名称
link_directories(./)
link_libraries(lib1)//将静态库链接到所有目标里

add_executable(main ${SOURCES} main.cpp)
add_executable(test ${SOURCES} test.cpp)

target_link_directories(main PUBLIC ./)
target_link_libraries(main lib1)//只链接到main里