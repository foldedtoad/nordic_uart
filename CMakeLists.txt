# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set_property(GLOBAL APPEND PROPERTY HEX_FILES_TO_MERGE
      ${app_provision_hex})

# build PCA10040 board
set(BOARD nrf52dk_nrf52832)

# build PCA10028 board
#set(BOARD nrf51dk_nrf51422)

find_package(Zephyr)
project(nordic_uart)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/nus.c)

target_include_directories(app PRIVATE src)

# zephyr_compile_options(-save-temps)
