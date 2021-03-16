KERNEL_SRC = /lib/modules/`uname -r`/build

all: build

obj-m += xr17v35x.o

xrpci-objs := xr17v35x.o

EXTRA_CFLAGS += -DDEBUG=0

build:
	$(MAKE) -C $(KERNEL_SRC) SUBDIRS=$(PWD) modules

install:
	cp xr17v35x.ko /lib/modules/$(shell uname -r)
	depmod -a

clean:
	rm -f *~ *.o
	rm -f *~ *.ko
	rm -f modules.order Module.symvers xr17v35x.mod.c xr17v35x.o.ur-safe
	rm -f .xr17v35x.ko.cmd .xr17v35x.mod.o.cmd .xr17v35x.o.cmd
	rm -rf .tmp_versions
	rm -f /lib/modules/$(shell uname -r)/xr17v35x.ko