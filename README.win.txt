---
title: DRA7xx Bootswitch User Guide
version: 1.1
date: 24 July 2017
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

This tool runs on Linux and Windows. Windows installation and usage
instructions follow. For Linux usage instructions, please refer to
the file `README.txt`.

Please refer to manifest.html for license information.

Usage
=====

1. Clone the `dra7xx-bootswitch` git repository.

2. Download the latest `libusb` windows binaries from <http://libusb.info/>.

3. Extract the files into a folder called `libusb` underneath the
`dra7xx-bootswitch` source directory. At the end of this operation,
the below paths must be present under `dra7xx-bootswitch` directory.

    1. `./libusb/include/libusb-1.0`
    2. `./libusb/MinGW64/static`

4. Install `MinGW64` compiler chain from sourceforge. Navigate to
<https://sourceforge.net/projects/mingw-w64/files/> and use the online
installer to install the tool chain.

5. Ensure `gcc` and `mingw32-make` installed in the above step are
available from windows path.

6. Run the below command to build the tool.

    ~~~
    mingw32-make -f Makefile.win
    ~~~

USB Driver setup
----------------

To ensure that the EVM can be accessed using libusb, drivers need to
be installed correctly.

1. Download `zadig` from <http://zadig.akeo.ie/>

2. Run `zadig`.

3. Place the DRA7xx EVM in USB peripheral boot mode.

    ~~~
    SYSBOOT[5:0] = 0b010000.
    ~~~

4. Connect a USB cable from the boot USB port(P2) to your PC.

5. Power on/Reboot the EVM.

6. The zadig window will show the name **VAYU** briefly. Click the
"Install Driver" button as soon as the name is displayed. If the UI
does not indicate the driver is installed successfully, repeat steps
5 and 6. You may want to open "Device manager" and uninstall the
"VAYU" device before trying again.


Running the tool
----------------

1. Place the DRA7xx EVM in USB peripheral boot mode. You would have done this
when doing the [USB Driver Setup].

    ~~~
    SYSBOOT[5:0] = 0b010000.
    ~~~

2. Connect a USB cable from the boot USB port(P2) to your PC.

3. Create a directory `C:\temp` and make sure that it is read/writable.

4. Run `bootswitch` on the command line. There are no command line
arguments. The tool will loop for 10 seconds for EVM to enumerate on
the USB port.

5. Power on/Reboot on the EVM. In peripheral boot mode, the EVM
    enumerates on the USB port and waits for a command from the PC to
    determine how to obtain `MLO`. `bootswitch` binary provides this
    command to the EVM.

    The tool expects a configuration file to be found at
    `C:\temp\bootsetting.txt`.  If no configuration file is found, the
    default is to boot the EVM from SD card.

    You can see the logs from the tool in
    `C:\temp\bootswitch_log.txt`. This tool was originally intended to
    be used from a udev script on Linux. As a result, all the logs are
    stored in the log file. Please review the log if there are any
    issues.

Controlling the boot
====================

The tool expects configuration file with settings to control the boot
to be found at `C:\temp\bootsetting.txt`. The sample configuration
file `bootsetting.txt` in the repository contains documentation of the
settings.

For starting, create `C:\temp\bootsetting.txt` with just the below two
lines.

~~~
0:5
C:\temp\u-boot-spl.bin
~~~

This configuration tells the tool that we will not be transferring the
binary over USB and that the EVM should boot from the SD card. The
second line containing the file path is a dont care.

Run the `bootswitch.exe` binary, reboot the EVM and observe it booting
from SD card.

To change the boot modes, please refer to the documentation in
`bootsetting.txt` in the `bootswitch` git repository.  Please make
sure that any setting changes are performed in `c:\temp\bootsetting.txt`
and not in './bootsetting.txt'.

Transferring MLO from PC
========================

To send the first stage boot loader (MLO/SPL) from the PC to the EVM,
modify `C:\temp\bootsetting.txt` as follows.

~~~
1:5
C:\u-boot\spl\u-boot-spl.bin
~~~

This setting tells the tool that we are using peripheral boot and that
the MLO/SPL binary should be picked up from the described path. The
value after `:` on the first line is a dont care when transferring MLO
from PC. Please note that the binary to be supplied is
`u-boot-spl.bin` and not `MLO`.

Run the `bootswitch.exe` binary, reboot the EVM and observe it booting
with the first stage boot loader binary supplied.

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

The tool generates debug output in `C:\temp\bootswitch_log.txt`. Please
use this for debugging any issues.

Testing
-------

Windows testing was done with laptop running Windows 10, using
libusb-1.0.21 and MinGW GCC version `6.3.0 x86_64-posix-seh-rev1`.

Support
=======

For support, please post any questions to

<https://e2e.ti.com/support/arm/automotive_processors/f/1020>

If you face any issues, please review the log file at
`C:\temp\bootswitch_log.txt` first.
