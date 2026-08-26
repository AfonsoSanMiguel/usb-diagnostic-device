#ifndef SCREEN_H
#define SCREEN_H

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define FONT_WIDTH    6
#define FONT_HEIGHT   8
#define SCREEN_LINES  (SCREEN_HEIGHT / FONT_HEIGHT)

int  screen_init(void);
void screen_cleanup(void);

void screen_clear(void);
void screen_update(void);

void screen_draw_text(int line, int col, const char *text, int inverted);

void screen_draw_bar(int page, int percent);

#endif
