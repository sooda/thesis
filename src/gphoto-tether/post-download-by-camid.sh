#!/bin/bash -e

mkdir -p multihax
i=0
for port in `./cams.sh`; do
	gphoto="gphoto2 --quiet --port $port"
	export CAMERAID=`$gphoto --get-config artist|grep ^Current|sed 's/^Current: //'`
	mkdir -p multihax/$CAMERAID
	cd multihax/$CAMERAID
	$gphoto -P &
	cd ../..
done

wait
