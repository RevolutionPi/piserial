#ifndef ATECC508A_H
#define ATECC508A_H

#include <stdint.h>

extern int atecc508a_serial(const char *dev_path, const unsigned int dev_addr,
                            uint8_t *serial);

#endif
