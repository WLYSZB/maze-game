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

// Maze类（完整寻路功能）
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
    std::vector<Coordinate> dfs_path;
    std::vector<Coordinate> bfs_path;
    std::vector<Coordinate> dijkstra_path;
    PathType current_path_type = PathType::NONE;

    bool is_valid(Coordinate c) const {
        return c.x >= 0 && c.x < cols && c.y >= 0 && c.y < rows
            && get_tile_type(c) != TileType::WALL
            && get_tile_type(c) != TileType::LAVA;
    }

    std::vector<Coordinate> get_neighbors(Coordinate c) const {
        std::vector<Coordinate> neighbors;
        if (is_valid({ c.x, c.y - 1 })) neighbors.push_back({ c.x, c.y - 1 });
        if (is_valid({ c.x, c.y + 1 })) neighbors.push_back({ c.x, c.y + 1 });
        if (is_valid({ c.x - 1, c.y })) neighbors.push_back({ c.x - 1, c.y });
        if (is_valid({ c.x + 1, c.y })) neighbors.push_back({ c.x + 1, c.y });
        return neighbors;
    }

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

        Coordinate curr = end_coord;
        while (curr.x != -1 && curr.y != -1) {
            bfs_path.push_back(curr);
            curr = prev[curr.y][curr.x];
        }
        std::reverse(bfs_path.begin(), bfs_path.end());
    }

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

        Coordinate curr = end_coord;
        while (curr.x != -1 && curr.y != -1) {
            dijkstra_path.push_back(curr);
            curr = prev[curr.y][curr.x];
        }
        std::reverse(dijkstra_path.begin(), dijkstra_path.end());
    }

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

        compute_dfs_path();
        compute_bfs_path();
        compute_dijkstra_path();
    }

public:
    Maze(const std::string& filepath) {
        load_textures();
        load_maze(filepath);
    }

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

    void set_current_path(PathType type) {
        current_path_type = type;
    }

    void draw_path_marker(Coordinate c, Color color) const {
        Vector2 pos = get_tile_position(c);
        DrawRectangle(
            static_cast<int>(pos.x + TILE_WIDTH / 2 - 8),
            static_cast<int>(pos.y + TILE_HEIGHT / 2 - 8),
            16, 16,
            color
        );
    }

    void draw(const Camera2D& camera) {
        BeginMode2D(camera);
        for (const auto& row : tiles) {
            for (const auto& tile : row) {
                DrawTextureV(textures[tile.type], tile.position, WHITE);
            }
        }
        switch (current_path_type) {
        case PathType::DFS:
            for (const auto& c : dfs_path) {
                draw_path_marker(c, Color{ 255, 0, 0, 150 });
            }
            break;
        case PathType::BFS:
            for (const auto& c : bfs_path) {
                draw_path_marker(c, Color{ 0, 0, 255, 150 });
            }
            break;
        case PathType::DIJKSTRA:
            for (const auto& c : dijkstra_path) {
                draw_path_marker(c, Color{ 0, 255, 0, 150 });
            }
            break;
        default:
            break;
        }
        EndMode2D();
    }

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

// Player类：实现移动、碰撞检测、状态管理
enum class PlayerState {
    STANDING,
    DOWN,
    LEFT,
    RIGHT,
    UP
};

const float PLAYER_SPEED = 200.0f;
const float PLAYER_FRAME_TIME = 0.1f;

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

    // 确定当前动画帧
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

    // 切换玩家状态（方向）
    void turn(PlayerState new_state) {
        state = new_state;
        curr_frame = 0;
        timer = 0;
    }

    // 移动更新（平滑移动到目标位置）
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

            // 检测是否到达终点
            Coordinate end_coor = maze.get_end_coord();
            if (curr_coor.x == end_coor.x && curr_coor.y == end_coor.y) {
                is_win = true;
            }
        }
    }

    // 键盘控制
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
    Player(const Maze& maze_ref) : maze(maze_ref), is_win(false), is_dead(false), walk_score(0) {
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

    // 更新玩家状态
    void update() {
        control_update();
        if (state != PlayerState::STANDING) {
            determine_frame_rectangle();
            walk_update();
        }
    }

    // 绘制玩家
    void draw(const Camera2D& camera) {
        BeginMode2D(camera);
        DrawTextureRec(texture, curr_frame_rectangle, position, WHITE);
        EndMode2D();
    }

    // 公共接口
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

// 主函数：添加玩家更新和游戏结束检测
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
        else if (current_state == GameState::RANDOM_MAZE_SELECT) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                current_state = GameState::MENU;
            }
        }
        else if (current_state == GameState::GAME_PLAYING) {
            // 路径控制
            if (IsKeyPressed(KEY_ZERO)) maze->set_current_path(PathType::NONE);
            else if (IsKeyPressed(KEY_ONE)) maze->set_current_path(PathType::DFS);
            else if (IsKeyPressed(KEY_TWO)) maze->set_current_path(PathType::BFS);
            else if (IsKeyPressed(KEY_THREE)) maze->set_current_path(PathType::DIJKSTRA);

            // 玩家更新
            if (!player->is_win_state() && !player->is_dead_state()) {
                player->update();
            }
            else {
                // 游戏结束
                current_state = GameState::GAME_OVER;
            }

            if (IsKeyPressed(KEY_M)) {
                current_state = GameState::MENU;
                maze.reset();
                player.reset();
            }
        }
        else if (current_state == GameState::GAME_OVER) {
            // 游戏结束界面（暂未实现选项）
            if (IsKeyPressed(KEY_ENTER)) {
                player->reset();
                current_state = GameState::GAME_PLAYING;
            }
            else if (IsKeyPressed(KEY_ESCAPE)) {
                break;
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
            player->draw(camera);
            DrawText(std::format("FPS: {}", GetFPS()).c_str(), 5, 5, 20, BLACK);
            DrawText(("Cost: " + std::to_string(player->get_score())).c_str(), 5, 25, 20, BLACK);
            DrawText("0: Hide Path | 1: DFS | 2: BFS | 3: Dijkstra", 5, 45, 20, BLACK);
            DrawText("Use arrow keys to move", 5, 65, 20, BLACK);
            DrawText("Press M to return to Menu", 5, 85, 20, BLACK);
        }
        else if (current_state == GameState::GAME_OVER) {
            std::string result = player->is_win_state() ? "YOU WIN!" : "YOU DIED!";
            DrawText(result.c_str(), GetScreenWidth() / 2 - MeasureText(result.c_str(), 60) / 2, 200, 60, player->is_win_state() ? YELLOW : RED);
            DrawText(("Total Cost: " + std::to_string(player->get_score())).c_str(), GetScreenWidth() / 2 - MeasureText(("Total Cost: " + std::to_string(player->get_score())).c_str(), 30) / 2, 300, 30, BLACK);
            DrawText("Press ENTER to replay | ESC to exit", GetScreenWidth() / 2 - MeasureText("Press ENTER to replay | ESC to exit", 25) / 2, 350, 25, BLACK);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}