cmake_minimum_required(VERSION 3.10)

project(main)

#配置编译选项

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g) #-g可执行文件可以调试

#头文件搜索路径
#include_directories()

#设置可执行文件的存放路径 同样也有静态库动态库的存放路径设置
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#库文件搜索路径
#link_directories()

#把.指定路径下所有源文件名字放入变量名SRC_LIST中
# aux_source_directory(. SRC_LIST)
set(SRC_LIST ./muduo_server.cpp )
#生成可执行文件 (设置需要编译的源文件列表),由SRC_LIST变量所定义的源文件编译而来
add_executable(server ${SRC_LIST})
#表示server这个目标程序，需要连接muduo_net, muduo_base和pthread这三个库文件
target_link_libraries(server muduo_net muduo_base pthread)

