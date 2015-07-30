#
#  (C) Copyright 2015 - 2016
#  Texas Instruments Incorporated, <www.ti.com>
#
#  Venkateswara Rao Mandela <venkat.mandela@ti.com>
#
#  SPDX-License-Identifier:	BSD-3-Clause
#
SRCFILES=bootswitch.c
OBJDIR=.
OBJFILES=$(patsubst %.c,$(OBJDIR)/%.o,$(SRCFILES))
CFLAGS=$(shell pkg-config --cflags libusb-1.0) -Wall -O3
LDLIBS=$(shell pkg-config --libs libusb-1.0)

bootswitch: $(OBJFILES)

clean:
	rm $(OBJFILES) bootswitch
