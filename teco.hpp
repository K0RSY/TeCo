/*
TeCo - one-file-headder C++ terminal and gui game engine
*/

#include <ncurses.h>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>

#define unfduration std::chrono::nanoseconds
#define second_ratio 1000000000L
#define unftime() std::chrono::time_point_cast<unfduration>(std::chrono::system_clock::now())

#ifdef _WIN32
#include <Windows.h>
#define unfsleep(sleep) usleep((sleep * 1000L) / second_ratio)

#else
#include <unistd.h>
#define unfsleep(sleep) usleep((sleep * 1000000L) / second_ratio) 

#endif

namespace teco {

// functions inits
void init(void (*) (), int, int, int);
void exit();
void tick();

void mainloop();

void draw();

bool is_key_pressed(int);

void playsound(char[]);

// enums
enum {
    GUI = 0,
    TUI = 1
};

enum {
    LOOPING = 0,
    STOP_ON_FIRST_FRAME = 1,
    STOP_ON_LAST_FRAME = 2
};

// variables
int fps;
int tps;
auto tick_slice = unfduration::zero();
auto draw_slice = unfduration::zero();

auto last_update_time = unftime();
auto accumulator = unfduration::zero();

void (*tick_function) ();

int graphics_type;

int c = 0;

std::vector<int> pressed_keys;

bool run = true;

// classes
class Source {
public:
    Source(char symbols_path[], char colors_path[]) {
    }
};

class Animation {
public:
    Animation(Source sources[], int loop_mode, int draws_per_frame) {
    }
};

class Sprite {
public:
    Sprite(int x, int y, Animation animations[], int default_animation);
};

std::vector<Sprite*> sprites;

Sprite::Sprite(int x, int y, Animation animations[], int default_animation) {
    sprites.push_back(this);
}

// functions
void init(void (*_tick_function) (), int _graphics_type = TUI, int _fps = 60, int _tps = 20) {
    graphics_type = _graphics_type;
    fps = _fps;
    tps = _tps;
    tick_slice = unfduration(second_ratio / tps);
    draw_slice = unfduration(second_ratio / fps);
    tick_function = _tick_function;

    if (graphics_type == TUI) {
        initscr();
        curs_set(0);
        
        noecho();
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);
    } else {
        exit();
    }
}

void tick() {
    tick_function();
    c++;
}

bool is_key_pressed(int key) {
    return std::find(pressed_keys.begin(), pressed_keys.end(), key) != pressed_keys.end();
}

void mainloop() {
    while (run) {
        auto delta_time = unftime() - last_update_time;
        last_update_time += delta_time;
        accumulator += delta_time;
        
        while (accumulator > tick_slice) {
            tick();
            accumulator -= tick_slice;
        }
        
        draw();
        
        if (delta_time < draw_slice)
            unfsleep((draw_slice - delta_time).count());
    }
    endwin();
}

void draw() {
    if (graphics_type == TUI) {
        // check pressed keys
        int ch = getch();
        pressed_keys.push_back(ch);
        flushinp();
        
        // draw sprites
        mvprintw(4, 2, "%d", c);
        
        refresh();
        clear();
    }
}

void playsound(char path_to_sound[]) {
    if (graphics_type == GUI) {}
}

void exit() {
    run = false;
}

}