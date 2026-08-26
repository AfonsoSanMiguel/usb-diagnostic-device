#include "scripts.h"
#include "hid.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "creds.h"
#define USB_IMAGE   "/piusb.bin"
#define PUBLIC_KEY  "/home/raspberrypi/project/public.pem"
#define CONFIG_FILE "/home/raspberrypi/project/config.txt"

#define LAYOUT_TEST_MARKER "DIAGLAYOUTOK"
#define LAYOUT_TEST_FILE   "layouttest.txt"

static const Script script_list[] = {
    { "CPU",              "cpu.sh",        "c",  0 },
    { "RAM",              "ram.sh",        "r",  0 },
    { "Particoes",        "particoes.sh",  "p",  0 },
    { "Processos",        "processos.sh",  "ps", 0 },
    { "Uptime",           "uptime.sh",     "u",  0 },
    { "Logs Sistema",     "logs.sh",       "l",  1 },
    { "Fich. Protegidos", "protegidos.sh", "pf", 1 }
};

static const int script_list_size = sizeof(script_list) / sizeof(script_list[0]);

int scripts_count(void) { return script_list_size; }

const Script* scripts_get(int index) {
    if (index < 0 || index >= script_list_size) return NULL;
    return &script_list[index];
}

static const char *autodetect_order[] = {
    "us", "gb", "pt", "es", "fr", "de", "it", "br", "ch", "be"
};
static const int autodetect_count =
    sizeof(autodetect_order) / sizeof(autodetect_order[0]);

static const char *layout_menu_list[] = {
    "us", "gb", "pt", "es", "fr", "de", "it", "br",
    "ch", "be", "dk", "no", "se", "fi", "hu", "cz",
    "sk", "hr", "si", "tr", "jp"
};
static const int layout_menu_size =
    sizeof(layout_menu_list) / sizeof(layout_menu_list[0]);

int scripts_layout_count(void) { return layout_menu_size; }

const char* scripts_layout_name(int index) {
    if (index < 0 || index >= layout_menu_size) return NULL;
    return layout_menu_list[index];
}

const char* scripts_autodetect_name(int index) {
    if (index < 0 || index >= autodetect_count) return NULL;
    return autodetect_order[index];
}

static void detect_layout_config(char *out, size_t outsize) {
    strncpy(out, "us", outsize - 1);
    out[outsize - 1] = '\0';

    FILE *f = fopen(CONFIG_FILE, "r");
    if (f == NULL) return;

    char buf[64];
    while (fgets(buf, sizeof(buf), f)) {
        char *p = strstr(buf, "layout=");
        if (p != NULL) {
            p += 7;
            size_t i = 0;
            while (p[i] && p[i] != '\n' && p[i] != '\r'
                   && p[i] != ' ' && i < outsize - 1) {
                out[i] = p[i];
                i++;
            }
            out[i] = '\0';
            break;
        }
    }
    fclose(f);
}

static void save_layout_config(const char *code) {
    FILE *f = fopen(CONFIG_FILE, "w");
    if (f == NULL) return;
    fprintf(f, "layout=%s\n", code);
    fclose(f);
}

void scripts_set_layout(const char *code) {
    if (code == NULL) return;
    hid_set_layout(code);
    save_layout_config(code);
}

void scripts_get_layout(char *out, size_t outsize) {
    detect_layout_config(out, outsize);
}

static int verify_signature(const char *script_name) {
    char cmd[1024];

    snprintf(cmd, sizeof(cmd),
        "mcopy -i %s ::/scripts/%s /tmp/verify_script.sh 2>/dev/null",
        USB_IMAGE, script_name);
    system(cmd);

    snprintf(cmd, sizeof(cmd),
        "mcopy -i %s ::/signatures/%s.sig /tmp/verify_script.sig 2>/dev/null",
        USB_IMAGE, script_name);
    system(cmd);

    snprintf(cmd, sizeof(cmd),
        "echo '%s' | cat - /tmp/verify_script.sh > /tmp/verify_input.bin",
        script_name);
    system(cmd);

    snprintf(cmd, sizeof(cmd),
        "openssl dgst -sha256 -verify %s "
        "-signature /tmp/verify_script.sig "
        "/tmp/verify_input.bin > /tmp/verify_result.txt 2>&1",
        PUBLIC_KEY);
    system(cmd);

    FILE *f = fopen("/tmp/verify_result.txt", "r");
    if (f == NULL) return -1;
    char result[64] = {0};
    fgets(result, sizeof(result), f);
    fclose(f);

    system("rm -f /tmp/verify_script.sh /tmp/verify_script.sig /tmp/verify_input.bin /tmp/verify_result.txt");

    if (strstr(result, "Verified OK") != NULL) return 0;
    return -1;
}

static int get_next_run_number(const char *prefix) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "mdir -i %s ::/outputs/ 2>/dev/null | grep -i ' %s[0-9]' | grep -o '%s[0-9]*' | grep -o '[0-9]*' | sort -n | tail -1 > /tmp/run_count.txt",
        USB_IMAGE, prefix, prefix);
    system(cmd);

    FILE *f = fopen("/tmp/run_count.txt", "r");
    if (f == NULL) return 1;
    int last = 0;
    fscanf(f, "%d", &last);
    fclose(f);
    system("rm -f /tmp/run_count.txt");

    return last + 1;
}

static int check_output(const char *log_name) {
    char src[2048];
    snprintf(src, sizeof(src), "::/outputs/%s", log_name);

    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "mcopy -i %s %s /tmp/check_output.txt 2>/dev/null",
        USB_IMAGE, src);

    usleep(2000000);

    system("rm -f /tmp/check_output.txt");
    system(cmd);

    FILE *f = fopen("/tmp/check_output.txt", "r");
    if (f == NULL) {

        return -5;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);

    int result = -1;
    if (size > 0 &&
        system("grep -q '^DIAG_EXIT=0$' /tmp/check_output.txt") == 0) {
        result = 0;
    }

    system("rm -f /tmp/check_output.txt");
    return result;
}

