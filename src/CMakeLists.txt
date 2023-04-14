# SPDX-License-Identifier: GPL-2.0-or-later

add_executable(revpi-serial revpi-serial.c atecc508a.c tpm2.c)
target_link_libraries(revpi-serial tss2-esys tss2-tcti-device)
install(TARGETS revpi-serial
  RUNTIME DESTINATION sbin
)

install(PROGRAMS piSerial
  DESTINATION sbin
)

