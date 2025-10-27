#include "teco.hpp"

void tick_tock() {
    if (teco::is_key_pressed(KEY_BACKSPACE))
        teco::exit();
}

int main() {
    teco::init(tick_tock, teco::TUI, 60, 20, 8);

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

    teco::mainloop();
    return 0;
}
