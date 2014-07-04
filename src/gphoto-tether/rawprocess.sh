#!/bin/bash

mkdir -p processed

ufraw-batch \
	--wb=camera \
	--exposure=auto \
	--out-type=jpeg \
	--compression=100 \
	--out-path processed \
	--size 2000x2000 \
	--overwrite \
	*.cr2 *.CR2
