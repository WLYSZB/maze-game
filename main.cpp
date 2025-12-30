#include "maze_game.h"
#include <iostream>
#include <vector>

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
        // 全屏切换（F11）
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
                current_maze_path = "D:/数据结构/迷宫小游戏/1/assets/maze0.txt";
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