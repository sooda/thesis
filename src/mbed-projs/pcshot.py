"""Control all cameras separately; mbed needs pcshot.cpp"""
import serial
import argparse
import sys
import time

NCAMS = 9

class CameraControl:
    def __init__(self, port):
        self.ser = serial.Serial(port, 9600)

    def focus(self, cams):
        self._write("F", cams)

    def shoot(self, cams):
        self._write("S", cams)

    def shootstop(self):
        self._write("S", [False] * 9)

    def reset(self):
        self.ser.write("R\r\n")

    def _write(self, flag, cams):
        if len(cams) != NCAMS:
            raise IndexError("need %s cams" % NCAMS)
        bitmask = "".join("1" if x else "0" for x in cams)
        print flag + bitmask
        self.ser.write(flag + bitmask + "\r\n")
        self.ser.setTimeout(0.0)
        back = self.ser.read(1000) # flush needed?
        #print "<" + back
        # TODO b"lol"

def nonecams():
    return [False] * NCAMS

def allcams():
    return [True] * NCAMS

def onlycam(i):
    if i < 0 or i > NCAMS:
        raise ValueError("bad camera index")
    c = nonecams()
    c[i] = True
    return c

def parseargs():
    parser = argparse.ArgumentParser(description="control eos cameras remotely",
            formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-d", "--device",
            help="serial device to use", default="/dev/ttynucleo0")
    parser.add_argument("-r", "--repeat",
            help="repeat count (1=only once)", type=int, default=1)
    parser.add_argument("-w", "--wait",
            help="wait delay between repeats", type=float, default=1.0)
    parser.add_argument("-m", "--mask",
            help="bitmask of 0s and 1s, 1 to enable cam", default="111111111")
    parser.add_argument("-f", "--focuswait",
            help="delay between focus button and first shutter press", type=float, default=1.0)
    parser.add_argument("-i", "--interwait",
            help="inter-camera delay to shoot in succession", type=float, default=0.0)
    return parser.parse_args()

def normalrepeat(ctl, args, mask):
    for r in range(args.repeat):
        if r != 0:
            time.sleep(args.wait / 2)
        ctl.shoot(mask)
        time.sleep(args.wait / 2)
        ctl.shootstop()

def cyclerepeat(ctl, args, mask):
    # reversed because from lsb to msb
    indices = list(reversed([i for i, enabled in enumerate(mask) if enabled]))
    for r in range(args.repeat):
        if r != 0:
            time.sleep(args.wait)
        for i, camid in enumerate(indices):
            if i != 0:
                time.sleep(args.interwait)
            ctl.shoot(onlycam(camid))

def run(args):
    if len(args.mask) > NCAMS:
        print "too many cameras in mask"
        return 1

    mask = [x == "1" for x in args.mask]
    # control uses lsb mask as 0, but for the user, using first as 0 is more intuitive
    mask = list(reversed(mask))
    mask = [False] * (NCAMS - len(mask)) + mask

    ctl = CameraControl(args.device)

    try:
        ctl.focus(mask)
        time.sleep(args.focuswait)

        if args.interwait == 0.0:
            normalrepeat(ctl, args, mask)
        else:
            cyclerepeat(ctl, args, mask)
    finally:
        ctl.reset()

def main():
    sys.exit(run(parseargs()))

if __name__ == "__main__":
    main()
