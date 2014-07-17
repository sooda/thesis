#!/bin/sh

# with pop-up flashes, each camera lights the object from the front.  if some
# of them flash at exactly the same time, the lighting is pretty uneven, so it
# might be better to flash them separately and hope that the subject stays
# still

# for self-shots, give some time in the beginning
# sleep 20
# number of shots, seconds between cameras, seconds between shots
./pcshotcycle.py 5 0.1 3
