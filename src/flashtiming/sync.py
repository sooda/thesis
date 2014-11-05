#!/usr/bin/python2

# given source data in frame pics and sync information, make ordered symlinks
# for easier handling

# first, run something like this:
# for each file: ffmpeg -y -i MVI_XXXX.MOV -qscale:v 1 out_%05d.jpg
# for each file: ffmpeg -y -i MVI_XXXX.MOV audio_XXXX.wav
# once: sox -M */audio*.wav auds.wav
# inspect audios and write delays in seconds to sync.txt

# after this:


import glob
import os.path
import re
import sys

# delays are advances from the last one instead of delays since the first?
inverted_delays = True
# fps maps the delay times to frame numbers
fps = 25
# one delay value in seconds per line, in increasing order
syncfile = "sync.txt"
# matching order as in syncfile
camnames = ["A", "B", "C", "D", "E", "F", "G", "H", "I"]

def files(camname):
    return sorted(glob.glob(camname + "/out_*.jpg"))

fileframere = re.compile("out_(\d+).jpg")

def fileframe(path):
    base = os.path.basename(path)
    match = fileframere.match(base)
    return int(match.group(1))

def mkdirf(path):
    try:
        os.mkdir(path)
    except OSError:
        # maybe exists already
        pass

def symlinkforce(src, dest):
    try:
        os.symlink(src, dest)
    except OSError:
        os.remove(dest)
        os.symlink(src, dest)

def writelinks(outdir, camframes):
    back = "../../../"
    for name, frames in zip(camnames, camframes):
        i = 0
        for frame in frames:
            symlinkforce(back + frame, "%s/by-cam/%s/out_%05d.jpg" % (outdir, name, i))
            mkdirf("%s/by-frame/%05d" % (outdir, i))
            symlinkforce(back + frame, "%s/by-frame/%05d/%s.jpg" % (outdir, i, name))
            i += 1

def main():
    offsets = map(float, open(syncfile).readlines())

    # delays must be increasing
    if inverted_delays:
        offsets = [offsets[0] - x for x in offsets]

    frame_offsets = [int(round(x * fps)) for x in offsets]
    # must start only when the slowest one has started
    first_frame = frame_offsets[-1]
    camframes = [fs[first_frame + offset:]
            for fs, offset
            in zip(map(files, camnames), frame_offsets)]

    outdir = sys.argv[1]
    mkdirf(outdir)
    mkdirf(outdir + "/by-cam")
    mkdirf(outdir + "/by-frame")

    for cam in camnames:
        mkdirf(outdir + "/by-cam/" + cam)

    writelinks(outdir, camframes)

if __name__ == "__main__":
    main()
