add_executable(piSerial piSerialNum.c read_sernum.c md5.c)
install(TARGETS piSerial
  RUNTIME DESTINATION bin
)

install(PROGRAMS ks8851-set-mac revpi-factory-reset
  DESTINATION sbin
)

install(FILES revpi-factory-reset.sh
  DESTINATION /etc/profile.d)
