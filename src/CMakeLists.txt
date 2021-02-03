add_executable(revpi-serial revpi-serial.c atecc508a.c)
install(TARGETS revpi-serial
  RUNTIME DESTINATION sbin
)

install(PROGRAMS ks8851-set-mac revpi-factory-reset piSerial
  DESTINATION sbin
)

install(FILES revpi-factory-reset.sh
  DESTINATION /etc/profile.d)
