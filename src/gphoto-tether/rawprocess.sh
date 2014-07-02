#!/bin/bash

mkdir -p processed

ls *.cr2 *.CR2 | xargs -n1 -P4 ufraw-batch \
	--wb=camera \
	--exposure=auto \
	--out-type=jpeg \
	--compression=100 \
	--out-path processed \
	--size 2000x2000 \
	--overwrite
