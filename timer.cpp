#include "maze_game.h"

Timer::Timer() : is_running(false), elapsed_time(0.0f) {}

void Timer::start() {
    if (!is_running) {
        start_time = std::chrono::high_resolution_clock::now();
        is_running = true;
    }
}

void Timer::stop() {
    if (is_running) {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = end_time - start_time;
        elapsed_time += duration.count();
        is_running = false;
    }
}

void Timer::reset() {
    elapsed_time = 0.0f;
    is_running = false;
}

float Timer::get_elapsed_time() const {
    if (is_running) {
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = current_time - start_time;
        return elapsed_time + duration.count();
    }
    return elapsed_time;
}