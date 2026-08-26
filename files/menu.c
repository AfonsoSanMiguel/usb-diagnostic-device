#include "menu.h"
#include "screen.h"
#include "scripts.h"
#include <string.h>
#include <stdio.h>
#include "creds.h"
#define VISIBLE_ITEMS 5

#define LAYOUT_DEFAULT_CURSOR 1

static MenuState current_state;
static int cursor;
static int scroll_offset;
static int last_result;
static int machine_cursor;
static int machine_scroll;
static int pending_sudo_index;
static const char *last_script_name;

static int layout_cursor;
static int layout_scroll;

static int result_mode;
static int last_layout_found;

void menu_init(void) {
    current_state = STATE_MENU;
    cursor = 0;
    scroll_offset = 0;
    last_result = 0;
    machine_cursor = 0;
    machine_scroll = 0;
    pending_sudo_index = -1;
    last_script_name = "";
    layout_cursor = LAYOUT_DEFAULT_CURSOR;
    layout_scroll = 0;
    result_mode = 0;
    last_layout_found = -1;
}

static void adjust_scroll(int cur, int *scroll) {
    if (cur < *scroll) {
        *scroll = cur;
    } else if (cur >= *scroll + VISIBLE_ITEMS) {
        *scroll = cur - VISIBLE_ITEMS + 1;
    }
}

static void draw_menu_screen(void) {
    screen_clear();
    screen_draw_text(0, 0, "== SCRIPTS ==", 0);

    char lay[16];
    scripts_get_layout(lay, sizeof(lay));
    char head[24];
    snprintf(head, sizeof(head), "Layout: %s", lay);
    screen_draw_text(1, 0, head, 0);

    int total = scripts_count() + 1;

    for (int i = 0; i < VISIBLE_ITEMS; i++) {
        int idx = scroll_offset + i;
        if (idx >= total) break;

        int line = 2 + i;
        int inverted = (idx == cursor) ? 1 : 0;

        const char *label;
        if (idx < scripts_count()) {
            label = scripts_get(idx)->name;
        } else {
            label = "Configurar Layout";
        }

        char line_text[32];
        snprintf(line_text, sizeof(line_text), "%s%s",
                 inverted ? "> " : "  ", label);
        screen_draw_text(line, 0, line_text, inverted);
    }

    screen_draw_text(7, 0, "UP/DOWN/SELECT", 0);
    screen_update();
}

static void draw_layout_screen(void) {
    screen_clear();
    screen_draw_text(0, 0, "== LAYOUT ==", 0);

    int total = scripts_layout_count() + 1;

    for (int i = 0; i < VISIBLE_ITEMS; i++) {
        int idx = layout_scroll + i;
        if (idx >= total) break;

        int line = 2 + i;
        int inverted = (idx == layout_cursor) ? 1 : 0;

        const char *label;
        if (idx == 0) {
            label = "Nao sei (auto)";
        } else {
            label = scripts_layout_name(idx - 1);
        }

        char line_text[32];
        snprintf(line_text, sizeof(line_text), "%s%s",
                 inverted ? "> " : "  ", label);
        screen_draw_text(line, 0, line_text, inverted);
    }

    screen_draw_text(7, 0, "SELECT p/ escolher", 0);
    screen_update();
}

static void draw_running_screen(void) {
    screen_clear();
    screen_draw_text(0, 0, "== A EXECUTAR ==", 0);
    screen_draw_text(3, 0, last_script_name, 0);
    screen_draw_text(5, 0, "Aguarde...", 0);
    screen_update();
}

static void layout_test_progress(const char *code, int attempt, int total) {
    screen_clear();
    screen_draw_text(0, 0, "== AUTO LAYOUT ==", 0);

    char line[32];
    snprintf(line, sizeof(line), "A testar: %s", code);
    screen_draw_text(3, 0, line, 0);

    snprintf(line, sizeof(line), "(%d/%d)", attempt, total);
    screen_draw_text(5, 0, line, 0);

    screen_update();
}

