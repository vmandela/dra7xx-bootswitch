#!/bin/bash -e
##############################################################################
#
#  (C) Copyright 2015 - 2016
#  Texas Instruments Incorporated, <www.ti.com>
#
#  Venkateswara Rao Mandela <venkat.mandela@ti.com>
#
#  SPDX-License-Identifier:	BSD-3-Clause
#
##############################################################################

echo "Installing dependencies"
sudo apt-get install libusb-1.0-0-dev
echo "Building"
make
if [ ! -e bootswitch ]; then
    echo "Unable to find bootswitch binary after build"
    echo "Exiting"
    exit 1
else
    echo "Build successful"
    BOOTSWITCH_PATH="$(pwd)/bootswitch"
fi

echo "Setting up udev rules"
cat <<EOF | sudo tee /etc/udev/rules.d/72-dra7xx-usbboot.rules
SUBSYSTEM=="usb",ATTRS{idVendor}=="0451",ATTRS{idProduct}=="d013",MODE:="777",RUN+="$BOOTSWITCH_PATH"
SUBSYSTEM=="usb",ATTRS{idVendor}=="0451",ATTRS{idProduct}=="d014",MODE:="777",RUN+="$BOOTSWITCH_PATH"
EOF
echo "Reloading udev rules"
sudo udevadm control --reload-rules
sudo udevadm trigger
echo "Copying default settings to /tmp/bootsetting.txt"
sed -e '/^#/d' bootsetting.txt > /tmp/bootsetting.txt
