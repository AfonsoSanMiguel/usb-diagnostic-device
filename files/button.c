#include "button.h"
#include <pigpio.h>
#include <stdio.h>
#include <time.h>

#define DEBOUNCE_MS 50

static int last_state_up = 1;
static int last_state_down = 1;
static int last_state_select = 1;

static unsigned int last_press_time = 0;

static unsigned int get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned int)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

int button_init(void) {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Erro: nao foi possivel inicializar pigpio\n");
        return -1;
    }

    gpioSetMode(BTN_UP, PI_INPUT);
    gpioSetMode(BTN_DOWN, PI_INPUT);
    gpioSetMode(BTN_SELECT, PI_INPUT);

    gpioSetPullUpDown(BTN_UP, PI_PUD_UP);
    gpioSetPullUpDown(BTN_DOWN, PI_PUD_UP);
    gpioSetPullUpDown(BTN_SELECT, PI_PUD_UP);

    return 0;
}

void button_cleanup(void) {
    gpioTerminate();
}

Button button_read(void) {
    unsigned int now = get_time_ms();
    if (now - last_press_time < DEBOUNCE_MS) {
        return BUTTON_NONE;
    }

    int state_up = gpioRead(BTN_UP);
    int state_down = gpioRead(BTN_DOWN);
    int state_select = gpioRead(BTN_SELECT);

    Button result = BUTTON_NONE;

    if (last_state_up == 1 && state_up == 0) {
        result = BUTTON_UP;
        last_press_time = now;
    }
    else if (last_state_down == 1 && state_down == 0) {
        result = BUTTON_DOWN;
        last_press_time = now;
    }
    else if (last_state_select == 1 && state_select == 0) {
        result = BUTTON_SELECT;
        last_press_time = now;
    }

    last_state_up = state_up;
    last_state_down = state_down;
    last_state_select = state_select;

    return result;
}
