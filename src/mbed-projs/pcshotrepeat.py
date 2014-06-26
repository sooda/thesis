#!/usr/bin/python2
from pcshot import CameraControl
import sys
import time

def main():
    try:
        count = int(sys.argv[1])
    except IndexError:
        count = 1
    try:
        delay = float(sys.argv[2])
    except IndexError:
        delay = 0

    state = [x == "1" for x in sys.argv[3:]]
    if len(state) > 9:
        print "need only 9 cams"
        return
    elif len(state) == 0:
        state = [True] * 9
    elif len(state) < 9:
        state = [False] * (9 - len(state)) + state

    ctl = CameraControl("/dev/ttynucleo0")

    ctl.focus(state)
    time.sleep(3.1)
    for i in range(count):
        ctl.shoot(state)
        time.sleep(0.1)
        ctl.shootstop()
        time.sleep(delay)
    ctl.reset()

if __name__ == "__main__":
    main()

