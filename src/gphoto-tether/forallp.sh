#!/bin/bash

for i in `./cams.sh`; do
	gphoto2 --port $i "$@" &
done

wait
