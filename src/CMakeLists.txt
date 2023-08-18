# SPDX-FileCopyrightText: KUNBUS GmbH <support@kunbus.com>
#
# SPDX-License-Identifier: GPL-2.0-or-later

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

add_executable(revpi-serial revpi-serial.c atecc508a.c tpm2.c)
target_include_directories(revpi-serial PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(revpi-serial tss2-esys tss2-tcti-device)
install(TARGETS revpi-serial
  RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR}
)

install(PROGRAMS piSerial
  DESTINATION ${CMAKE_INSTALL_SBINDIR}
)

