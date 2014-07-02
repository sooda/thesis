#!/bin/sh

# preparation: label the cameras, set same labels to the artist field.

# this gets a mapping between artist names and ports for gphoto's --port.

for port in `./cams.sh`; do
	name=`gphoto2 --quiet --port $port --get-config artist|grep ^Current|sed 's/^Current: //'`
	if [ "$1" = "" ]; then
		echo "camera at port $port = $name"
	else
		[ "$1" = "$name" ] && echo $port
	fi
done
