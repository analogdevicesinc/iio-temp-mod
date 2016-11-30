#!/bin/sh
# Script to build iio-temp-mod on a Raspberry PI
# Download NOOBS_v2_0_0.zip and install Raspbian GNU/Linux 8 (jessie)
# Open Terminal CTRL-ALT-T
# wget https://github.com/analogdevicesinc/iio-temp-mod/blob/master/rpi_setup.sh
# chomd +x rpi_setup.sh
# ./rpi_setup.sh

cd /home/pi

cat  /etc/os-release

sudo apt-get -y install cmake libxml2-dev bison flex libjson0-dev
sudo apt-get -y remove libserialport0 libserialport-dev

uname -m | grep arm
if [ $? -eq 0 ]
then
	wget http://ftp.de.debian.org/debian/pool/main/libs/libserialport/libserialport0_0.1.1-1_armhf.deb
	wget http://ftp.de.debian.org/debian/pool/main/libs/libserialport/libserialport-dev_0.1.1-1_armhf.deb
	sudo dpkg -i libserialport0_0.1.1-1_armhf.deb
	sudo dpkg -i libserialport-dev_0.1.1-1_armhf.deb
fi

(
git clone https://github.com/analogdevicesinc/libiio.git
cd libiio
cmake .
make
sudo make install
)

(
git clone https://github.com/analogdevicesinc/iio-temp-mod.git
cd iio-temp-mod
make
sudo make install
)

iio_temp_mod -h

echo Removing serial console from kernel command line
echo Press any Key to continue ........
read key
sudo cp /boot/cmdline.txt /boot/cmdline.txt.orig
sudo sed -i 's/console=serial0,115200//g'  /boot/cmdline.txt
echo Please reboot the system
