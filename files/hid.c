#include "hid.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define HID_DEVICE  "/dev/hidg0"
#define LAYOUT_DIR  "/home/raspberrypi/project/files/languages"

static int hid_fd = -1;

typedef struct {
    unsigned char scancode;
    unsigned char modifier;
} HidKey;

static HidKey layout_table[128];

static char current_layout[16] = "us";

int hid_set_layout(const char *code) {
    if (code == NULL) return -1;

    char path[256];
    snprintf(path, sizeof(path), "%s/%s.json", LAYOUT_DIR, code);

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "hid: nao foi possivel abrir %s\n", path);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) { fclose(f); return -1; }

    char *buffer = malloc(size + 1);
    if (buffer == NULL) { fclose(f); return -1; }
    if (fread(buffer, 1, size, f) != (size_t)size) {
        free(buffer); fclose(f); return -1;
    }
    buffer[size] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (root == NULL) {
        fprintf(stderr, "hid: erro de parse em %s\n", path);
        return -1;
    }

    memset(layout_table, 0, sizeof(layout_table));

    cJSON *item = NULL;
    cJSON_ArrayForEach(item, root) {
        const char *key = item->string;
        if (key == NULL) continue;
        if (strlen(key) != 1) continue;
        if (!cJSON_IsString(item)) continue;

        unsigned int mod = 0, res = 0, sc = 0;
        if (sscanf(item->valuestring, "%x,%x,%x", &mod, &res, &sc) == 3) {
            unsigned char idx = (unsigned char)key[0];
            if (idx < 128) {
                layout_table[idx].modifier = (unsigned char)mod;
                layout_table[idx].scancode = (unsigned char)sc;
            }
        }
    }

    cJSON_Delete(root);

    strncpy(current_layout, code, sizeof(current_layout) - 1);
    current_layout[sizeof(current_layout) - 1] = '\0';
    return 0;
}

const char* hid_get_layout(void) {
    return current_layout;
}

static HidKey ascii_to_hid(char c) {
    unsigned char idx = (unsigned char)c;
    if (idx < 128) return layout_table[idx];
    HidKey empty = {0x00, 0x00};
    return empty;
}

static int send_key(unsigned char modifier, unsigned char scancode) {
    unsigned char report[8] = {0};

    report[0] = modifier;
    report[2] = scancode;
    if (write(hid_fd, report, 8) != 8) return -1;

    memset(report, 0, 8);
    if (write(hid_fd, report, 8) != 8) return -1;

    usleep(50000);
    return 0;
}

int hid_init(void) {
    hid_fd = open(HID_DEVICE, O_RDWR);
    if (hid_fd < 0) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", HID_DEVICE);
        return -1;
    }

    if (hid_set_layout("us") != 0) {
        fprintf(stderr, "hid: AVISO - layout 'us' nao carregado. "
                        "Confirma que existe %s/us.json\n", LAYOUT_DIR);
    }
    return 0;
}

void hid_cleanup(void) {
    if (hid_fd >= 0) {
        close(hid_fd);
        hid_fd = -1;
    }
}

int hid_type(const char *text) {
    if (hid_fd < 0 || text == NULL) return -1;

    while (*text != '\0') {
        HidKey k = ascii_to_hid(*text);
        if (k.scancode != 0x00) {
            if (send_key(k.modifier, k.scancode) != 0) return -1;
        }
        text++;
    }
    return 0;
}

int hid_enter(void) {
    return send_key(0x00, 0x28);
}

int hid_ctrl(char c) {
    if (hid_fd < 0) return -1;
    HidKey k = ascii_to_hid(c);
    if (k.scancode == 0x00) return -1;
    return send_key(0x01, k.scancode);
}

int hid_open_terminal(void) {
    unsigned char report[8] = {0};

    report[0] = 0x05;
    report[2] = 0x17;
    if (write(hid_fd, report, 8) != 8) return -1;

    memset(report, 0, 8);
    if (write(hid_fd, report, 8) != 8) return -1;

    usleep(2000000);
    return 0;
}
