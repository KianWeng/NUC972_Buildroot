#!/bin/sh

TAG="BURN_MACADDR"
BOOTARGS="root=/dev/ram0 console=ttyS0,115200n8 rdinit=/sbin/init mem=64M watchdog=on ethaddr0="
usage()
{
	printf "a command line interface to U-Boot environment\n \
		-h --help   print the usage information\n \
		ethaddr     the mac addr that will burn to flash.\n"
}
regex="([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}"
logger -t $TAG -s "Starting burn mac addr..."
case "$1" in
	-h|--hlep)
		usage
		;;
	*)
		result=`echo $1 | egrep $regex | wc -l`
		if [ $result -eq 0 ]; then
			logger -t $TAG -s "Not a correct mac addr, please put a correct mac addr!!!"
		else
			logger -t $TAG -s "Get a correct mac addr, we will burn it to the spi flash."	
			fw_setenv bootargs ${BOOTARGS}$1
		fi
		;;
esac
exit $?
