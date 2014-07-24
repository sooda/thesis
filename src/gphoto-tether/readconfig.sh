#!/bin/sh

# save configs of a specific camera to a file for later inspection
#  (currentconfig.txt)
# also, dump a configuration command for all other cams
#  (calibsettings.sh)
# also, prompt for setting them immediately to all cams

if [ "$1" = "" ]; then
	echo "usage: $0 <camera name>"
	echo "(use camnames.sh for port-based mapping or use stickers in the cams)"
	exit 1
fi

port=`./camnames.sh $1`

if [ "$port" = "" ]; then
	echo "camera $1 not found."
	exit 2
fi

# used settings are like /main/imagesettings/imageformat
confs=`grep ^/ interesting-settings.txt|sed 's#.*/##'|tr '\n' ' '`

# prefix each config path with --get-config
confsget="--get-config ${confs// / --get-config }"

gphoto2 --port $port $confsget | egrep '^(Current|Label)' > currentconfig.txt # sed 'N;s/\n/ /'
# force this too if it's stuck in internal ram
echo "Label: Capture Target" >> currentconfig.txt
echo "Current: Memory card" >> currentconfig.txt

# (parse again; intermediate file is used because reading the config can be useful for humans)

# construct a gphoto2 command
(echo "#!/bin/sh"
echo -n "./forallp.sh "
while read labelline; do
	read confline
	labelline=${labelline/Label: /}
	confline=${confline/Current: /}
	if [[ "$confline" = Label:* ]]; then
		echo Bad config, some expected setting could not be read from the camera. >&2
		echo Check interesting-settings.txt and/or run this script with -x. >&2
		exit 1 # exits the () loop
	fi
	echo "\\"
	echo -n "    --set-config '$labelline'='$confline' "
done < currentconfig.txt ; echo) > calibsettings.sh || exit 1

chmod +x calibsettings.sh
cat calibsettings.sh
echo "calibrate all others? [y/n]"
read yes
if [ "$yes" = "y" ]; then
	./calibsettings.sh
	echo Calibrated.
fi
