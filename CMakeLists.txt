cmake_minimum_required(VERSION 3.11)   # 指定最低 CMake 版本

project(Aimbot_111)          # 定义项目名称和版本

# 设置 C++ 标准为 C++11
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# 查找 OpenCV
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

# 包含 OpenCV 的头文件路径
include_directories(${OpenCV_INCLUDE_DIRS})

# 添加源文件
add_library(Aimbot_111 src/Aimbot.cpp src/Number.cpp src/serial.cpp)        # 创建一个库目标 MyLib
add_executable(MyExecutable src/main.cpp)  # 创建一个可执行文件目标 MyExecutable

# 链接库到可执行文件
target_link_libraries(MyExecutable 
    Aimbot_111 
    ${OpenCV_LIBS} 
    Threads::Threads
)