#include "creds.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CRED_FILE  "/home/raspberrypi/project/credentials.enc"
#define KEY_FILE   "/home/raspberrypi/project/.credkey"

#define MAX_MACH   32
#define NAME_LEN   32

static char machine_names[MAX_MACH][NAME_LEN];
static int  machine_count = -1;

static FILE* creds_open(void) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "openssl enc -d -aes-256-cbc -pbkdf2 "
        "-in %s -pass file:%s 2>/dev/null",
        CRED_FILE, KEY_FILE);
    return popen(cmd, "r");
}

static void load_names(void) {
    machine_count = 0;
    FILE *f = creds_open();
    if (f == NULL) return;

    char line[512];
    while (fgets(line, sizeof(line), f) && machine_count < MAX_MACH) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        char *tab = strchr(line, '\t');
        if (tab == NULL) continue;
        size_t nlen = (size_t)(tab - line);
        if (nlen >= NAME_LEN) nlen = NAME_LEN - 1;
        memcpy(machine_names[machine_count], line, nlen);
        machine_names[machine_count][nlen] = '\0';
        machine_count++;
    }
    pclose(f);
}

int creds_machine_count(void) {
    if (machine_count < 0) load_names();
    return machine_count;
}

const char* creds_machine_name(int index) {
    if (machine_count < 0) load_names();
    if (index < 0 || index >= machine_count) return NULL;
    return machine_names[index];
}

int creds_get_password(int index, char *out, size_t outsize) {
    if (out == NULL || outsize == 0) return -1;
    out[0] = '\0';

    if (machine_count < 0) load_names();
    if (index < 0 || index >= machine_count) return -1;

    FILE *f = creds_open();
    if (f == NULL) return -1;

    int cur = 0, found = -1;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        char *tab = strchr(line, '\t');
        if (tab == NULL) continue;
        if (cur == index) {
            char *pw = tab + 1;
            pw[strcspn(pw, "\r\n")] = '\0';
            strncpy(out, pw, outsize - 1);
            out[outsize - 1] = '\0';
            found = 0;

        }
        cur++;
    }
    pclose(f);
    return found;
}
