"""Control all cameras separately; mbed needs pcshot.cpp"""
import serial

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
