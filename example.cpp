#include "teco.hpp"

void tick_tock() {
    if (teco::is_key_pressed(SDLK_BACKSPACE))
        teco::exit();
}

int main() {
    teco::init(tick_tock, teco::GUI, 60, 20, 8);

    const char *symbols = "./test.tcsb"; 
    const char *colors = "./test.tccl";

    teco::Source test_source {symbols, colors};

    std::vector<teco::Source> test_sources {test_source};

    teco::Animation test_animation {test_sources, 1, 2};

    std::vector<teco::Animation> test_animations {test_animation};

    teco::Sprite test_sprite = teco::Sprite(
        0, 0,
        test_animations,
        0,
        8
    ); 
 
	teco::Screen test_screen;
	test_screen.add_sprite(test_sprite);

	for (int line = 0; line <= teco::HEIGHT_IN_SYMBOLS; line++) {
		for (int column = 0; column <= teco::WIDTH_IN_SYMBOLS; column++) {
			if (test_screen.symbols[line][column] != '#') {
				std::cout << " ";
			}
			else {
				std::cout << test_screen.symbols[line][column];
			}
		}
		std::cout << std::endl;
	} 

    teco::mainloop();
    return 0;
}
