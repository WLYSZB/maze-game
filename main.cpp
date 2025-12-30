#include <raylib.h>
#include <format>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <iostream>
#include <memory>
#include "raymath.h"

// 迷宫相关常量
const int TILE_WIDTH = 48;
const int TILE_HEIGHT = 48;
const int MAX_SCREEN_WIDTH = 1920;
const int MAX_SCREEN_HEIGHT = 1080;

// 游戏状态枚举
enum class GameState {
    MENU,
    RANDOM_MAZE_SELECT,
    GAME_PLAYING,
    GAME_OVER
};

// 游戏结束选项枚举
enum class GameOverOption {
    REPLAY,
    MENU,
    EXIT
};

// 地块类型枚举
enum class TileType {
    END = -2,
    START = -1,
    FLOOR = 0,
    WALL = 1,
    GRASS = 2,
    LAVA = 3
};

// 路径类型枚举
enum class PathType {
    NONE,
    DFS,
    BFS,
    DIJKSTRA
};

// 坐标结构体
struct Coordinate {
    int x, y;
    bool operator==(const Coordinate& other) const {
        return x == other.x && y == other.y;
    }
};

// 坐标哈希函数
template<> struct std::hash<Coordinate> {
    size_t operator()(const Coordinate& c) const {
        return hash<int>()(c.x) ^ (hash<int>()(c.y) << 1);
    }
};

// Maze类：实现基础框架+解析+绘制
class Maze {
private:
    struct Tile {
        TileType type;
        Vector2 position;
    };
    std::map<TileType, Texture2D> textures;
    std::vector<std::vector<Tile>> tiles;
    Coordinate start_coord;
    Coordinate end_coord;
    int rows;
    int cols;
    PathType current_path_type = PathType::NONE;

    // 加载所有地块纹理
    void load_textures() {
        Image img_end = LoadImage("D:/数据结构/迷宫小游戏/实验3/assets/end.png");
        textures[TileType::END] = LoadTextureFromImage(img_end);
        UnloadImage(img_end);

        Image img_start = LoadImage("D:/数据结构/迷宫小游戏/实验3/assets/start.png");
        textures[TileType::START] = LoadTextureFromImage(img_start);
        UnloadImage(img_start);

        Image img_floor = LoadImage("D:/数据结构/迷宫小游戏/实验3/assets/floor.png");
        textures[TileType::FLOOR] = LoadTextureFromImage(img_floor);
        UnloadImage(img_floor);

        Image img_wall = LoadImage("D:/数据结构/迷宫小游戏/实验3/assets/wall.png");
        textures[TileType::WALL] = LoadTextureFromImage(img_wall);
        UnloadImage(img_wall);

        Image img_grass = LoadImage("D:/数据结构/迷宫小游戏/实验3/assets/grass.png");
        textures[TileType::GRASS] = LoadTextureFromImage(img_grass);
        UnloadImage(img_grass);

        Image img_lava = LoadImage("D:/数据结构/迷宫小游戏/实验3/assets/lava.png");
        textures[TileType::LAVA] = LoadTextureFromImage(img_lava);
        UnloadImage(img_lava);
    }

    // 从文件加载迷宫数据
    void load_maze(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open maze file!" << std::endl;
            return;
        }

        file >> rows >> cols;
        tiles.resize(rows, std::vector<Tile>(cols));

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                int tile_val;
                file >> tile_val;
                TileType type = static_cast<TileType>(tile_val);
                tiles[r][c].type = type;
                tiles[r][c].position = {
                    static_cast<float>(c * TILE_WIDTH),
                    static_cast<float>(r * TILE_HEIGHT)
                };

                if (type == TileType::START) {
                    start_coord = { c, r };
                }
                else if (type == TileType::END) {
                    end_coord = { c, r };
                }
            }
        }
        file.close();
    }

