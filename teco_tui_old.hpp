/*
TeCo - one-file-headder C++ terminal and gui game engine
*/

#include <ncurses.h>
#include <SDL2/SDL.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <string>
#include <fstream>

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
void init(void (*) (), int, int, int, int);
void exit();
void tick();

void mainloop();

void process_io_tui();
void process_io_gui();
void handle_events_tui();
void draw_tui();
void handle_events_gui();
void draw_gui();
void playsounds(const char[64]);

bool is_key_pressed(int);

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

enum {
    PROCEDURAL_SPRITES = 0,
    SPRITES = 1
};

// consts
const int STANDARD_BACKGROUND_RED = 0x12; 
const int STANDARD_BACKGROUND_GREEN = 0x12;
const int STANDARD_BACKGROUND_BLUE = 0x12;			
const int STANDARD_WINDOW_WIDTH = 640;
const int STANDARD_WINDOW_HEIGHT = 480;

// variables
int fps;
int tps;
auto tick_slice = unfduration::zero();
auto draw_slice = unfduration::zero();

long tick_counter = 0;

auto last_update_time = unftime();
auto accumulator = unfduration::zero();

void (*tick_function) ();

int graphics_type;

std::vector<int> pressed_keys;

bool run = true;

int layer_count;
  
int window_width = STANDARD_WINDOW_WIDTH;
int window_height = STANDARD_WINDOW_HEIGHT;
		
SDL_DisplayMode display_mode;
SDL_Event event;
SDL_Renderer *renderer = NULL;
SDL_Window *window = NULL;
SDL_Surface *window_surface = NULL;

// classes
class Source {
public:
    std::vector<std::string> symbols;
    std::vector<std::string> colors;

    Source(const char symbols_path[64], const char colors_path[64]) {
        symbols = read_file(symbols_path);
        colors = read_file(colors_path);
    }

    std::vector<std::string> read_file(const char file_name[64]) {
        std::vector<std::string> result;
        std::string line;
     
        std::ifstream in (file_name);
        if (in.is_open()) {
            while (std::getline(in, line)) {
                result.push_back(line);
            }
        }
        in.close();

        return result;
    }
};

class Animation {
public:
    std::vector<Source> sources;
    int loop_mode;
    int ticks_per_frame;

    Animation(std::vector<Source> _sources, int _loop_mode = STOP_ON_FIRST_FRAME, int _ticks_per_frame = 2) {
        sources = _sources;
        loop_mode = _loop_mode;
        ticks_per_frame = _ticks_per_frame;
    }
};

class Sprite {
public:
    int x;
    int y;
    std::vector<Animation> animations;
    int default_animation_index;
    int layer;
    int current_animation_index;
    int current_frame;
    bool is_playing_animations;
    int current_tick;

    Sprite(int, int, std::vector<Animation>, int, int);

    void play_animation(int animation_index) {
        if (animation_index != current_animation_index) {
            current_animation_index = animation_index;
            current_frame = 0;
            current_tick = 0;
        }
    }

    void update_animations() {
        if (is_playing_animations && current_tick++ == 0) {
            if (++current_frame >= animations[current_animation_index].sources.size()) {
                switch (animations[current_animation_index].loop_mode) {
                    case LOOPING:
                        current_frame = 0;
                        break;
                    case STOP_ON_LAST_FRAME:
                        is_playing_animations = false;
                        current_frame--;
                        break;
                    case STOP_ON_FIRST_FRAME:
                        is_playing_animations = false;
                        current_frame = 0;
                        break;
                }
            }
        }
        if (current_tick > animations[current_animation_index].ticks_per_frame - 1) {
            current_tick = 0;
        }
    }
};

std::vector<std::vector<std::vector<Sprite>>> sprites;

Sprite::Sprite(int _x, int _y, std::vector<Animation> _animations, int _default_animation_index = 0, int _layer = 8) {
    x = _x;
    y = _y;
    animations = _animations;
    default_animation_index = _default_animation_index;
    layer = _layer;
    current_animation_index = default_animation_index;
    
    sprites[layer - 1][SPRITES].push_back(*this);
}