static void draw_result_screen(void) {
    screen_clear();

    if (result_mode == 1) {

        screen_draw_text(0, 0, "== AUTO LAYOUT ==", 0);
        if (last_layout_found >= 0) {
            const char *name = scripts_autodetect_name(last_layout_found);
            screen_draw_text(2, 0, "Layout detetado:", 0);
            char line[32];
            snprintf(line, sizeof(line), ">>> %s <<<", name ? name : "?");
            screen_draw_text(4, 0, line, 0);
            screen_draw_text(7, 0, "SELECT p/ menu", 0);
	} else if (last_result == -4) {
        screen_draw_text(4, 0, "SEM CREDENCIAL", 0);
        screen_draw_text(5, 0, "Config no Pi", 0);
	} else {
            screen_draw_text(2, 0, "Nenhum layout", 0);
            screen_draw_text(3, 0, "funcionou.", 0);
            screen_draw_text(7, 0, "SELECT p/ voltar", 0);
        }
        screen_update();
        return;
    }

    screen_draw_text(0, 0, "== RESULTADO ==", 0);
    screen_draw_text(2, 0, last_script_name, 0);

    if (last_result == 0) {
        screen_draw_text(4, 0, "SUCESSO", 0);
        screen_draw_text(7, 0, "SELECT p/ voltar", 0);
    } else if (last_result == -2) {
        screen_draw_text(4, 0, "ADULTERADO", 0);
        screen_draw_text(5, 0, "Script invalido!", 0);
        screen_draw_text(7, 0, "SELECT p/ voltar", 0);
    } else if (last_result == -4) {
        screen_draw_text(4, 0, "SEM CREDENCIAL", 0);
        screen_draw_text(5, 0, "Config no Pi", 0);
        screen_draw_text(7, 0, "SELECT p/ voltar", 0);
    } else if (last_result == -5) {
        screen_draw_text(4, 0, "LAYOUT ERRADO", 0);
        screen_draw_text(5, 0, "Corrigir layout", 0);
        screen_draw_text(7, 0, "SELECT: layout", 0);
    } else if (last_result == -6) {
        screen_draw_text(4, 0, "PASSWORD ERRADA", 0);
        screen_draw_text(5, 0, "Verificar no Pi", 0);
        screen_draw_text(7, 0, "SELECT p/ voltar", 0);
    } else {
        screen_draw_text(4, 0, "ERRO", 0);
        screen_draw_text(5, 0, "Ver output.log", 0);
        screen_draw_text(7, 0, "SELECT p/ voltar", 0);
    }
    screen_update();
}

static void draw_machine_screen(void) {
    screen_clear();
    screen_draw_text(0, 0, "== MAQUINA ==", 0);

    if (creds_machine_count() == 0) {
        screen_draw_text(2, 0, "Sem maquinas", 0);
        screen_draw_text(3, 0, "Configure no Pi", 0);
        screen_draw_text(7, 0, "SELECT p/ voltar", 0);
        screen_update();
        return;
    }

    int total = creds_machine_count() + 1;
    for (int i = 0; i < VISIBLE_ITEMS; i++) {
        int idx = machine_scroll + i;
        if (idx >= total) break;
        int line = 2 + i;
        int inverted = (idx == machine_cursor) ? 1 : 0;
        const char *label = (idx == 0) ? "< Voltar"
                                       : creds_machine_name(idx - 1);
        char t[32];
        snprintf(t, sizeof(t), "%s%s", inverted ? "> " : "  ", label);
        screen_draw_text(line, 0, t, inverted);
    }
    screen_draw_text(7, 0, "SELECT p/ escolher", 0);
    screen_update();
}

void menu_draw(void) {
    switch (current_state) {
        case STATE_MENU:    draw_menu_screen();    break;
        case STATE_RUNNING: draw_running_screen(); break;
        case STATE_RESULT:  draw_result_screen();  break;
        case STATE_LAYOUT:  draw_layout_screen();  break;
        case STATE_MACHINE_SELECT: draw_machine_screen(); break;
   }
}

