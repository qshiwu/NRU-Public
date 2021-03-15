#!/bin/sh
### BEGIN INIT INFO
# Provides:          Neousys
# Required-Start:    $remote_fs $syslog $time
# Required-Stop:     $remote_fs $syslog $time
# Default-Start:     2 3 4 5
# Default-Stop:
# Short-Description: Run NRU-startup jobs
# Description: Init ignition power control and CAN Bus
### END INIT INFO

### To Make Ignition Power Control Possible on Ubuntu 
gsettings set org.gnome.settings-daemon.plugins.power button-power shutdown
sudo echo 362 > /sys/class/gpio/export
sudo echo out > /sys/class/gpio/gpio362/direction
sudo echo 0 > /sys/class/gpio/gpio362/value


### CAN Bus related setup
###############################################
# CANBus Configure
sudo busybox devmem 0x0c303000 32 0x0000c400
sudo busybox devmem 0x0c303008 32 0x0000c458
sudo busybox devmem 0x0c303010 32 0x0000c400
sudo busybox devmem 0x0c303018 32 0x0000c458
sudo modprobe can
sudo modprobe can_raw
sudo modprobe mttcan
###############################################


# Put In User Manual
###############################################
# Set CANBus bit rate is 500kbps, enable flexible data rate (FD), 
# payload bitrate is 2Mbps and enable bus error reporting
sudo ip link set can0 type can bitrate 500000 dbitrate 2000000 berr-reporting on fd on
sudo ip link set can1 type can bitrate 500000 dbitrate 2000000 berr-reporting on fd on
###############################################

# Put In User Manual
###############################################
# Set CANBus Link Up
sudo ip link set up can0
sudo ip link set up can1
###############################################



##### For SIMCOM 7600 #########################

# Enable USB interface mPCIe module
sudo echo 371 > /sys/class/gpio/export
sudo echo out > /sys/class/gpio/gpio371/direction
sudo echo 1 > /sys/class/gpio/gpio371/value




