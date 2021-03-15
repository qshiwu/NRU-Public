#!/bin/sh


##### For SIMCOM 7600 #########################

# Insert Relevant Drivers
sudo modprobe option
sleep 1

sudo modprobe usb_wwan
sleep 1

sudo modprobe sim7500_sim7600_wwan
sleep 1

# Wake up wwan
ifconfig -a
sleep 1

sudo ifconfig wwan0 up
sleep 1

# SIM7600SA Dial (Guide 3.3)
sudo echo 'send AT$QCRMCALL=1,1
sleep 4
! pkill minicom
' > script.txt

sleep 1

sudo minicom -b 115200 -D /dev/ttyUSB2 -S script.txt

sleep 2

# SIM7600SA Get IP (Guide 3.4)
sudo udhcpc -i wwan0

sleep 2


# Done!



