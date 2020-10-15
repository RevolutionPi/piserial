add_executable(piSerial piSerialNum.c read_sernum.c md5.c read_ek.c)
target_link_libraries(piSerial tss2-esys tss2-tcti-device)
install(TARGETS piSerial
  RUNTIME DESTINATION bin
)

install(PROGRAMS ks8851-set-mac revpi-factory-reset
  DESTINATION sbin
)
