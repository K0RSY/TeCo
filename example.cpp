#include "teco.hpp"

void tick() {
    if (teco::is_key_pressed(KEY_BACKSPACE))
        teco::exit();
}

int main() {
    teco::init(tick, teco::TUI, 60, 20);
    teco::mainloop();
    return 0;
}
