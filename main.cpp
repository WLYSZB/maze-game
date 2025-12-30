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
#include <random>
#include <chrono>
#include <utility>
#include "raymath.h"
#include <memory>

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
    bool operator==(const Coordinate& other) const {
        return x == other.x && y == other.y;
    }
};

// 为坐标添加哈希函数（用于unordered_map）
template<> struct std::hash<Coordinate> {
    size_t operator()(const Coordinate& c) const {
        return hash<int>()(c.x) ^ (hash<int>()(c.y) << 1);
    }
};

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
    bool is_valid(Coordinate c) const {
        return c.x >= 0 && c.x < cols && c.y >= 0 && c.y < rows
            && get_tile_type(c) != TileType::WALL
            && get_tile_type(c) != TileType::LAVA;
    }

    // 获取四方向邻居（用于路径计算）
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

    void load_textures() {
        // 加载所有地块纹理
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

        // 重新计算路径
        compute_dfs_path();
        compute_bfs_path();
        compute_dijkstra_path();
    }

    // 迷宫生成
    void generate_random_maze(int rows_, int cols_) {
        // 1. 强制迷宫尺寸为奇数
        rows = (rows_ % 2 == 0) ? rows_ + 1 : rows_;
        cols = (cols_ % 2 == 0) ? cols_ + 1 : cols_;
        tiles.resize(rows, std::vector<Tile>(cols));

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dir_dist(0, 3); // 方向随机
        std::uniform_int_distribution<> type_dist(0, 19); // 地块类型概率

        // 2. 初始化：所有位置设为墙
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                tiles[y][x].type = TileType::WALL;
                tiles[y][x].position = {
                    static_cast<float>(x * TILE_WIDTH),
                    static_cast<float>(y * TILE_HEIGHT)
                };
            }
        }

        // 3. 单元格结构：单元格(2i+1,2j+1)，墙在偶数位置
        using Cell = std::pair<int, int>;
        std::vector<std::vector<bool>> visited((rows - 1) / 2, std::vector<bool>((cols - 1) / 2, false));
        std::stack<Cell> stack;

        // 起点单元格（对应迷宫坐标(1,1)）
        Cell start_cell = { 0, 0 };
        stack.push(start_cell);
        visited[start_cell.first][start_cell.second] = true;
        int visited_cells = 1;
        int total_cells = ((rows - 1) / 2) * ((cols - 1) / 2); // 总单元格数

        // 4. 随机DFS生成迷宫
        while (visited_cells < total_cells) {
            Cell curr = stack.top();
            int i = curr.first;
            int j = curr.second;
            std::vector<std::pair<Cell, Cell>> neighbors;

            // 上邻居
            if (i > 0 && !visited[i - 1][j]) {
                neighbors.push_back({ {i - 1, j}, {2 * i, 2 * j + 1} });
            }
            // 下邻居
            if (i < (rows - 1) / 2 - 1 && !visited[i + 1][j]) {
                neighbors.push_back({ {i + 1, j}, {2 * (i + 1), 2 * j + 1} });
            }
            // 左邻居
            if (j > 0 && !visited[i][j - 1]) {
                neighbors.push_back({ {i, j - 1}, {2 * i + 1, 2 * j} });
            }
            // 右邻居
            if (j < (cols - 1) / 2 - 1 && !visited[i][j + 1]) {
                neighbors.push_back({ {i, j + 1}, {2 * i + 1, 2 * (j + 1)} });
            }

            if (!neighbors.empty()) {
                // 随机选邻居
                auto [next_cell, wall_pos] = neighbors[dir_dist(gen) % neighbors.size()];
                int wall_x = wall_pos.second;
                int wall_y = wall_pos.first;
                int cell_x = 2 * next_cell.second + 1;
                int cell_y = 2 * next_cell.first + 1;

                // 打通墙（设为地板）
                tiles[wall_y][wall_x].type = TileType::FLOOR;
                // 标记目标单元格为地板
                tiles[cell_y][cell_x].type = TileType::FLOOR;

                // 访问下一个单元格
                visited[next_cell.first][next_cell.second] = true;
                stack.push(next_cell);
                visited_cells++;
            }
            else {
                stack.pop();
            }
        }

        // 5. 设置起点和终点
        start_coord = { 1, 1 };
        tiles[start_coord.y][start_coord.x].type = TileType::START;
        end_coord = { cols - 2, rows - 2 };
        tiles[end_coord.y][end_coord.x].type = TileType::END;

        // 6. 随机添加草地/熔岩
        std::vector<Coordinate> all_floors;
        for (int y = 1; y < rows - 1; y += 2) { // 仅遍历单元格位置
            for (int x = 1; x < cols - 1; x += 2) {
                Coordinate c = { x, y };
                if (tiles[y][x].type == TileType::FLOOR) {
                    all_floors.push_back(c);
                }
            }
        }

        // 随机修改地板为草地/熔岩
        std::shuffle(all_floors.begin(), all_floors.end(), gen);
        int modify_count = std::min((int)all_floors.size() / 3, 30);
        for (int i = 0; i < modify_count; ++i) {
            Coordinate c = all_floors[i];
            int rand_type = type_dist(gen);
            if (rand_type < 15) { // 75% 草地
                tiles[c.y][c.x].type = TileType::GRASS;
            }
            else if (rand_type < 18) { // 15% 保持地板
                tiles[c.y][c.x].type = TileType::FLOOR;
            }
            else { // 10% 熔岩
                tiles[c.y][c.x].type = TileType::LAVA;
            }
        }

        // 7. 校验路径有效性，若被熔岩阻断则修复
        while (!validate_maze_path()) {
            // 找到最近的熔岩改为地板
            for (int y = 1; y < rows - 1; ++y) {
                for (int x = 1; x < cols - 1; ++x) {
                    if (tiles[y][x].type == TileType::LAVA) {
                        tiles[y][x].type = TileType::FLOOR;
                        if (validate_maze_path()) break;
                    }
                }
                if (validate_maze_path()) break;
            }
        }

        // 8. 重新计算路径
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

    // 随机生成迷宫
    Maze(int rows, int cols) {
        load_textures();
        generate_random_maze(rows, cols);
    }

    ~Maze() {
        // 释放纹理资源
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

    // 带相机的绘制方法
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

    // 公共方法
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

    void determine_frame_rectangle() {
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

    void turn(PlayerState new_state) {
        state = new_state;
        curr_frame = 0;
        timer = 0;
    }

    void walk_update() {
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

    void control_update() {
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

public:
    Player(const Maze& maze_ref) : maze(maze_ref), is_win(false) {
        walk_score = 0;
        is_dead = false;

        // 加载玩家纹理
        Image img_player = LoadImage("D:/数据结构/迷宫小游戏/实验3/assets/character.png");
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

    ~Player() {
        UnloadTexture(texture);
    }

    void update() {
        control_update();
        if (state != PlayerState::STANDING) {
            determine_frame_rectangle();
            walk_update();
        }
    }

    void draw(const Camera2D& camera) {
        BeginMode2D(camera);
        DrawTextureRec(texture, curr_frame_rectangle, position, WHITE);
        EndMode2D();
    }

    int get_score() const { return walk_score; }
    bool is_win_state() const { return is_win; }
    bool is_dead_state() const { return is_dead; }

    void reset() {
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
};

// 计时工具类
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    bool is_running;
    float elapsed_time;

public:
    Timer() : is_running(false), elapsed_time(0.0f) {}

    void start() {
        if (!is_running) {
            start_time = std::chrono::high_resolution_clock::now();
            is_running = true;
        }
    }

    void stop() {
        if (is_running) {
            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> duration = end_time - start_time;
            elapsed_time += duration.count();
            is_running = false;
        }
    }

    void reset() {
        elapsed_time = 0.0f;
        is_running = false;
    }

    float get_elapsed_time() const {
        if (is_running) {
            auto current_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> duration = current_time - start_time;
            return elapsed_time + duration.count();
        }
        return elapsed_time;
    }
};

// 初始化相机（适配迷宫尺寸）
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

// 主函数
int main() {
    // 初始化窗口（可调整大小）
    InitWindow(1280, 720, "Maze Game");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    // 游戏状态管理
    GameState current_state = GameState::MENU;
    GameOverOption selected_option = GameOverOption::REPLAY;
    std::unique_ptr<Maze> maze = nullptr;
    std::unique_ptr<Player> player = nullptr;
    Timer game_timer;
    int selected_difficulty = 0;
    const std::vector<std::pair<int, int>> difficulties = { {15, 15}, {25, 25}, {30, 30} }; // 迷宫难度
    std::string current_maze_path; // 自定义迷宫路径
    int current_random_size[2] = { 15, 15 }; // 随机迷宫尺寸
    Camera2D camera = { 0 };

    while (!WindowShouldClose()) {
        // 全屏切换（F11）拉完了，没鸟用
        if (IsKeyPressed(KEY_F11)) {
            if (IsWindowFullscreen()) {
                SetWindowSize(1280, 720);
                ClearWindowState(FLAG_FULLSCREEN_MODE);
            }
            else {
                SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
                SetWindowState(FLAG_FULLSCREEN_MODE);
            }
            // 重新初始化相机
            if (maze) {
                camera = init_camera(maze->get_cols(), maze->get_rows());
            }
        }

        // 菜单状态
        if (current_state == GameState::MENU) {
            if (IsKeyPressed(KEY_SPACE)) {
                // 加载自定义迷宫
                current_maze_path = "D:/数据结构/迷宫小游戏/实验3/assets/maze0.txt";
                maze = std::make_unique<Maze>(current_maze_path);
                player = std::make_unique<Player>(*maze);
                game_timer.reset();
                game_timer.start();
                camera = init_camera(maze->get_cols(), maze->get_rows());
                current_state = GameState::GAME_PLAYING;
            }
            else if (IsKeyPressed(KEY_Z)) {
                current_state = GameState::RANDOM_MAZE_SELECT;
                selected_difficulty = 0;
            }
            else if (IsKeyPressed(KEY_ESCAPE)) {
                break;
            }
        }
        // 随机迷宫选择
        else if (current_state == GameState::RANDOM_MAZE_SELECT) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                selected_difficulty = (selected_difficulty - 1 + difficulties.size()) % difficulties.size();
            }
            else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                selected_difficulty = (selected_difficulty + 1) % difficulties.size();
            }
            else if (IsKeyPressed(KEY_ENTER)) {
                // 生成选中难度的完美迷宫
                auto [rows, cols] = difficulties[selected_difficulty];
                current_random_size[0] = rows;
                current_random_size[1] = cols;
                maze = std::make_unique<Maze>(rows, cols);
                player = std::make_unique<Player>(*maze);
                game_timer.reset();
                game_timer.start();
                camera = init_camera(maze->get_cols(), maze->get_rows());
                current_state = GameState::GAME_PLAYING;
            }
            else if (IsKeyPressed(KEY_ESCAPE)) {
                current_state = GameState::MENU;
            }
        }
        // 游戏进行中
        else if (current_state == GameState::GAME_PLAYING) {
            // 相机控制：右键拖动视角
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                Vector2 delta = GetMouseDelta();
                delta = Vector2Scale(delta, -1.0f / camera.zoom);
                camera.target = Vector2Add(camera.target, delta);
            }
            // 滚轮缩放
            camera.zoom += ((float)GetMouseWheelMove() * 0.1f);
            camera.zoom = Clamp(camera.zoom, 0.2f, 2.0f); // 限制缩放范围

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

            // 玩家更新
            if (!player->is_win_state() && !player->is_dead_state()) {
                player->update();
            }
            else {
                game_timer.stop();
                current_state = GameState::GAME_OVER;
                selected_option = GameOverOption::REPLAY;
            }

            // 直接返回菜单（M键）
            if (IsKeyPressed(KEY_M)) {
                current_state = GameState::MENU;
                maze.reset();
                player.reset();
            }
        }
        // 游戏结束
        else if (current_state == GameState::GAME_OVER) {
            // 选择结束选项
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                selected_option = static_cast<GameOverOption>((static_cast<int>(selected_option) - 1 + 3) % 3);
            }
            else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                selected_option = static_cast<GameOverOption>((static_cast<int>(selected_option) + 1) % 3);
            }
            // 确认选择
            else if (IsKeyPressed(KEY_ENTER)) {
                if (selected_option == GameOverOption::REPLAY) {
                    // 重玩当前迷宫
                    player->reset();
                    game_timer.reset();
                    game_timer.start();
                    current_state = GameState::GAME_PLAYING;
                }
                else if (selected_option == GameOverOption::MENU) {
                    // 返回主菜单
                    current_state = GameState::MENU;
                    maze.reset();
                    player.reset();
                }
                else if (selected_option == GameOverOption::EXIT) {
                    // 退出游戏
                    break;
                }
            }
            // 直接退出
            else if (IsKeyPressed(KEY_ESCAPE)) {
                break;
            }
        }

        // 绘制逻辑
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (current_state == GameState::MENU) {
            // 绘制主菜单
            DrawText("MAZE GAME", GetScreenWidth() / 2 - MeasureText("MAZE GAME", 60) / 2, 100, 60, BLACK);
            DrawText("Press SPACE to start with custom maze", GetScreenWidth() / 2 - MeasureText("Press SPACE to start with custom maze", 30) / 2, 250, 30, BLACK);
            DrawText("Press Z to select random maze", GetScreenWidth() / 2 - MeasureText("Press Z to select random maze", 30) / 2, 300, 30, BLACK);
            DrawText("Press ESC to exit", GetScreenWidth() / 2 - MeasureText("Press ESC to exit", 30) / 2, 350, 30, BLACK);
            DrawText("F11: Toggle Fullscreen", GetScreenWidth() / 2 - MeasureText("F11: Toggle Fullscreen", 20) / 2, 400, 20, GRAY);
        }
        else if (current_state == GameState::RANDOM_MAZE_SELECT) {
            // 绘制难度选择
            DrawText("SELECT MAZE DIFFICULTY", GetScreenWidth() / 2 - MeasureText("SELECT MAZE DIFFICULTY", 40) / 2, 100, 40, BLACK);

            for (int i = 0; i < difficulties.size(); ++i) {
                std::string text = std::format("{}x{}", difficulties[i].first, difficulties[i].second);
                int y_pos = 200 + i * 60;

                if (i == selected_difficulty) {
                    // 高亮选中项
                    DrawRectangle(
                        GetScreenWidth() / 2 - MeasureText(text.c_str(), 30) / 2 - 20,
                        y_pos - 10,
                        MeasureText(text.c_str(), 30) + 40,
                        40,
                        LIGHTGRAY
                    );
                    DrawText(text.c_str(), GetScreenWidth() / 2 - MeasureText(text.c_str(), 30) / 2, y_pos, 30, RED);
                }
                else {
                    DrawText(text.c_str(), GetScreenWidth() / 2 - MeasureText(text.c_str(), 30) / 2, y_pos, 30, BLACK);
                }
            }

            DrawText("Use arrow keys to select, ENTER to confirm", GetScreenWidth() / 2 - MeasureText("Use arrow keys to select, ENTER to confirm", 20) / 2, 400, 20, DARKGRAY);
            DrawText("Press ESC to go back | F11: Fullscreen", GetScreenWidth() / 2 - MeasureText("Press ESC to go back | F11: Fullscreen", 20) / 2, 430, 20, DARKGRAY);
        }
        else if (current_state == GameState::GAME_PLAYING) {
            // 绘制游戏场景
            maze->draw(camera);
            player->draw(camera);

            // 绘制UI
            DrawText(std::format("FPS: {}", GetFPS()).c_str(), 5, 5, 20, BLACK);
            DrawText("Use arrow keys to move", 5, 25, 20, BLACK);
            DrawText(("Cost: " + std::to_string(player->get_score())).c_str(), 5, 45, 20, BLACK);
            DrawText(("Time: " + std::format("{:.2f}s", game_timer.get_elapsed_time())).c_str(), 5, 65, 20, BLACK);
            DrawText("0: Hide Path | 1: DFS | 2: BFS | 3: Dijkstra", 5, 85, 20, BLACK);
            DrawText("Right Mouse: Drag View | Mouse Wheel: Zoom", 5, 105, 20, BLACK);
            DrawText("F11: Fullscreen | M: Return to Menu", 5, 125, 20, BLACK);
        }
        else if (current_state == GameState::GAME_OVER) {
            // 半透明遮罩
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 0, 0, 0, 180 });

            // 游戏结果
            std::string result_title = player->is_win_state() ? "YOU WIN!" : "YOU DIED!";
            Color title_color = player->is_win_state() ? YELLOW : RED;
            DrawText(result_title.c_str(), GetScreenWidth() / 2 - MeasureText(result_title.c_str(), 60) / 2, 100, 60, title_color);

            // 统计信息
            DrawText(("Total Cost: " + std::to_string(player->get_score())).c_str(),
                GetScreenWidth() / 2 - MeasureText(("Total Cost: " + std::to_string(player->get_score())).c_str(), 30) / 2,
                200, 30, WHITE);
            DrawText(("Time: " + std::format("{:.2f}s", game_timer.get_elapsed_time())).c_str(),
                GetScreenWidth() / 2 - MeasureText(("Time: " + std::format("{:.2f}s", game_timer.get_elapsed_time())).c_str(), 30) / 2,
                240, 30, WHITE);

            // 结束选项
            std::vector<std::pair<GameOverOption, std::string>> options = {
                {GameOverOption::REPLAY, "Replay Current Maze"},
                {GameOverOption::MENU, "Return to Main Menu"},
                {GameOverOption::EXIT, "Exit Game"}
            };

            for (int i = 0; i < options.size(); ++i) {
                int y_pos = 300 + i * 50;
                if (static_cast<GameOverOption>(i) == selected_option) {
                    DrawRectangle(
                        GetScreenWidth() / 2 - MeasureText(options[i].second.c_str(), 30) / 2 - 20,
                        y_pos - 10,
                        MeasureText(options[i].second.c_str(), 30) + 40,
                        40,
                        LIGHTGRAY
                    );
                    DrawText(options[i].second.c_str(), GetScreenWidth() / 2 - MeasureText(options[i].second.c_str(), 30) / 2, y_pos, 30, RED);
                }
                else {
                    DrawText(options[i].second.c_str(), GetScreenWidth() / 2 - MeasureText(options[i].second.c_str(), 30) / 2, y_pos, 30, WHITE);
                }
            }

            DrawText("Use arrow keys to select, ENTER to confirm", GetScreenWidth() / 2 - MeasureText("Use arrow keys to select, ENTER to confirm", 20) / 2, 450, 20, LIGHTGRAY);
            DrawText("ESC: Exit Game Directly", GetScreenWidth() / 2 - MeasureText("ESC: Exit Game Directly", 20) / 2, 480, 20, LIGHTGRAY);
        }

        EndDrawing();
    }

    // 释放资源
    CloseWindow();
    return 0;
}