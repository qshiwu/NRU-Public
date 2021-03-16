[Usage]
(for both x86 & ARM)
1. cd to /driver/x86 or /driver/ARM_TX2:
 - use root
 - $ make clean
   $ make
   $ make install

2. cd to /tools
 - use root
 - $ make clean
   $ make
   $ make install
   $ emp2init

After these 2 steps, the driver has been loaded


[Others]
1. if driver first build on ARM TX2:
cd to /usr/src/linux-header-`uname -r`
$ make modules_prepare

2. disable 8250 on x86:
 - 1) cd to /etc/default, then modify "grub":
   add "8250.nr_uarts=0" in GRUB_CMDLINE_LINUX_DEFAULT=
 - 2) $ grub2-mkconfig -o /boot/grub2/grub.cfg (Fedora)
      $ update-grub (Ubuntu)
 - 3) Reboot

3. disable 8250 on ARM TX2:
must rebuild kernel image
 - menuconfig location: Device Drivers->Character devices->Serial drivers->8250/16550 PCI device support (un-check)
 - Replace the new Image to /boot/Image
 - Reboot

4. if kernel version >= 4.11, you must unbind native "8250_exar" kernel driver:
$ sudo mv /lib/modules/`uname -r`/kernel/driver/tty/serial/8250/8250_exar.ko /root/
$ sudo depmod -a