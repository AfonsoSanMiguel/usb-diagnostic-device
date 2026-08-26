#ifndef CREDS_H
#define CREDS_H

#include <stddef.h>

int creds_machine_count(void);

const char* creds_machine_name(int index);

int creds_get_password(int index, char *out, size_t outsize);

#endif
