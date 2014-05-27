Camera control scripts
======================

Some gphoto2 shell script fun.

cams.sh: list ports of all connected cameras
forall.sh: run gphoto2 with given args for all connected cameras serially
forallp.sh: same as above, but in in parallel
shutters.sh: read shutter counters
config.sh: set common configuration parameters
config.sh workaround: set only capture target to memory card, quicker
runcapture.sh: set all cams to tether mode, save pictures to pics/<arg>/<cam>
rawprocess.sh: ufraw-batch converter to 2000x2000, run in a dir full of cr2's

delete all files WITHOUT ASKING: ./forallp.sh -D -R

list all files: ./forall.sh -L
