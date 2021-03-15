# SPDX-License-Identifier: GPL-2.0
#
# Makefile for USB Network drivers
#
obj-m += sim7500_sim7600_wwan.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	
install: all
	mkdir -p /lib/modules/$(shell uname -r)/kernel/drivers/net/usb/
	cp -f sim7500_sim7600_wwan.ko /lib/modules/$(shell uname -r)/kernel/drivers/net/usb/
	depmod
