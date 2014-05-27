#!/bin/sh

case "$ACTION" in
    start)
		echo "$CAMERAID: START"
	;;
    download)
		echo "$CAMERAID: download happened: $ARGUMENT"
		# (do something with it? gphoto can rename the files alright)
		ls -lh $ARGUMENT
	;;
    stop)
	echo "$CAMERAID: STOP"
	;;
esac

exit 0
