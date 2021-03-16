Install instructions for Simcom SIM7500/SIM7600 series Linux NDIS network driver (based on MDM9x07 chipset) without rebuilding the Linux kernel.
Jörgen Storvist, Techship 2018
https://techship.com/

Should you want to build in the drivers into the Linux kernel, please relate to "SIM7500_SIM7600 Linux NDIS User Guide_V2.01.pdf" document.

All commands are supposed to be executed with elevated system privileges/as root user. 
In order to install the drivers certain pre-requirments are neccessary. 

Ensure that your original kernel was built with the following config options enabled, this will allow the option and usbnet driver pre-requirments to be included in kernel. (usually already included in larger distributions)
CONFIG_USB_SERIAL=y
CONFIG_USB_SERIAL_WWAN=y
CONFIG_USB_SERIAL_OPTION=y
CONFIG_USBNET=y

Build-tools and Linux header files for your kernel version are also required, these can be installed e.g. through your OS distributions package manager, on Debian/Ubuntu systems:
apt-get install build-essential make gcc
apt-get install linux-headers-`uname -r`

The in-kernel qmi_wwan driver should be blacklisted and prevented from loading as it will block the Simcom wwan driver, this is how it can be done on in Ubuntu systems:
grep -q -F 'blacklist qmi_wwan' /etc/modprobe.d/blacklist-modem.conf || echo 'blacklist qmi_wwan' >> /etc/modprobe.d/blacklist-modem.conf

Build and install the driver:
Navigate to your selected working directory, e.g.:
cd /usr/src/sim7600/

Build and install the drivers:
make install

Some warnings might appear, but verify that no errors are reported. 
Restarting the host system should now result in the correct network drivers being loaded for the cellular module once module is detected in the system.

It can be verified with:
lsusb
Bus 001 Device 005: ID 1e0e:9001 Qualcomm / Option

lsusb -t
/:  Bus 01.Port 1: Dev 1, Class=root_hub, Driver=xhci_hcd/8p, 480M
    |__ Port 4: Dev 5, If 0, Class=Vendor Specific Class, Driver=option, 480M
    |__ Port 4: Dev 5, If 1, Class=Vendor Specific Class, Driver=option, 480M
    |__ Port 4: Dev 5, If 2, Class=Vendor Specific Class, Driver=option, 480M
    |__ Port 4: Dev 5, If 3, Class=Vendor Specific Class, Driver=option, 480M
    |__ Port 4: Dev 5, If 4, Class=Vendor Specific Class, Driver=option, 480M
    |__ Port 4: Dev 5, If 5, Class=Vendor Specific Class, Driver=simcom_wwan, 480M

dmesg | grep 'simcom_wwan'
simcom_wwan 1-4:1.5 wwan0: register 'simcom_wwan' at usb-0000:00:15.0-4, SIMCOM wwan/QMI device, 8a:d8:ff:c2:87:11

Additional make options and information:
If you've built the driver previously already, first clean out any old builds with:
make clean

If you only want to build the driver but not install it into /lib/modules/4.18.0-041800-generic/kernel/drivers/net/usb/, use make without install parameter:
make
