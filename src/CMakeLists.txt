add_executable(revpi-serial piSerialNum.c read_sernum.c md5.c read_ek.c)
target_link_libraries(revpi-serial tss2-esys tss2-tcti-device)
install(TARGETS revpi-serial
  RUNTIME DESTINATION bin
)

install(PROGRAMS ks8851-set-mac revpi-factory-reset piSerial
  DESTINATION sbin
)

install(FILES revpi-factory-reset.sh
  DESTINATION /etc/profile.d)
