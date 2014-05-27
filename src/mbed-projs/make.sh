#!/bin/bash -e
[ "$1" = "" ] && exit 1
proj=$1
./mybuild.py $proj
ls -l builds/$proj*
cp -v builds/$proj.bin /mnt
sync
