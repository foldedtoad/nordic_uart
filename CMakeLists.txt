cmake_minimum_required(VERSION 3.13.1)

# build PCA10040 board
#set(BOARD nrf52dk_nrf52832)

# build PCA10028 board
set(BOARD nrf51dk_nrf51422)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(nordic_uart)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/nus.c)

target_include_directories(app PRIVATE src)

#target_sources(app PRIVATE ${app_sources})
