// SPDX-FileCopyrightText: 2021 KUNBUS GmbH <support@kunbus.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TPM2_H
#define TPM2_h

#include <stdint.h>

extern int tpm2_serial(const char *dev_path, uint8_t * serial);

#endif
