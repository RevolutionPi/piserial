#!/bin/bash
#
# SPDX-License-Identifier: GPL-2.0
#
# Copyright: 2021 Kunbus GmbH
#

if [ "$USER" != pi ] ; then
	return
fi

# Environment variable for whiptail to set the color palette as the black color
# for the background and grey color for the boxes.
export NEWT_COLORS='root=,black entry=white,black'

while [ ! -r /home/pi/.revpi-factory-reset ] ; do
	clear
	msg="Please select the Product Type:"
	ovl=$(whiptail --notags --title "PRODUCT TYPE" --menu "$msg" 0 0 0 \
		compact "Revolution Pi Compact" \
		connect "Revolution Pi Connect" \
		connect-se "Revolution Pi Connect SE" \
		core "Revolution Pi Core" \
		core-se "Revolution Pi Core SE" \
		flat "Revolution Pi Flat" \
		3>&1 1>&2 2>&3)
	if [ "$?" == "1" ]; then
		return
	fi

	msg="Please enter the Serial Number on the front plate of your RevPi:"
	ser=$(whiptail --title "SERIAL NUMBER" --inputbox "$msg" 0 0 3>&1 1>&2 2>&3)
	if [ "$?" == "1" ]; then
		return
	fi

	msg="Please enter the MAC Address on the front plate of your RevPi:"
	mac=$(whiptail --title "MAC ADDRESS" --inputbox "$msg" 0 0 "C83E-A7" 3>&1 1>&2 2>&3)
	if [ "$?" == "1" ]; then
		return
	fi

	# this creates /home/pi/.revpi-factory-reset on success:
	/usr/bin/sudo /usr/sbin/revpi-factory-reset "$ovl" "$ser" "$mac" 2>/dev/null
	if [ "$?" = 1 ]; then
		$(whiptail --nocancel --title "ERROR" --msgbox "Invalid serial number or mac address" 0 0 3>&1 1>&2 2>&3)
	fi
done
