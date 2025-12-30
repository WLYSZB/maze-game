#include <raylib.h>
#include <format>
#include "maze.h"
#include "player.h"

int main(){
    InitWindow(800, 480, "Game");
    Maze maze("../assets/maze0.txt");
    Player player(maze);

    while (!WindowShouldClose()){

        player.update();

        BeginDrawing();
            ClearBackground(RAYWHITE);

            maze.draw();
            player.draw();
            
            DrawText(std::format("FPS: {}", GetFPS()).c_str(), 5, 5, 20, WHITE);
            DrawText("Use arrow keys to move", 500, 240, 20, BLACK);
        EndDrawing();
    }
    CloseWindow(); 
    return 0;
}