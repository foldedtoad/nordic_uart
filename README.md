# nordic_uart
An implementation of nordic_uart for Zephyr RTOS.

This was build with Zephyr Version 2.4.99.

It has been tested with Nordic's "nRF UART v2.0" (see Android Play Store).

To configure, run ./configure.sh from the project root.
Then "cd build" and "make".

The default build is for the PCA10028, but the CMakefile.txt can be modified to build for PCA10040.
