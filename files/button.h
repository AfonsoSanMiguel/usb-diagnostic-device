#ifndef BUTTON_H
#define BUTTON_H
#define BTN_UP      17
#define BTN_DOWN    27
#define BTN_SELECT  22

typedef enum {
    BUTTON_NONE = 0,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_SELECT
} Button;

int button_init(void);

void button_cleanup(void);
Button button_read(void);

#endif