static int type_and_check(const Script *s, const char *log_name) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "cd /media/$(whoami)/PEN; "
        "bash scripts/%s > outputs/%s 2>&1; "
        "echo DIAG_EXIT=$? >> outputs/%s",
        s->path, log_name, log_name);
    hid_type(cmd);
    hid_enter();
    return check_output(log_name);
}

int scripts_run(int index) {
    const Script *s = scripts_get(index);
    if (s == NULL) return -1;

    if (verify_signature(s->path) != 0) return -2;

    int run_number = get_next_run_number(s->prefix);
    char log_name[16];
    snprintf(log_name, sizeof(log_name), "%s%d.log", s->prefix, run_number);

    char layout[16];
    detect_layout_config(layout, sizeof(layout));
    hid_set_layout(layout);

    if (hid_open_terminal() != 0) return -1;
    usleep(1000000);

    return type_and_check(s, log_name);
}

#define SUDO_PROMPT_DELAY_US 1500000
#define SUDO_PROBE_FILE      "sudoprobe.txt"
#define SUDO_PROBE_MARKER    "SUDOLAYOUTOK"

static void delete_sudo_probe(void) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "mdel -i %s ::/outputs/%s 2>/dev/null", USB_IMAGE, SUDO_PROBE_FILE);
    system(cmd);
}

static int check_sudo_probe(void) {
    char cmd[1024];
    system("rm -f /tmp/sudoprobe.txt");
    snprintf(cmd, sizeof(cmd),
        "mcopy -i %s ::/outputs/%s /tmp/sudoprobe.txt 2>/dev/null",
        USB_IMAGE, SUDO_PROBE_FILE);
    usleep(2000000);
    system(cmd);
    int ok = (system("grep -q " SUDO_PROBE_MARKER
                     " /tmp/sudoprobe.txt 2>/dev/null") == 0);
    system("rm -f /tmp/sudoprobe.txt");
    return ok ? 0 : -1;
}

static int type_and_check_sudo(const Script *s, const char *log_name,
                               const char *password) {
    delete_sudo_probe();

    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "cd /media/$(whoami)/PEN; echo %s > outputs/%s; sudo -k; "
        "sudo bash scripts/%s > outputs/%s 2>&1; "
        "echo DIAG_EXIT=$? >> outputs/%s",
        SUDO_PROBE_MARKER, SUDO_PROBE_FILE,
        s->path, log_name, log_name);
    hid_type(cmd);
    hid_enter();

    if (check_sudo_probe() != 0) {

        hid_ctrl('c');
        hid_enter();
        delete_sudo_probe();
        return -5;
    }

    hid_type(password);
    hid_enter();
    delete_sudo_probe();

    int r = check_output(log_name);
    if (r == 0) return 0;

    hid_ctrl('c');
    hid_enter();
    char mk[512];
    snprintf(mk, sizeof(mk),
        "echo DIAG_EXIT=1 >> /media/$(whoami)/PEN/outputs/%s", log_name);
    hid_type(mk);
    hid_enter();

    r = check_output(log_name);
    return r;
}

int scripts_run_sudo(int index, int machine_index) {
    const Script *s = scripts_get(index);
    if (s == NULL || !s->needs_sudo) return -1;

    if (verify_signature(s->path) != 0) return -2;

    char password[256];
    if (creds_get_password(machine_index, password, sizeof(password)) != 0) {
        return -4;
    }

    int run_number = get_next_run_number(s->prefix);
    char log_name[16];
    snprintf(log_name, sizeof(log_name), "%s%d.log", s->prefix, run_number);

    char layout[16];
    detect_layout_config(layout, sizeof(layout));
    hid_set_layout(layout);

    if (hid_open_terminal() != 0) {
        memset(password, 0, sizeof(password));
        return -1;
    }
    usleep(1000000);

    int r = type_and_check_sudo(s, log_name, password);
    memset(password, 0, sizeof(password));

    if (r == -1) r = -6;
    return r;
}

static void delete_layout_test(void) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "mdel -i %s ::/outputs/%s 2>/dev/null", USB_IMAGE, LAYOUT_TEST_FILE);
    system(cmd);
}

static int check_layout_marker(void) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "mcopy -i %s ::/outputs/%s /tmp/layouttest.txt 2>/dev/null",
        USB_IMAGE, LAYOUT_TEST_FILE);

    usleep(2000000);
    system(cmd);

    int found = (system("grep -q " LAYOUT_TEST_MARKER
                        " /tmp/layouttest.txt 2>/dev/null") == 0);
    system("rm -f /tmp/layouttest.txt");
    return found ? 0 : -3;
}

int scripts_run_autodetect(LayoutTestCallback cb) {
    if (hid_open_terminal() != 0) return -1;
    usleep(1000000);

    char cmd[1024];

    for (int i = 0; i < autodetect_count; i++) {
        if (cb != NULL) cb(autodetect_order[i], i + 1, autodetect_count);

        delete_layout_test();
        hid_set_layout(autodetect_order[i]);

        snprintf(cmd, sizeof(cmd),
            "echo %s > /media/$(whoami)/PEN/outputs/%s",
            LAYOUT_TEST_MARKER, LAYOUT_TEST_FILE);
        hid_type(cmd);
        hid_enter();

        if (check_layout_marker() == 0) {
            save_layout_config(autodetect_order[i]);
            delete_layout_test();
            return i;
        }

        hid_enter();
    }

    return -3;
}
