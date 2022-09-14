add_executable(revpi-serial revpi-serial.c atecc508a.c tpm2.c)
target_link_libraries(revpi-serial tss2-esys tss2-tcti-device)
install(TARGETS revpi-serial
  RUNTIME DESTINATION sbin
)

install(PROGRAMS lan95xx-set-mac ks8851-set-mac revpi-factory-reset piSerial
  DESTINATION sbin
)

install(FILES revpi-factory-reset.sh
  DESTINATION /etc/profile.d)

add_custom_target(lan9514.bin ALL
  COMMAND xxd -r lan9514.hex lan9514.bin
  DEPENDS /usr/bin/xxd
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
  VERBATIM)

install(FILES lan9514.bin revpi-functions
  DESTINATION /usr/share/revpi)
