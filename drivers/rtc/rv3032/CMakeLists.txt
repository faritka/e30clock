# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_sources(rv3032.c )

zephyr_include_directories(
  ${ZEPHYR_E30CLOCK_MODULE_DIR}/app/include
)
