#!/bin/bash -e

if [ "$1" = "" ]; then
	echo "giev pics dir name"
	exit 1
fi

#trap ctrlc INT

function ctrlc() {
	echo lol
}

SHOTNAME=$1
cwd=`pwd`

i=0
for port in `./cams.sh`; do
	gphoto="gphoto2 --quiet --port $port"
	export CAMERAID=`$gphoto --get-config artist|grep ^Current|sed 's/^Current: //'`
	echo $i:$port:$CAMERAID
	$gphoto --filename pics/by-cam/$SHOTNAME/$CAMERAID/snap-$CAMERAID-%02n.%C \
		--wait-event-and-download \
		--hook-script $cwd/camhook.sh \
		| egrep -v "^(UNKNOWN PTP|UNKNOWN Camera)" &
	i=$((i+1))
done

wait