void menu_handle_button(Button btn) {
    if (btn == BUTTON_NONE) return;

    switch (current_state) {
        case STATE_MENU: {
            int total = scripts_count() + 1;

            if (btn == BUTTON_UP) {
                cursor--;
                if (cursor < 0) cursor = total - 1;
                adjust_scroll(cursor, &scroll_offset);
            } else if (btn == BUTTON_DOWN) {
                cursor++;
                if (cursor >= total) cursor = 0;
                adjust_scroll(cursor, &scroll_offset);
            } else if (btn == BUTTON_SELECT) {
		if (cursor < scripts_count()) {
                    const Script *s = scripts_get(cursor);
                    if (s != NULL) {
                        if (s->needs_sudo) {

                            pending_sudo_index = cursor;
                            machine_cursor = 0;
                            machine_scroll = 0;
                            current_state = STATE_MACHINE_SELECT;
                        } else {

                            last_script_name = s->name;
                            current_state = STATE_RUNNING;
                            draw_running_screen();
                            last_result = scripts_run(cursor);
                            result_mode = 0;
                            current_state = STATE_RESULT;
                        }
                    }
                } else {

                    layout_cursor = LAYOUT_DEFAULT_CURSOR;
                    layout_scroll = 0;
                    adjust_scroll(layout_cursor, &layout_scroll);
                    current_state = STATE_LAYOUT;
                }
            }
            break;
        }

        case STATE_LAYOUT: {
            int total = scripts_layout_count() + 1;

            if (btn == BUTTON_UP) {
                layout_cursor--;
                if (layout_cursor < 0) layout_cursor = total - 1;
                adjust_scroll(layout_cursor, &layout_scroll);
            } else if (btn == BUTTON_DOWN) {
                layout_cursor++;
                if (layout_cursor >= total) layout_cursor = 0;
                adjust_scroll(layout_cursor, &layout_scroll);
            } else if (btn == BUTTON_SELECT) {
                if (layout_cursor == 0) {

                    current_state = STATE_RUNNING;
                    last_layout_found =
                        scripts_run_autodetect(layout_test_progress);
                    result_mode = 1;
                    current_state = STATE_RESULT;
                } else {

                    scripts_set_layout(scripts_layout_name(layout_cursor - 1));
                    current_state = STATE_MENU;
                }
            }
            break;
        }

	case STATE_MACHINE_SELECT: {
            if (creds_machine_count() == 0) {
                if (btn == BUTTON_SELECT) current_state = STATE_MENU;
                break;
            }
            int total = creds_machine_count() + 1;
            if (btn == BUTTON_UP) {
                machine_cursor--;
                if (machine_cursor < 0) machine_cursor = total - 1;
                adjust_scroll(machine_cursor, &machine_scroll);
            } else if (btn == BUTTON_DOWN) {
                machine_cursor++;
                if (machine_cursor >= total) machine_cursor = 0;
                adjust_scroll(machine_cursor, &machine_scroll);
            } else if (btn == BUTTON_SELECT) {
                if (machine_cursor == 0) {
                    current_state = STATE_MENU;
                } else {
                    const Script *s = scripts_get(pending_sudo_index);
                    last_script_name = s ? s->name : "";
                    current_state = STATE_RUNNING;
                    draw_running_screen();
                    last_result = scripts_run_sudo(pending_sudo_index,
                                                   machine_cursor - 1);
                    result_mode = 0;
                    current_state = STATE_RESULT;
                }
            }
            break;
        }

        case STATE_RUNNING:
            break;

	case STATE_RESULT:
            if (btn == BUTTON_SELECT) {

                int to_layout =
                    (result_mode == 1 && last_layout_found < 0) ||
                    (result_mode == 0 && last_result == -5);

                result_mode = 0;

                if (to_layout) {
                    layout_cursor = LAYOUT_DEFAULT_CURSOR;
                    layout_scroll = 0;
                    adjust_scroll(layout_cursor, &layout_scroll);
                    current_state = STATE_LAYOUT;
                } else {
                    current_state = STATE_MENU;
                }
            }
            break;
    }
}
