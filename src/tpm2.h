#ifndef TPM2_H
#define TPM2_h

#include <stdint.h>

extern int tpm2_serial(const char *dev_path, uint8_t * serial);

#endif