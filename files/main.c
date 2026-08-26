#include "button.h"
#include "screen.h"
#include "menu.h"
#include "hid.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static volatile int running = 1;

static void signal_handler(int signum) {
    (void)signum;
    running = 0;
}

int main(void) {
    printf("Iniciando dispositivo de diagnostico...\n");

    if (screen_init() != 0) {
        fprintf(stderr, "Falha a inicializar o ecra\n");
        return 1;
    }

    if (button_init() != 0) {
        fprintf(stderr, "Falha a inicializar os botoes\n");
        screen_cleanup();
        return 1;
    }

    if (hid_init() != 0) {
        fprintf(stderr, "Falha a inicializar o HID\n");
        button_cleanup();
        screen_cleanup();
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    menu_init();
    menu_draw();

    printf("Sistema pronto. A entrar no loop principal.\n");

    while (running) {
        Button btn = button_read();
        if (btn != BUTTON_NONE) {
            menu_handle_button(btn);
            menu_draw();
        }
        usleep(20000);
    }

    printf("\nA terminar...\n");
    screen_clear();
    screen_update();
    hid_cleanup();
    button_cleanup();
    screen_cleanup();

    return 0;
}
