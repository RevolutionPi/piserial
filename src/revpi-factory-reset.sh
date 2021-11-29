#!/bin/bash
#
# SPDX-License-Identifier: GPL-2.0
#
# Copyright: 2021 Kunbus GmbH
#

if [ "$USER" != pi ] ; then
	return
fi

while [ ! -r /home/pi/.revpi-factory-reset ] ; do
	echo
	echo -n "Please enter the product (compact/connect/core/flat): "
	read ovl
	echo
	echo Please enter the serial number and MAC address on the front plate
	echo of your RevPi to reset the image to factory defaults:
	echo
	echo -ne "Serial:\t\t"
	read ser
	echo -ne "MAC Address:\t"
	read mac
	echo
	# this creates /home/pi/.revpi-factory-reset on success:
	/usr/bin/sudo /usr/sbin/revpi-factory-reset "$ovl" "$ser" "$mac"
done
