#!/bin/sh
##########################################################################
# Perform operations on startup to load the aesdchar driver and on shutdown to unload. 
##########################################################################

case $1 in 
	start)

		echo "Starting 'aesdchar' module"
		cd /usr/bin/
		./aesdchar_load 
		;;
	stop)
		echo "Stopping 'aesdchar' module"
		cd /usr/bin/
		./aesdchar_unload 

		;;
	*)
		echo "Usage: $0 {start | stop}"
	exit 1
esac

exit 0