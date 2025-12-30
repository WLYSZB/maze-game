#include "maze_game.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <utility>

bool Coordinate::operator==(const Coordinate& other) const {
    return x == other.x && y == other.y;
}

size_t std::hash<Coordinate>::operator()(const Coordinate& c) const {
    return hash<int>()(c.x) ^ (hash<int>()(c.y) << 1);
}

bool Maze::is_valid(Coordinate c) const {
    return c.x >= 0 && c.x < cols && c.y >= 0 && c.y < rows
        && get_tile_type(c) != TileType::WALL
        && get_tile_type(c) != TileType::LAVA;
}

std::vector<Coordinate> Maze::get_neighbors(Coordinate c) const {
    std::vector<Coordinate> neighbors;
    if (is_valid({ c.x, c.y - 1 })) neighbors.push_back({ c.x, c.y - 1 }); // 上
    if (is_valid({ c.x, c.y + 1 })) neighbors.push_back({ c.x, c.y + 1 }); // 下
    if (is_valid({ c.x - 1, c.y })) neighbors.push_back({ c.x - 1, c.y }); // 左
    if (is_valid({ c.x + 1, c.y })) neighbors.push_back({ c.x + 1, c.y }); // 右
    return neighbors;
}

bool Maze::validate_maze_path() {
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

void Maze::compute_dfs_path() {
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

void Maze::compute_bfs_path() {
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

int Maze::get_tile_cost(TileType type) const {
    return (type == TileType::GRASS) ? 3 : 1;
}

void Maze::compute_dijkstra_path() {
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

void Maze::load_textures() {
    // 加载所有地块纹理
    Image img_end = LoadImage("D:/数据结构/迷宫小游戏/1/assets/end.png");
    textures[TileType::END] = LoadTextureFromImage(img_end);
    UnloadImage(img_end);

    Image img_start = LoadImage("D:/数据结构/迷宫小游戏/1/assets/start.png");
    textures[TileType::START] = LoadTextureFromImage(img_start);
    UnloadImage(img_start);

    Image img_floor = LoadImage("D:/数据结构/迷宫小游戏/1/assets/floor.png");
    textures[TileType::FLOOR] = LoadTextureFromImage(img_floor);
    UnloadImage(img_floor);

    Image img_wall = LoadImage("D:/数据结构/迷宫小游戏/1/assets/wall.png");
    textures[TileType::WALL] = LoadTextureFromImage(img_wall);
    UnloadImage(img_wall);

    Image img_grass = LoadImage("D:/数据结构/迷宫小游戏/1/assets/grass.png");
    textures[TileType::GRASS] = LoadTextureFromImage(img_grass);
    UnloadImage(img_grass);

    Image img_lava = LoadImage("D:/数据结构/迷宫小游戏/1/assets/lava.png");
    textures[TileType::LAVA] = LoadTextureFromImage(img_lava);
    UnloadImage(img_lava);
}

void Maze::load_maze(const std::string& filepath) {
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

void Maze::generate_random_maze(int rows_, int cols_) {
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

Maze::Maze(const std::string& filepath) {
    load_textures();
    load_maze(filepath);
}

Maze::Maze(int rows, int cols) {
    load_textures();
    generate_random_maze(rows, cols);
}

Maze::~Maze() {
    // 释放纹理资源
    for (auto& pair : textures) {
        UnloadTexture(pair.second);
    }
}

void Maze::set_current_path(PathType type) {
    current_path_type = type;
}

void Maze::draw_path_marker(Coordinate c, Color color) const {
    Vector2 pos = get_tile_position(c);
    DrawRectangle(
        static_cast<int>(pos.x + TILE_WIDTH / 2 - 8),
        static_cast<int>(pos.y + TILE_HEIGHT / 2 - 8),
        16, 16,
        color
    );
}

void Maze::draw(const Camera2D& camera) {
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

Coordinate Maze::get_start_coord() const { return start_coord; }
Coordinate Maze::get_end_coord() const { return end_coord; }
TileType Maze::get_tile_type(const Coordinate& coord) const {
    if (coord.y >= 0 && coord.y < rows && coord.x >= 0 && coord.x < cols) {
        return tiles[coord.y][coord.x].type;
    }
    return TileType::WALL;
}
Vector2 Maze::get_tile_position(const Coordinate& coord) const {
    if (coord.y >= 0 && coord.y < rows && coord.x >= 0 && coord.x < cols) {
        return tiles[coord.y][coord.x].position;
    }
    return { 0, 0 };
}
int Maze::get_rows() const { return rows; }
int Maze::get_cols() const { return cols; }