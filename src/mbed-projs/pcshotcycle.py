#!/usr/bin/python2
from pcshot import CameraControl, allcams, onlycam, NCAMS
import sys
import time

# for continuous mode shooting (i.e. shoot as long as shutter is down), only
# start shooting separately; next iterations modify nothing, they just wait
cams_continuous = False

def main():
    count = int(sys.argv[1])
    delay = float(sys.argv[2])
    delay2 = float(sys.argv[3])

    ctl = CameraControl("/dev/ttynucleo0")

    ctl.focus(allcams())
    time.sleep(3.0)
    state = [False] * 9
    for i in range(count):
        for ci in range(NCAMS):
            if cams_continuous:
                state[ci] = True
                ctl.shoot(state)
            else:
                ctl.shoot(onlycam(ci))
            time.sleep(delay)
        time.sleep(delay2 - delay)
    ctl.reset()

if __name__ == "__main__":
    main()


