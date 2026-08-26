#include "screen.h"

int main(void) {
    if (screen_init() != 0) return 1;
    screen_clear();
    screen_draw_text(3, 22, "A carregar...", 0);
    screen_update();

    return 0;
}
