# SPDX-FileCopyrightText: KUNBUS GmbH <support@kunbus.com>
#
# SPDX-License-Identifier: GPL-2.0-or-later

cmake_minimum_required(VERSION 3.13)

project(revpi-serial
  VERSION 3.1.0
  LANGUAGES C
)

include(GNUInstallDirs)

add_subdirectory(src)
add_subdirectory(docs)
add_subdirectory(etc)
