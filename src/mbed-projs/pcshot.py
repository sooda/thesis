import serial
import sys
import time


count = int(sys.argv[1])
delay = int(sys.argv[2])
try:
    bitmask = sys.argv[3]
except IndexError:
    bitmask = "1" * 9

s = serial.Serial("/dev/ttyACM0", 9600)

for i in range(count):
    s.write(bitmask + "\r\n")
    time.sleep(delay)
