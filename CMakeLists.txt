cmake_minimum_required(VERSION 3.10)

project(main)

#配置编译选项 

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g) #-g可执行文件可以调试

#指定搜索的子目录


#配置最终的可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)

include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
#加载子目录
add_subdirectory(src)

# if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
#     message(STATUS "Setting build type to 'Debug' as none was specified.")
#     # 使用 CACHE STRING 确保这个设置可以被用户覆盖
#     set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
#     # 提供可能的选项给 GUI 工具
#     set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
# endif()

# #