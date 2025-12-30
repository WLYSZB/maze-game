/*
maze.h 头文件定义所有和迷宫相关的内容
 */

#ifndef MAZE_H
#define MAZE_H

#include "raylib.h"
#include <vector>
#include <unordered_map>
#include <string>

constexpr float TILE_WIDTH{48.0F}; // 地块宽度
constexpr float TILE_HEIGHT{48.0F}; // 地块高度

// 地块的种类
enum TileType{
    END=-2, START, FLOOR, WALL, GRASS, LAVA  
};

// 地块的地图坐标
struct Coordinate{
    int x,y;
};

// 地块
struct Tile{
    TileType type;
    Vector2 position; // 地块的屏幕像素坐标
};

struct Maze{
private:
    std::unordered_map<TileType, Texture2D> textures; // 所有地块贴图
    std::vector<std::vector<Tile>> tiles; // 使用二维矩阵存储所有地块
    Coordinate start_coord{-1, -1}; // 起点坐标
    Coordinate end_coord{-1, -1};  // 终点坐标

    void load_textures(); // 读取所有贴图到显存
    void load_maze(const std::string&); // 读取地图文件

public:
    Maze(const std::string&);
    ~Maze();

    void draw() const; // 将地图绘制到屏幕
    auto get_start_coord() const -> Coordinate; // 获取起点的地图坐标
    auto get_tile_type(Coordinate) const -> TileType; // 获取地块的类型
    auto get_tile_position(Coordinate) const -> Vector2; // 获取地块的屏幕像素坐标
};

#endif