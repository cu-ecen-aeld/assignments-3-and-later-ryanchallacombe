#!/bin/sh
##########################################################################
# Add a startup script aesdsocket-start-stop in the “server” directory 
# of your assignment 3 repository which uses start-stop-daemon to start 
# your aesdsocket application in daemon mode with the -d option.
#
# On stop it should send SIGTERM to gracefully exit your daemon and perform 
# associated cleanup steps.
##########################################################################


case $1 in 
	start)
		echo "Starting aesdsocket daemon"
		
		# native system binary location
		# start-stop-daemon --start --name aesdsocket --startas /home/ryan/projects/assignment-1-ryanchallacombe/server/aesdsocket -- -d

		# embedded system binay location
		start-stop-daemon --start --name aesdsocket --startas /usr/bin/aesdsocket -- -d
		;;
	stop)
		echo "Stopping aesdsocket daemon"

		# SIGTERM is the dfault signal sent on stop
		start-stop-daemon --stop --name aesdsocket		
		;;
	*)
		echo "Usage: $0 {start | stop}"
	exit 1
esac

exit 0
