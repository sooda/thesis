#!/usr/bin/python2
# a small attempt to parallelize the computation for speed. does not seem to
# work, cv2+numpy do not release python's locks properly :(

import cv2
import numpy
import sys
import threading
import Queue

def getval(fname):
    a = cv2.imread(fname, cv2.CV_LOAD_IMAGE_GRAYSCALE)
    brightest = numpy.max(a)
    if brightest >= 200:
        _, a = cv2.threshold(a, 127, 255, cv2.THRESH_BINARY)
        cols = a.T
        start = [numpy.argmax(col) for col in cols]
        return numpy.min(start)
        #print numpy.min(start)
        #print numpy.mean(start)
        #print numpy.max(start)

def process(images, outputs):
    while True:
        try:
            img = images.get_nowait()
        except Queue.Empty:
            break
        outputs.put((img, getval(img)))

def main():
    images = Queue.Queue()
    for i in sys.argv[1:]:
        images.put(i)

    outputs = Queue.Queue()
    workers = []
    for i in range(4):
        t = threading.Thread(target=process, args=(images, outputs))
        t.start()
        workers.append(t)

    for i in sys.argv[1:]:
        val = outputs.get()
        if val[1] is not None:
            print val[0], val[1]

if __name__ == "__main__":
    main()
