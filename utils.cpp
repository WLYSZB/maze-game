#include "maze_game.h"

Camera2D init_camera(int maze_cols, int maze_rows) {
    Camera2D camera = { 0 };
    camera.target = { (float)(maze_cols * TILE_WIDTH) / 2, (float)(maze_rows * TILE_HEIGHT) / 2 };
    camera.offset = { (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 };
    camera.rotation = 0.0f;

    // 自动计算缩放比例，适配屏幕
    float scale_x = (float)GetScreenWidth() / (maze_cols * TILE_WIDTH);
    float scale_y = (float)GetScreenHeight() / (maze_rows * TILE_HEIGHT);
    camera.zoom = std::min(scale_x, scale_y) * 0.8f; // 留10%余量

    return camera;
}