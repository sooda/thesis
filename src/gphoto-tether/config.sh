#!/bin/bash

# note: some of these only affect jpegs.

# note 2: error 0x2019 = devicebusy,
# some of these cannot be set if focus is pressed down

# aeb: auto exposure bracketing
# drivemode: continuous is also fun
if [ "$1" = "workaround" ]; then
	./forallp.sh \
		--set-config capturetarget="Memory card"
else
	./forallp.sh \
		--set-config imageformat=RAW \
		--set-config imageformatsd=RAW \
		--set-config shutterspeed="1/100" \
		--set-config iso="100" \
		--set-config colorspace="sRGB" \
		--set-config picturestyle="Standard" \
		--set-config focusmode="One Shot" \
		--set-config aperture="11" \
		--set-config meteringmode="Evaluative" \
		--set-config drivemode="Single" \
		--set-config capturetarget="Memory card"

#		--set-config aeb="off" \
# aeb can be configured only when the flash is off
fi
