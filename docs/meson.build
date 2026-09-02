# SPDX-FileCopyrightText: 2026 KUNBUS GmbH <support@kunbus.com>
#
# SPDX-License-Identifier: GPL-2.0-or-later

scdoc = dependency('scdoc', required: get_option('man-pages'))
if scdoc.found()
  scdoc_prog = find_program(scdoc.get_variable(pkgconfig: 'scdoc'), native: true)
  mandir = get_option('mandir')
  custom_target(
    'piSerial.8',
    input: 'piSerial.8.scd',
    output: 'piSerial.8',
    command: scdoc_prog,
    install: true,
    feed: true,
    capture: true,
    install_dir: '@0@/man8'.format(mandir),
  )
endif
