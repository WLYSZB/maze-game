#include "maze_game.h"
#include <cmath>

void Player::determine_frame_rectangle() {
    timer += GetFrameTime();
    if (timer >= PLAYER_FRAME_TIME) {
        timer = 0;
        curr_frame = (curr_frame + 1) % 3;
    }

    int frame_width = texture.width / 3;
    int frame_height = texture.height / 4;
    int state_row = static_cast<int>(state) - 1;

    curr_frame_rectangle = {
        static_cast<float>(curr_frame * frame_width),
        static_cast<float>(state_row * frame_height),
        static_cast<float>(frame_width),
        static_cast<float>(frame_height)
    };
}

void Player::turn(PlayerState new_state) {
    state = new_state;
    curr_frame = 0;
    timer = 0;
}

void Player::walk_update() {
    Vector2 direction = Vector2Subtract(target_position, position);
    float distance = Vector2Length(direction);

    if (distance > 2) {
        direction = Vector2Normalize(direction);
        position = Vector2Add(position, Vector2Scale(direction, speed * GetFrameTime()));
    }
    else {
        position = target_position;
        curr_coor = {
            static_cast<int>(round(position.x / TILE_WIDTH)),
            static_cast<int>(round(position.y / TILE_HEIGHT))
        };
        state = PlayerState::STANDING;

        // 检测熔岩/终点
        TileType current_tile = maze.get_tile_type(curr_coor);
        if (current_tile == TileType::LAVA) {
            is_dead = true;
        }
        else {
            int step_cost = (current_tile == TileType::GRASS) ? 3 : 1;
            walk_score += step_cost;
        }

        Coordinate end_coor = maze.get_end_coord();
        if (curr_coor.x == end_coor.x && curr_coor.y == end_coor.y) {
            is_win = true;
        }
    }
}

void Player::control_update() {
    if (is_win || is_dead || state != PlayerState::STANDING) return;

    Coordinate target_coor = curr_coor;
    if (IsKeyDown(KEY_DOWN)) {
        target_coor.y += 1;
        if (maze.get_tile_type(target_coor) != TileType::WALL) {
            turn(PlayerState::DOWN);
            target_position = maze.get_tile_position(target_coor);
        }
    }
    else if (IsKeyDown(KEY_LEFT)) {
        target_coor.x -= 1;
        if (maze.get_tile_type(target_coor) != TileType::WALL) {
            turn(PlayerState::LEFT);
            target_position = maze.get_tile_position(target_coor);
        }
    }
    else if (IsKeyDown(KEY_RIGHT)) {
        target_coor.x += 1;
        if (maze.get_tile_type(target_coor) != TileType::WALL) {
            turn(PlayerState::RIGHT);
            target_position = maze.get_tile_position(target_coor);
        }
    }
    else if (IsKeyDown(KEY_UP)) {
        target_coor.y -= 1;
        if (maze.get_tile_type(target_coor) != TileType::WALL) {
            turn(PlayerState::UP);
            target_position = maze.get_tile_position(target_coor);
        }
    }
}

Player::Player(const Maze& maze_ref) : maze(maze_ref), is_win(false) {
    walk_score = 0;
    is_dead = false;

    // 加载玩家纹理
    Image img_player = LoadImage("D:/数据结构/迷宫小游戏/1/assets/character.png");
    texture = LoadTextureFromImage(img_player);
    UnloadImage(img_player);

    // 初始化位置和状态
    curr_coor = maze.get_start_coord();
    position = maze.get_tile_position(curr_coor);
    target_position = position;
    speed = PLAYER_SPEED;
    state = PlayerState::STANDING;
    curr_frame = 0;
    timer = 0;

    int frame_width = texture.width / 3;
    int frame_height = texture.height / 4;
    curr_frame_rectangle = { 0, 0, static_cast<float>(frame_width), static_cast<float>(frame_height) };
}

Player::~Player() {
    UnloadTexture(texture);
}

void Player::update() {
    control_update();
    if (state != PlayerState::STANDING) {
        determine_frame_rectangle();
        walk_update();
    }
}

void Player::draw(const Camera2D& camera) {
    BeginMode2D(camera);
    DrawTextureRec(texture, curr_frame_rectangle, position, WHITE);
    EndMode2D();
}

int Player::get_score() const { return walk_score; }
bool Player::is_win_state() const { return is_win; }
bool Player::is_dead_state() const { return is_dead; }

void Player::reset() {
    is_win = false;
    is_dead = false;
    walk_score = 0;
    curr_coor = maze.get_start_coord();
    position = maze.get_tile_position(curr_coor);
    target_position = position;
    state = PlayerState::STANDING;
    curr_frame = 0;
    timer = 0;
}