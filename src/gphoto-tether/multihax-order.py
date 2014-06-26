#!/usr/bin/python2
import sys
#import shutil
import os
import glob

camsdirs = sys.argv[1:]
i = 0
globs = []
for camdir in camsdirs:
    print camdir
    files = sorted(glob.glob(camdir + "/*.JPG"), reverse=True)
    globs.append(files)
try:
    while True:
        for g in globs:
            imagefile = g.pop()
            os.symlink("../" + imagefile, "multidest/%04d.jpg" % i)
            i += 1
except IndexError:
    pass # empty list

#for imagefile in files:
#    print imagefile
#    shutil.copy(imagefile, "multidest/%04d.jpg" % i)
#    i += 1
