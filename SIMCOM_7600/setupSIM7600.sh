#!/bin/bash

echo "* If your SIM card has password, please press 'q' to quit, and setup your PIN code in neousys_start_SIM7600.sh first. "

read -n 1 k <&1
if [[ $k = q ]] || [[ $k = Q ]]; then
    echo ""
    echo "SIM7600SA Installation Stop."
    exit
fi

echo "Continue SIM7600SA Installation ..."

# The following kernel option are expected to be y or m
cat /proc/config.gz | zgrep CONFIG_USB_SERIAL=
cat /proc/config.gz | zgrep CONFIG_USB_SERIAL_WWAN=
cat /proc/config.gz | zgrep CONFIG_USB_SERIAL_OPTION=

### One Time Setup
# Add Blacklist to unwanted modeom driver
sudo grep -q -F 'blacklist qmi_wwan' /etc/modprobe.d/blacklist-modem.conf
sudo echo 'blacklist qmi_wwan' >>/etc/modprobe.d/blacklist-modem.conf

sudo apt update
# Install minicom
sudo apt-get install -y minicom

# Install udhcpc
sudo apt-get install -y udhcpc


# Make Simcom driver
cd sim7600_driver/
sudo make install

# Backup current NRU startup script
cd ..
now=$(date +"%m%d%y_%H%M")
cp /etc/init.d/neousys_startup.sh startup_script_backup/neousys_startup_$now.sh

# Copy a modified startup script for SIM7600SA
cp neousys_start_SIM7600.sh /etc/init.d/neousys_startup.sh

# Backup current /etc/network/interfaces
now=$(date +"%m%d%y_%H%M")
cp /etc/network/interfaces interfaces_backup/interfaces_$now

# Copy a modified interface for SIM7600SA
cp interfaces_sim7600 /etc/network/interfaces

echo "Reboot to test 4G LTE"
