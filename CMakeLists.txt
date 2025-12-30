# 1. 指定最低CMake版本（VS2022推荐3.20及以上，匹配raylib要求）
cmake_minimum_required(VERSION 3.20)

# 2. 定义项目名（可自定义，如"RaylibTest"）和使用的语言（CXX即C++）
project(Mazegame CXX)

# 3. 设置C++标准（raylib支持C++11及以上，推荐20以兼容新特性）
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制要求指定的C++标准

# 4. 关键：指定raylib的解压路径（替换为你实际的解压路径！）
set(RAYLIB_PATH "D:/raylib-5.5_win64_msvc16")
# 5. 配置raylib的头文件路径（让编译器找到raylib.h）
include_directories(${RAYLIB_PATH}/include)

# 6. 配置raylib的库文件路径（让编译器找到raylib.lib）
link_directories(${RAYLIB_PATH}/lib)

# 7. 添加可执行文件（指定源文件：main.cpp，即后续要写的测试代码）
add_executable(${PROJECT_NAME} main.cpp "maze_game.h" "maze.cpp" "player.cpp" "timer.cpp" "utils.cpp")

# 8. 链接raylib库和Windows系统依赖库（winmm.lib是raylib必需的系统库，避免链接错误）
target_link_libraries(${PROJECT_NAME} 
    raylib          # raylib的核心库
    winmm           # Windows多媒体库（raylib音频/输入依赖）
    opengl32        # OpenGL库（raylib图形渲染依赖，msvc版本需显式链接）
)

# 9. 可选：指定VS工程的默认平台为x64（与下载的raylib"win64"版本匹配，避免架构不兼容）
set(CMAKE_VS_PLATFORM_NAME x64 CACHE STRING "" FORCE)