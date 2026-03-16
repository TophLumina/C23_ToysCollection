cmake_minimum_required(VERSION 3.10)
project(ThreadPool_Project VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准 (代码中使用了 std::make_unique，需要 C++14)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 配置多平台符号隐藏机制 (Linux/Mac)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# 查找线程库 (pthread 等)
find_package(Threads REQUIRED)

# 创建动态库 (SHARED)
add_library(ThreadPool SHARED
    ThreadPool.cpp
    ThreadPool.h
)

# 链接线程库
target_link_libraries(ThreadPool PRIVATE Threads::Threads)

# 指定头文件包含路径
# 当其他项目引入此库时，能正确找到头文件
target_include_directories(ThreadPool PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# 以下为安装配置，方便将编译好的产物(dll, lib, 头文件)集中到一个目录
# 在命令行使用 `cmake --install . --prefix ./install/` 即可将结果输出到本地 install 目录下
include(GNUInstallDirs)

install(TARGETS ThreadPool
    EXPORT ThreadPoolTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}   # Windows 下的 .dll 文件
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}   # Windows 下的 .lib (导入库) 或者 Linux 下的 .so
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}   # 静态库 / 导入库
)

install(FILES ThreadPool.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}       # 头文件
)
