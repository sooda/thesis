#!/bin/bash

# list all applicable names for --port argument

gphoto2 --auto-detect|grep -o 'usb:...,...'
