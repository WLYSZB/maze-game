#include <raylib.h>
#include <format>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <iostream>
#include <cmath>
#include <stack>
#include <queue>
#include <tuple>
#include <algorithm>
#include <climits>
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

// Maze类：添加寻路算法实现
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
    PathType current_path_type = PathType::NONE;

    // 检查坐标是否可通行（排除墙和熔岩）
    bool is_valid(Coordinate c) const {
        return c.x >= 0 && c.x < cols && c.y >= 0 && c.y < rows
            && get_tile_type(c) != TileType::WALL
            && get_tile_type(c) != TileType::LAVA;
    }

    // 获取四方向邻居
    std::vector<Coordinate> get_neighbors(Coordinate c) const {
        std::vector<Coordinate> neighbors;
        if (is_valid({ c.x, c.y - 1 })) neighbors.push_back({ c.x, c.y - 1 }); // 上
        if (is_valid({ c.x, c.y + 1 })) neighbors.push_back({ c.x, c.y + 1 }); // 下
        if (is_valid({ c.x - 1, c.y })) neighbors.push_back({ c.x - 1, c.y }); // 左
        if (is_valid({ c.x + 1, c.y })) neighbors.push_back({ c.x + 1, c.y }); // 右
        return neighbors;
    }

    // 校验起点到终点是否有有效路径
    bool validate_maze_path() {
        std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
        std::queue<Coordinate> q;
        q.push(start_coord);
        visited[start_coord.y][start_coord.x] = true;

        while (!q.empty()) {
            Coordinate curr = q.front();
            q.pop();
            if (curr == end_coord) return true;

            for (auto& neighbor : get_neighbors(curr)) {
                if (!visited[neighbor.y][neighbor.x]) {
                    visited[neighbor.y][neighbor.x] = true;
                    q.push(neighbor);
                }
            }
        }
        return false;
    }

    // DFS路径计算
    void compute_dfs_path() {
        std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
        std::stack<std::pair<Coordinate, std::vector<Coordinate>>> s;
        s.push({ start_coord, {start_coord} });

        while (!s.empty()) {
            auto [curr, path] = s.top();
            s.pop();

            if (curr == end_coord) {
                dfs_path = path;
                return;
            }
            if (visited[curr.y][curr.x]) continue;
            visited[curr.y][curr.x] = true;

            auto neighbors = get_neighbors(curr);
            std::reverse(neighbors.begin(), neighbors.end());
            for (auto& neighbor : neighbors) {
                if (!visited[neighbor.y][neighbor.x]) {
                    std::vector<Coordinate> new_path = path;
                    new_path.push_back(neighbor);
                    s.push({ neighbor, new_path });
                }
            }
        }
    }

    // BFS路径计算
    void compute_bfs_path() {
        std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
        std::vector<std::vector<Coordinate>> prev(rows, std::vector<Coordinate>(cols, { -1, -1 }));
        std::queue<Coordinate> q;
        q.push(start_coord);
        visited[start_coord.y][start_coord.x] = true;

        while (!q.empty()) {
            Coordinate curr = q.front();
            q.pop();
            if (curr == end_coord) break;

            for (auto& neighbor : get_neighbors(curr)) {
                if (!visited[neighbor.y][neighbor.x]) {
                    visited[neighbor.y][neighbor.x] = true;
                    prev[neighbor.y][neighbor.x] = curr;
                    q.push(neighbor);
                }
            }
        }

        // 回溯路径
        Coordinate curr = end_coord;
        while (curr.x != -1 && curr.y != -1) {
            bfs_path.push_back(curr);
            curr = prev[curr.y][curr.x];
        }
        std::reverse(bfs_path.begin(), bfs_path.end());
    }

    // Dijkstra路径计算
    int get_tile_cost(TileType type) const {
        return (type == TileType::GRASS) ? 3 : 1;
    }

    void compute_dijkstra_path() {
        const int INF = INT_MAX;
        std::vector<std::vector<int>> dist(rows, std::vector<int>(cols, INF));
        std::vector<std::vector<Coordinate>> prev(rows, std::vector<Coordinate>(cols, { -1, -1 }));
        using PriorityNode = std::tuple<int, int, int>;
        std::priority_queue<PriorityNode, std::vector<PriorityNode>, std::greater<>> pq;

        dist[start_coord.y][start_coord.x] = 0;
        pq.emplace(0, start_coord.x, start_coord.y);

        while (!pq.empty()) {
            auto [cost, x, y] = pq.top();
            pq.pop();
            Coordinate curr = { x, y };

            if (curr == end_coord) break;
            if (cost > dist[y][x]) continue;

            for (auto& neighbor : get_neighbors(curr)) {
                int nx = neighbor.x, ny = neighbor.y;
                int new_cost = cost + get_tile_cost(get_tile_type(neighbor));
                if (new_cost < dist[ny][nx]) {
                    dist[ny][nx] = new_cost;
                    prev[ny][nx] = curr;
                    pq.emplace(new_cost, nx, ny);
                }
            }
        }

        // 回溯路径
        Coordinate curr = end_coord;
        while (curr.x != -1 && curr.y != -1) {
            dijkstra_path.push_back(curr);
            curr = prev[curr.y][curr.x];
        }
        std::reverse(dijkstra_path.begin(), dijkstra_path.end());
    }

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

        // 计算三种路径
        compute_dfs_path();
        compute_bfs_path();
        compute_dijkstra_path();
    }

