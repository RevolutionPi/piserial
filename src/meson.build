# SPDX-FileCopyrightText: 2026 KUNBUS GmbH <support@kunbus.com>
#
# SPDX-License-Identifier: GPL-2.0-or-later

version_tgt = configure_file(
  input: 'version.h.in',
  output: 'version.h',
  configuration: {
    'PROJECT_VERSION': meson.project_version(),
  },
)

tss2_esys_dep = dependency('tss2-esys')
tss2_tcti_device_dep = dependency('tss2-tcti-device')

executable(
  'revpi-serial',
  'revpi-serial.c', 'atecc508a.c', 'tpm2.c',
  include_directories: '.',
  dependencies: [tss2_esys_dep, tss2_tcti_device_dep],
  install: true,
  install_dir: sbindir,
)

install_data(
  'piSerial',
  install_dir: sbindir,
)
