#include "teco.hpp"

void tick_tock() {
    if (teco::is_key_pressed(KEY_BACKSPACE))
        teco::exit();
}

int main() {
    teco::init(tick_tock, teco::TUI, 60, 20);
    teco::mainloop();
    return 0;
}