public:
    // 从文件加载迷宫
    Maze(const std::string& filepath) {
        load_textures();
        load_maze(filepath);
    }

    // 随机生成迷宫（预留接口）
    Maze(int rows, int cols) {
        load_textures();
        this->rows = rows;
        this->cols = cols;
    }

    ~Maze() {
        for (auto& pair : textures) {
            UnloadTexture(pair.second);
        }
    }

    // 设置当前显示的路径类型
    void set_current_path(PathType type) {
        current_path_type = type;
    }

    // 绘制路径标记
    void draw_path_marker(Coordinate c, Color color) const {
        Vector2 pos = get_tile_position(c);
        DrawRectangle(
            static_cast<int>(pos.x + TILE_WIDTH / 2 - 8),
            static_cast<int>(pos.y + TILE_HEIGHT / 2 - 8),
            16, 16,
            color
        );
    }

    // 绘制迷宫+路径
    void draw(const Camera2D& camera) {
        BeginMode2D(camera);
        // 绘制所有地块
        for (const auto& row : tiles) {
            for (const auto& tile : row) {
                DrawTextureV(textures[tile.type], tile.position, WHITE);
            }
        }
        // 绘制选中的路径
        switch (current_path_type) {
        case PathType::DFS:
            for (const auto& c : dfs_path) {
                draw_path_marker(c, Color{ 255, 0, 0, 150 }); // 红色
            }
            break;
        case PathType::BFS:
            for (const auto& c : bfs_path) {
                draw_path_marker(c, Color{ 0, 0, 255, 150 }); // 蓝色
            }
            break;
        case PathType::DIJKSTRA:
            for (const auto& c : dijkstra_path) {
                draw_path_marker(c, Color{ 0, 255, 0, 150 }); // 绿色
            }
            break;
        default: // NONE
            break;
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

// 相机初始化
Camera2D init_camera(int maze_cols, int maze_rows) {
    Camera2D camera = { 0 };
    camera.target = { (float)(maze_cols * TILE_WIDTH) / 2, (float)(maze_rows * TILE_HEIGHT) / 2 };
    camera.offset = { (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 };
    camera.rotation = 0.0f;
    float scale_x = (float)GetScreenWidth() / (maze_cols * TILE_WIDTH);
    float scale_y = (float)GetScreenHeight() / (maze_rows * TILE_HEIGHT);
    camera.zoom = std::min(scale_x, scale_y) * 0.8f;
    return camera;
}

// 主函数：添加路径显示控制
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
        // 随机迷宫选择（占位）
        else if (current_state == GameState::RANDOM_MAZE_SELECT) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                current_state = GameState::MENU;
            }
        }
        // 游戏进行中：添加路径控制
        else if (current_state == GameState::GAME_PLAYING) {
            // 路径显示控制
            if (IsKeyPressed(KEY_ZERO)) {
                maze->set_current_path(PathType::NONE);
            }
            else if (IsKeyPressed(KEY_ONE)) {
                maze->set_current_path(PathType::DFS);
            }
            else if (IsKeyPressed(KEY_TWO)) {
                maze->set_current_path(PathType::BFS);
            }
            else if (IsKeyPressed(KEY_THREE)) {
                maze->set_current_path(PathType::DIJKSTRA);
            }

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
            DrawText("FPS: {}", GetScreenWidth() - 100, 5, 20, BLACK);
            DrawText("0: Hide Path | 1: DFS | 2: BFS | 3: Dijkstra", 5, 5, 20, BLACK);
            DrawText("Press M to return to Menu", 5, 25, 20, BLACK);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}