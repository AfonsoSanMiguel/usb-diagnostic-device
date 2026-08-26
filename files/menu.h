#ifndef MENU_H
#define MENU_H

#include "button.h"

typedef enum {
    STATE_MENU,
    STATE_RUNNING,
    STATE_RESULT,
    STATE_LAYOUT,
    STATE_MACHINE_SELECT
} MenuState;

void menu_init(void);

void menu_draw(void);

void menu_handle_button(Button btn);

#endif
