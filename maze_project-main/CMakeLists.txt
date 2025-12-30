cmake_minimum_required(VERSION 3.30)

project(raylib_template)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug) #如果使用CLion或Visual Studio，请注释掉这一行

include(FetchContent)
FetchContent_Declare(raylib GIT_REPOSITORY
        https://gitee.com/xavierwoo2008/raylib.git # 国内镜像，下载速度更快
        # https://github.com/raysan5/raylib.git # 官方仓库地址，网络条件好时使用
        GIT_TAG 5.5 # 使用raylib的版本
        )
FetchContent_MakeAvailable(raylib)

add_executable(game 
        main.cpp
        maze.h
        maze.cpp
        player.h
        player.cpp
)

target_link_libraries(game raylib)

