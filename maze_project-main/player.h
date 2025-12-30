/*
player.h 头文件定义所有和角色相关的内容
*/

#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "maze.h"

// 当前角色的状态
enum PlayerState{
    STANDING = -1, DOWN, LEFT, RIGHT, UP 
};

struct Player{
private:
    Texture2D texture; // 角色的贴图
    Vector2 position; // 角色当前在屏幕上的位置（像素坐标）
    PlayerState state{STANDING};
    Rectangle curr_frame_rectangle{0,0, 48.0F, 48.0F}; // 关键帧矩形，用于标注贴图中渲染的区域
    int curr_frame{0}; // 当前关键帧
    float timer{0.0F}; // 计时器，用于计算关键帧动画
    Coordinate curr_coor; // 当前坐标（地图坐标）
    Vector2 target_position; // 目标位置，角色需要移动到的位置
    float speed{50}; // 移动速度
    const Maze& maze; // 迷宫对象，用于获取地块类型以及其在屏幕上的像素位置

    void determine_frame_rectangle(); //计算当前应显示的关键帧
    void turn(PlayerState); // 切换角色状态
    void walk_update(); // 计算并更新角色的位置
    void control_update(); // 控制角色移动
public:
    Player(const Maze&);
    ~Player();

    void update(); // 根据按键以及角色状态更新角色的位置
    void draw() const; // 将角色绘制到屏幕
};

#endif