# SPDX-FileCopyrightText: 2023 KUNBUS GmbH <support@kunbus.com>
#
# SPDX-License-Identifier: GPL-2.0-or-later

install(FILES modules-load.d/piserial.conf
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/modules-load.d/
)
