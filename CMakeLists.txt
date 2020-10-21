cmake_minimum_required(VERSION 3.13.1)

set(BOARD nrf51dk_nrf51422)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

