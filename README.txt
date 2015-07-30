---
title: DRA7xx Bootswitch User Guide
version: 1.0
date: 18 October 2016
---

Purpose
=======

This document is the user guide of the DRA7xx Bootswitch
tool. Bootswitch can be used to

1. Load `MLO` to a DRA7xx EVM via peripheral boot. (or)
2. Control the boot media from which the DRA7xx EVM boots.

This document uses the term `MLO` to refer to the first stage
bootloader as this tool was developed while working with
U-Boot. However it can be used with other first stage boot loaders as
well.

This tool runs on Linux but is based on libusb which is cross
platform.  It _should_ be possible to run this on Windows as well.

Please refer to manifest.html for license information.

Usage
=====


1. Clone the `bootswitch` git repository. Run `./install.sh` inside the cloned repository.

    ~~~{.bash}
    host $ git clone git://git.ti.com/glsdk/dra7xx-bootswitch.git
    host $ cd dra7xx-bootswitch
    host $ ./install.sh
    Installing dependencies
    Building
    Build successful
    Setting up udev rules
    Reloading udev rules
    Copying default settings to /tmp/bootsetting.txt
    ~~~

    The script `install.sh` does the following.

    1. Install build dependencies.
    2. Build the software
    3. Setup udev rules to autotrigger bootswitch on detecting a DRA7xx EVM.
    4. Setup default settings in `/tmp/bootsetting.txt`

2. Place the DRA7xx EVM in USB peripheral boot mode.

    ~~~
    SYSBOOT[5:0] = 0b010000.
    ~~~

3. Connect a USB cable from the boot USB port(P2) to your PC.

1. Reboot the EVM. In peripheral boot mode, the EVM waits for a
    command from the PC to determine how to obtain `MLO`. `bootswitch`
    binary provides this command to the EVM.

    The tool expects a configuration file to be found at
    `/tmp/bootsetting.txt`.  If no configuration file is found, the
    default is to boot the EVM from SD card. The default configuration
    setup by `install.sh` is also setup for SD boot.


Controlling the boot
====================

The tool expects configuration file to be found at
`/tmp/bootsetting.txt`.  This configuration file contains information
on how to control the boot. A default should be setup by the
`install.sh` script.  If not, please copy the sample configuration
file `bootsetting.txt` to `/tmp` on the host PC. The file
`bootsetting.txt` contains documentation as well. The below commands
strips the documentation when copying it to the final location.

~~~ {.bash}
host $ sed -e '/^#/d' bootsetting.txt > /tmp/bootsetting.txt
~~~

or create `/tmp/bootsetting.txt` with just the below two
lines.

~~~
0:5
/home/user/u-boot/spl/u-boot-spl.bin
~~~

This configuration tells the tool that we will not be transferring the
binary over USB and that the EVM should boot from the SD card. The
second line containing the file path is a dont care. Reboot the EVM
and observe it booting from SD card.

To change the boot modes, please refer to the documentation in
`bootsetting.txt` in the `bootswitch` git repository.  Please make
sure that any setting changes are performed in `/tmp/bootsetting.txt`
and not in './bootsetting.txt'.

Transferring MLO from PC
========================

To send the first stage boot loader (MLO/SPL) from the PC to the EVM,
modify `/tmp/bootsetting.txt` as follows.

~~~
1:5
/home/user/u-boot/spl/u-boot-spl.bin
~~~

This setting tells the tool that we are using peripheral boot and that
the MLO/SPL binary should be picked up from the described path. The
value after `:` on the first line is a dont care when transferring MLO
from PC. Please note that the binary to be supplied is
`u-boot-spl.bin` and not `MLO`.

Important points
----------------

 a) The binary is loaded to 0x40300000 as per the TRM.
 b) The binary is expected to be in the .bin format. For MLO, you need to point
 to u-boot-spl.bin and **not** the MLO.
 c) The size of the binary should be less than 504 KB.


For more information, please refer to the documentation in
`bootsetting.txt` in the `bootswitch` git repository.

Debug traces
============

The tool generates debug output in `/tmp/bootswitch_log.txt`. Please
use this for debugging any issues.
