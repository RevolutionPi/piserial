# SPDX-License-Identifier: GPL-2.0-or-later
#
# SPDX-FileCopyrightText: 2021-2024 KUNBUS GmbH

add_custom_target(
	piSerial.8 ALL
	COMMAND scdoc < ${CMAKE_SOURCE_DIR}/docs/piSerial.8.scd > ${PROJECT_BINARY_DIR}/piSerial.8
	DEPENDS ${CMAKE_SOURCE_DIR}/docs/piSerial.8.scd
)

install(FILES ${PROJECT_BINARY_DIR}/piSerial.8
	DESTINATION ${CMAKE_INSTALL_MANDIR}/man8/
)
