#!/bin/bash -e

mkdir -p $1
i=0
for port in `./cams.sh`; do
	gphoto="gphoto2 --quiet --port $port"
	export CAMERAID=`$gphoto --get-config artist|grep ^Current|sed 's/^Current: //'`
	mkdir -p $1/$CAMERAID
	pushd $1/$CAMERAID > /dev/null
	$gphoto -P &
	popd > /dev/null
done

wait
