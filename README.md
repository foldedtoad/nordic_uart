# nordic_uart
An implementation of nordic_uart for Zephyr RTOS.

This was build with Zephyr Version 3.4 on Ubuntu 22.04 LTS.

It has been tested with Nordic's "nRF UART v2.0" (see Android Play Store).
It has also been tested with IoS app "BLE Serial nRF Tiny" (see Apple app store)

To configure, run ./configure.sh from the project root.
Then "cd build" and "make".

The default build is for the PCA10040, but the CMakeList.txt can be modified to build for PCA10028.