public:
    // 从文件加载迷宫
    Maze(const std::string& filepath) {
        load_textures();
        load_maze(filepath);
    }

    // 随机生成迷宫（预留接口，暂未实现）
    Maze(int rows, int cols) {
        load_textures();
        this->rows = rows;
        this->cols = cols;
    }

    ~Maze() {
        // 释放纹理资源
        for (auto& pair : textures) {
            UnloadTexture(pair.second);
        }
    }

    // 设置当前显示的路径类型（预留接口）
    void set_current_path(PathType type) {
        current_path_type = type;
    }

    // 绘制迷宫地块（基础绘制，无路径显示）
    void draw(const Camera2D& camera) {
        BeginMode2D(camera);
        // 绘制所有地块
        for (const auto& row : tiles) {
            for (const auto& tile : row) {
                DrawTextureV(textures[tile.type], tile.position, WHITE);
            }
        }
        EndMode2D();
    }

    // 公共接口
    Coordinate get_start_coord() const { return start_coord; }
    Coordinate get_end_coord() const { return end_coord; }
    TileType get_tile_type(const Coordinate& coord) const {
        if (coord.y >= 0 && coord.y < rows && coord.x >= 0 && coord.x < cols) {
            return tiles[coord.y][coord.x].type;
        }
        return TileType::WALL;
    }
    Vector2 get_tile_position(const Coordinate& coord) const {
        if (coord.y >= 0 && coord.y < rows && coord.x >= 0 && coord.x < cols) {
            return tiles[coord.y][coord.x].position;
        }
        return { 0, 0 };
    }
    int get_rows() const { return rows; }
    int get_cols() const { return cols; }
};

// Player类框架（未实现核心功能）
class Player {
private:
    const Maze& maze;
public:
    Player(const Maze& maze_ref) : maze(maze_ref) {}
    ~Player() {}
    void update() {}
    void draw(const Camera2D& camera) {}
    int get_score() const { return 0; }
    bool is_win_state() const { return false; }
    bool is_dead_state() const { return false; }
    void reset() {}
};

// 相机初始化（适配迷宫尺寸）
Camera2D init_camera(int maze_cols, int maze_rows) {
    Camera2D camera = { 0 };
    camera.target = { (float)(maze_cols * TILE_WIDTH) / 2, (float)(maze_rows * TILE_HEIGHT) / 2 };
    camera.offset = { (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 };
    camera.rotation = 0.0f;
    // 自动计算缩放比例
    float scale_x = (float)GetScreenWidth() / (maze_cols * TILE_WIDTH);
    float scale_y = (float)GetScreenHeight() / (maze_rows * TILE_HEIGHT);
    camera.zoom = std::min(scale_x, scale_y) * 0.8f;
    return camera;
}

// 主函数：完善菜单跳转逻辑
int main() {
    InitWindow(1280, 720, "Maze Game");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    GameState current_state = GameState::MENU;
    std::unique_ptr<Maze> maze = nullptr;
    std::unique_ptr<Player> player = nullptr;
    Camera2D camera = { 0 };
    std::string current_maze_path;

    while (!WindowShouldClose()) {
        // 菜单状态
        if (current_state == GameState::MENU) {
            if (IsKeyPressed(KEY_SPACE)) {
                // 加载自定义迷宫
                current_maze_path = "D:/数据结构/迷宫小游戏/实验3/assets/maze0.txt";
                maze = std::make_unique<Maze>(current_maze_path);
                player = std::make_unique<Player>(*maze);
                camera = init_camera(maze->get_cols(), maze->get_rows());
                current_state = GameState::GAME_PLAYING;
            }
            else if (IsKeyPressed(KEY_Z)) {
                current_state = GameState::RANDOM_MAZE_SELECT;
            }
            else if (IsKeyPressed(KEY_ESCAPE)) {
                break;
            }
        }
        // 随机迷宫选择（仅占位，未实现）
        else if (current_state == GameState::RANDOM_MAZE_SELECT) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                current_state = GameState::MENU;
            }
        }
        // 游戏进行中（仅绘制迷宫）
        else if (current_state == GameState::GAME_PLAYING) {
            if (IsKeyPressed(KEY_M)) {
                current_state = GameState::MENU;
                maze.reset();
                player.reset();
            }
        }

        // 绘制逻辑
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (current_state == GameState::MENU) {
            DrawText("MAZE GAME", GetScreenWidth() / 2 - MeasureText("MAZE GAME", 60) / 2, 100, 60, BLACK);
            DrawText("Press SPACE to start with custom maze", GetScreenWidth() / 2 - MeasureText("Press SPACE to start with custom maze", 30) / 2, 250, 30, BLACK);
            DrawText("Press Z to select random maze", GetScreenWidth() / 2 - MeasureText("Press Z to select random maze", 30) / 2, 300, 30, BLACK);
            DrawText("Press ESC to exit", GetScreenWidth() / 2 - MeasureText("Press ESC to exit", 30) / 2, 350, 30, BLACK);
        }
        else if (current_state == GameState::GAME_PLAYING) {
            maze->draw(camera);
            DrawText("Game Playing (Maze Loaded)", 5, 5, 20, BLACK);
            DrawText("Press M to return to Menu", 5, 25, 20, BLACK);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}