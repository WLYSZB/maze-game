#include "maze.h"
#include <fstream>
#include <iostream>
#include <format>
#include <utility>

/*
    将所有迷宫图片读入显存
*/
void Maze::load_textures(){
    Image end_image = LoadImage("../assets/end.png");
    textures[END] = LoadTextureFromImage(end_image);
    UnloadImage(end_image);

    Image start_image = LoadImage("../assets/start.png");
    textures[START] = LoadTextureFromImage(start_image);
    UnloadImage(start_image);

    Image floor_image = LoadImage("../assets/floor.png");
    textures[FLOOR] = LoadTextureFromImage(floor_image);
    UnloadImage(floor_image);

    Image wall_image = LoadImage("../assets/wall.png");
    textures[WALL] = LoadTextureFromImage(wall_image);
    UnloadImage(wall_image);

    Image grass_image = LoadImage("../assets/grass.png");
    textures[GRASS] = LoadTextureFromImage(grass_image);
    UnloadImage(grass_image);

    Image lava_image = LoadImage("../assets/lava.png");
    textures[LAVA] = LoadTextureFromImage(lava_image);
    UnloadImage(lava_image);
}

/*
    读取迷宫文件
    参数
        maze_file：迷宫文本文件
*/
void Maze::load_maze(const std::string& maze_file){
    std::ifstream ifs(maze_file);
    if(!ifs.is_open()){
        std::cerr<<std::format("Failed to open {}", maze_file);
        exit(1);
    }

    int row, column;
    ifs>>row>>column;

    for(int r{0}; r<row; ++r){
        std::vector<Tile> tile_row;
        for(int c{0}; c<column; ++c){
            int value;
            ifs>>value;
            TileType type{static_cast<TileType>(value)};
            tile_row.push_back({
                .type = type,
                .position = {c*TILE_WIDTH, r*TILE_HEIGHT}
            });
            if (type == START){
                start_coord = {c, r};
            }
            if (type == END){
                end_coord = {c, r};
            }
        }
        tiles.push_back(std::move(tile_row));
    }
}

Maze::Maze(const std::string& maze_file){
    load_textures();
    load_maze(maze_file);
}

Maze::~Maze(){

    //将所有迷宫图片从显存中删除
    for(auto [_, texture] : textures){
        UnloadTexture(texture);
    }
}

/*
    在屏幕上绘制迷宫
*/
void Maze::draw() const {
    for(const auto& row : tiles){
        for(const auto& tile : row){
            auto texture_it {textures.find(tile.type)};
            DrawTextureV(texture_it->second, tile.position, WHITE);
        }
    }
}

/*
    获得起点坐标
    返回值
        起点的坐标
*/
auto Maze::get_start_coord() const -> Coordinate{
    return start_coord;
}

/*
    获得地块类型
    参数
        coor：地块坐标
    返回值
        coor位置地块的类型
*/
auto Maze::get_tile_type(const Coordinate coor) const -> TileType{
    return tiles[coor.y][coor.x].type;
}

/*
    获得地块的屏幕位置
    参数
        coor：地块坐标
    返回值
        coor位置地块的屏幕显示位置
*/
auto Maze::get_tile_position(const Coordinate coor) const -> Vector2{
    return tiles[coor.y][coor.x].position;
}