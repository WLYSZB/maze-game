#ifndef MAZE_GAME_H
#define MAZE_GAME_H

#include <raylib.h>
#include <format>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <queue>
#include <tuple>
#include <climits>
#include <chrono>
#include <memory>
#include "raymath.h"

// 迷宫相关常量
const int TILE_WIDTH = 48;
const int TILE_HEIGHT = 48;
const int MAX_SCREEN_WIDTH = 1920;  // 最大屏幕宽度
const int MAX_SCREEN_HEIGHT = 1080; // 最大屏幕高度

// 游戏状态枚举
enum class GameState {
    MENU,
    RANDOM_MAZE_SELECT,
    GAME_PLAYING,
    GAME_OVER
};

// 游戏结束选项枚举
enum class GameOverOption {
    REPLAY,    // 重玩当前迷宫
    MENU,      // 返回主菜单
    EXIT       // 退出游戏
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

// 路径类型枚举（控制显示哪种路径）
enum class PathType {
    NONE,   // 不显示路径
    DFS,    // 深度优先路径
    BFS,    // 广度优先路径
    DIJKSTRA // Dijkstra最短路径
};

// 坐标结构体
struct Coordinate {
    int x, y;
    bool operator==(const Coordinate& other) const;
};

// 为坐标添加哈希函数（用于unordered_map）
template<> struct std::hash<Coordinate> {
    size_t operator()(const Coordinate& c) const;
};

// 玩家状态枚举
enum class PlayerState {
    STANDING,
    DOWN,
    LEFT,
    RIGHT,
    UP
};

// 玩家相关常量
const float PLAYER_SPEED = 200.0f;
const float PLAYER_FRAME_TIME = 0.1f;

// Maze类：管理迷宫数据与路径（迷宫生成）
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

    // 三种路径存储
    std::vector<Coordinate> dfs_path;
    std::vector<Coordinate> bfs_path;
    std::vector<Coordinate> dijkstra_path;
    PathType current_path_type = PathType::NONE; // 当前显示的路径类型

    // 检查坐标是否可通行（排除墙和熔岩）
    bool is_valid(Coordinate c) const;

    // 获取四方向邻居（用于路径计算）
    std::vector<Coordinate> get_neighbors(Coordinate c) const;

    // 校验起点到终点是否有有效路径
    bool validate_maze_path();

    // DFS路径计算
    void compute_dfs_path();

    // BFS路径计算
    void compute_bfs_path();

    // Dijkstra路径计算
    int get_tile_cost(TileType type) const;
    void compute_dijkstra_path();

    void load_textures();
    void load_maze(const std::string& filepath);
    void generate_random_maze(int rows_, int cols_);

public:
    // 从文件加载迷宫
    Maze(const std::string& filepath);

    // 随机生成迷宫
    Maze(int rows, int cols);

    ~Maze();

    // 设置当前显示的路径类型
    void set_current_path(PathType type);

    // 绘制路径标记
    void draw_path_marker(Coordinate c, Color color) const;

    // 带相机的绘制方法
    void draw(const Camera2D& camera);

    // 公共方法
    Coordinate get_start_coord() const;
    Coordinate get_end_coord() const;
    TileType get_tile_type(const Coordinate& coord) const;
    Vector2 get_tile_position(const Coordinate& coord) const;
    int get_rows() const;
    int get_cols() const;
};

// Player类
class Player {
private:
    Texture2D texture;
    Vector2 position;
    Coordinate curr_coor;
    Vector2 target_position;
    Rectangle curr_frame_rectangle;
    int curr_frame;
    float timer;
    float speed;
    PlayerState state;
    const Maze& maze;
    bool is_win;
    int walk_score;
    bool is_dead;

    void determine_frame_rectangle();
    void turn(PlayerState new_state);
    void walk_update();
    void control_update();

public:
    Player(const Maze& maze_ref);
    ~Player();

    void update();
    void draw(const Camera2D& camera);

    int get_score() const;
    bool is_win_state() const;
    bool is_dead_state() const;

    void reset();
};

// 计时工具类
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    bool is_running;
    float elapsed_time;

public:
    Timer();

    void start();
    void stop();
    void reset();
    float get_elapsed_time() const;
};

// 初始化相机（适配迷宫尺寸）
Camera2D init_camera(int maze_cols, int maze_rows);

#endif // MAZE_GAME_H