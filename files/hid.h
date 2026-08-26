#ifndef HID_H
#define HID_H

int  hid_init(void);
void hid_cleanup(void);

int hid_type(const char *text);
int hid_enter(void);
int hid_open_terminal(void);

int hid_set_layout(const char *code);

int hid_ctrl(char c);

const char* hid_get_layout(void);

#endif
