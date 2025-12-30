#include <raylib.h>
#include <vector>
#include <string>
using namespace std;

// 全局纹理（存储各地块图片）
Texture2D textures[6]; // 顺序：0-普通,1-墙,2-草地,3-熔岩,4-起点,5-终点
const int TILE_SIZE = 32; // 每个地块尺寸（像素）

// 加载所有纹理资源
void LoadMazeTextures() {
    // 加载图片→转换为纹理→释放图片内存
    Image img;
    // 普通地面（0）
    img = LoadImage("D:/数据结构/迷宫小游戏/1/resources/floor.png");
    textures[0] = LoadTextureFromImage(img);
    UnloadImage(img);
    // 墙（1）
    img = LoadImage("D:/数据结构/迷宫小游戏/1/resources/wall.png");
    textures[1] = LoadTextureFromImage(img);
    UnloadImage(img);
    // 草地（2）
    img = LoadImage("D:/数据结构/迷宫小游戏/1/resources/grass.png");
    textures[2] = LoadTextureFromImage(img);
    UnloadImage(img);
    // 熔岩（3）
    img = LoadImage("D:/数据结构/迷宫小游戏/1/resources/lava.png");
    textures[3] = LoadTextureFromImage(img);
    UnloadImage(img);
    // 起点（4）
    img = LoadImage("D:/数据结构/迷宫小游戏/1/resources/start.png");
    textures[4] = LoadTextureFromImage(img);
    UnloadImage(img);
    // 终点（5）
    img = LoadImage("D:/数据结构/迷宫小游戏/1/resources/end.png");
    textures[5] = LoadTextureFromImage(img);
    UnloadImage(img);
}

int main() {
    // 初始化窗口（先临时设为320x320，后续根据迷宫尺寸调整）
    InitWindow(800, 600, "迷宫小游戏 - 基础任务1");
    LoadMazeTextures();  // 加载资源

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        // 绘制单个普通地面+墙+起点示例（验证资源加载）
        DrawTexture(textures[0], 0, 0, WHITE);       // 普通地面（0,0）
        DrawTexture(textures[1], TILE_SIZE, 0, WHITE); // 墙（32,0）
        DrawTexture(textures[4], 0, TILE_SIZE, WHITE); // 起点（0,32）
        EndDrawing();
    }

    // 释放资源
    for (int i = 0; i < 6; i++) UnloadTexture(textures[i]);
    CloseWindow();
    return 0;
}