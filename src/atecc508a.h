// SPDX-FileCopyrightText: 2021-2023 KUNBUS GmbH <support@kunbus.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

extern int atecc508a_serial(const char *dev_path, const unsigned int dev_addr,
                            uint8_t *serial);
