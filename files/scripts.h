#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <stddef.h>

typedef struct {
    const char *name;
    const char *path;
    const char *prefix;
    int needs_sudo;
} Script;

int scripts_count(void);
const Script* scripts_get(int index);

int scripts_run(int index);

typedef void (*LayoutTestCallback)(const char *layout_code, int attempt, int total);

int scripts_run_autodetect(LayoutTestCallback cb);

const char* scripts_autodetect_name(int index);

void scripts_set_layout(const char *code);

int scripts_layout_count(void);
const char* scripts_layout_name(int index);

void scripts_get_layout(char *out, size_t outsize);

int scripts_run_sudo(int index, int machine_index);
#endif