// functions
void init(void (*_tick_function) (), int _graphics_type = TUI, int _fps = 60, int _tps = 20, int _layer_count = 8) {
    graphics_type = _graphics_type;
    fps = _fps;
    tps = _tps;
    tick_slice = unfduration(second_ratio / tps);
    draw_slice = unfduration(second_ratio / fps);
    tick_function = _tick_function;
    layer_count = _layer_count;

    for (int layer_index = 0; layer_index < layer_count; layer_index++) {
        std::vector<std::vector<Sprite>> layer {};
        for (int sprite_type_index = 0; sprite_type_index < 3; sprite_type_index++) {
            std::vector<Sprite> sprite_type {};
            layer.push_back(sprite_type);
        }
        sprites.push_back(layer);
    }

    if (graphics_type == TUI) {
        initscr();
        curs_set(0);
        
        noecho();
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);

        #undef process_io
        #define process_io() process_io_tui()

    } else {
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
			std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
			exit();
		}

		window = SDL_CreateWindow(
			"Хранитель света и тайн Балитики",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			STANDARD_WINDOW_WIDTH, STANDARD_WINDOW_HEIGHT,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
		);

		if (window == NULL) {
			std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		}

		renderer = SDL_CreateRenderer(
			window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
		);

		if (renderer == NULL)
		{
			std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		}

		SDL_SetRenderDrawColor(renderer, STANDARD_BACKGROUND_RED, STANDARD_BACKGROUND_GREEN, STANDARD_BACKGROUND_BLUE, 0x00);

		#undef process_io
        #define process_io() process_io_gui()
    }
}

void tick() {
    tick_function();
    tick_counter++;
    for (auto layer : sprites) {
        for (int sprite_type_index = 0; sprite_type_index < layer.size(); sprite_type_index++) {
            for (auto sprite : layer[sprite_type_index]) {
                sprite.update_animations();
            }
        }
    }
}

bool is_key_pressed(int key) {
    bool result = std::find(pressed_keys.begin(), pressed_keys.end(), key) != pressed_keys.end();
    pressed_keys.clear();
    return result;
}

void mainloop() {
    while (run) {
        auto delta_time = unftime() - last_update_time;
        last_update_time = unftime();
        accumulator += delta_time;
        
        while (accumulator > tick_slice) {
            tick();
            accumulator -= tick_slice;
        }

        process_io();
        
        if (delta_time < draw_slice)
            unfsleep((draw_slice - delta_time).count());
    }
    endwin();
}

void process_io_tui() {
	handle_events_tui();
	draw_tui();
}

void process_io_gui() {
	handle_events_gui();
	draw_gui();
    const char *path = "path";
	playsounds(path);
}

void handle_events_tui() {
    int ch = getch();
    pressed_keys.push_back(ch);
    flushinp();
}

void handle_events_gui() {
	while (SDL_PollEvent(&event) != 0) {
		if (event.type == SDL_QUIT) {
			exit();
		}

		else if (event.type == SDL_WINDOWEVENT) {
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				window_width = event.window.data1;
				window_height = event.window.data2;
			}
		}

		else if (event.type == SDL_KEYDOWN) {
			pressed_keys.push_back(event.key.keysym.sym);
		}
	}
}

void draw_tui() {
    for (auto layer : sprites) {
        for (int sprite_type_index = 0; sprite_type_index < layer.size(); sprite_type_index++) {
            for (auto sprite : layer[sprite_type_index]) {
                auto source = sprite.animations[sprite.current_animation_index].sources[sprite.current_frame];
                int x = COLS / 2 + sprite.x - (source.symbols[0].size() / 2);
                int y = LINES / 2 + sprite.y - (source.symbols.size() / 2);

                for (int symbols_line_index = 0; symbols_line_index < source.symbols.size(); symbols_line_index++) {
                    mvprintw(y + symbols_line_index, x, source.symbols[symbols_line_index].c_str());
                }
            }
        }
    }
    
    refresh();
    clear();
}

void draw_gui() {
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);	
}

void playsounds(const char path_to_sound[64]) {
}

void exit() {
    run = false;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

}
