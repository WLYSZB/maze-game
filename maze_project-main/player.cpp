#include "player.h"
#include "raymath.h"
#include <cmath>

constexpr float PLAYER_FRAME_TIME{0.1F};

Player::Player(const Maze& m): maze(m){
    Image player_image {LoadImage("../assets/character.png")};
    texture = LoadTextureFromImage(player_image);
    UnloadImage(player_image);

    auto start_coord {m.get_start_coord()};
    curr_coor = start_coord;
    position = m.get_tile_position(start_coord);
}

Player::~Player(){
    // 将角色贴图从显存中删除
    UnloadTexture(texture);
}

/*
    角色更新主函数，每帧调用1次
*/
void Player::update(){
    control_update();
    if (state == STANDING) {return;}
    determine_frame_rectangle();
    walk_update();
}

/*
    计算角色当前动画帧
*/
void Player::determine_frame_rectangle(){
    timer += GetFrameTime();
    if(timer >= PLAYER_FRAME_TIME){
        timer = 0;
        ++curr_frame;
        if(curr_frame > 2){curr_frame = 0;}
    }
    curr_frame_rectangle.x = curr_frame * texture.width/3;
}

/*
    在屏幕上绘制角色
*/
void Player::draw() const {
    DrawTextureRec(texture, curr_frame_rectangle, position, WHITE);
}

/*
    转向
    参数
        to_state：更改的方向
*/
void Player::turn(const PlayerState to_state){
    if (state == to_state){return;}

    state = to_state;
    curr_frame = 0;
    curr_frame_rectangle.x = 0;
    curr_frame_rectangle.y = static_cast<int>(to_state) * TILE_HEIGHT;
    timer = 0.0F;
}

/*
    移动更新函数，每帧调用一次
*/
void Player::walk_update(){
    if(state == STANDING) {return;}

    //监测是否已经到达目标点
    if(constexpr float epsilon{0.001F}; // 浮点数不能直接判断等于，需要使用误差判断
            std::fabs(target_position.x - position.x) <= epsilon
            && std::fabs(target_position.y - position.y) <= epsilon){
        state = STANDING;
        return;
    }

    //根据目标和当前位置计算速度向量
    auto direction {Vector2Subtract(target_position, position)};
    direction = Vector2Normalize(direction);
    auto velocity = Vector2Scale(direction, speed*GetFrameTime());

    // 更新当前位置
    position = Vector2Add(position, velocity);
}

/*
    控制更新函数，每帧调用一次
*/
void Player::control_update(){
    if(state != STANDING) {return;}

    Coordinate target_coor{curr_coor};
    
    // 检测是否按下方向键
    if(IsKeyDown(KEY_DOWN)){
        if(state != DOWN){turn(DOWN);}
        ++target_coor.y;
    }else if(IsKeyDown(KEY_LEFT)){
        if(state != LEFT){turn(LEFT);}
        --target_coor.x;
    }else if(IsKeyDown(KEY_RIGHT)){
        if(state != RIGHT){turn(RIGHT);}
        ++target_coor.x;
    }else if(IsKeyDown(KEY_UP)){
        if(state != UP){turn(UP);}
        --target_coor.y;
    }else{
        return;
    }

    if(maze.get_tile_type(target_coor) == WALL){return;}

    //更新目标位置和当前位置
    target_position = maze.get_tile_position(target_coor);
    curr_coor = target_coor;
}