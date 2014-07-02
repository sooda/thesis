Camera control scripts
======================

Some gphoto2 shell script fun. In alphabetical order:

camhook.sh:
	example for hooking captured files
camnames.sh:
	print a port-camname-mapping, or port of a single cam
cams.sh:
	list ports of all connected cameras
capture-by-camid.sh:
	capture tethered to <capname>/<camid>/<shotid>
capture-by-shotid.sh:
	capture tethered to <capname>/<shotid>/<camid>
config.sh:
	set common configuration parameters
	TODO: configuration tool for this?
	"./config.sh workaround": set only capture target to memory card,
	releases uilock if shutter is half-pressed to focus
configrepeatshot.sh:
	as above, for continuous cycle testing
forall.sh:
	run gphoto2 with given args for all connected cameras serially
forallp.sh:
	same as above, but in in parallel
interesting-settings.txt:
	settings for readconfig.sh, see below
multihax-order.py:
	interleave pictures taken from all cameras consecutively
multihax-run.sh:
	run the above in reverse cam order
	(if cams' shutter releases are connected so, needed a while ago)
post-download-by-camid.sh:
	download all previously shot files stored in the cameras to
	directories by camera name, in parallel; -DR is safe after this
rawprocess.sh:
	ufraw-batch converter to 2000x2000, run in a dir full of cr2's
readconfig.sh:
	read and copy settings from one camera to all others and dump
	the read data to a file for inspection, producing
	currentconfig.txt and calibsettings.sh
release-uilock.sh:
	run gphoto2 -L for all cams, this releases the locked buttons and
	blank screen that get stuck when fiddling with settings for some reason
shutters.sh:
	read shutter counters

delete all files recursively WITHOUT ASKING: ./forallp.sh -DR
  (appears to see only DCIM and MISC folders, ignores e.g. magic lantern)

list all files: ./forall.sh -L
