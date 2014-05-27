#!/bin/bash

# note: don't worry about "cannot open file *.cr2" or something
# means just that no pics of that name exist

mkdir -p processed

for i in *.cr2 *.CR2; do
	ufraw-batch \
		--wb=camera \
		--exposure=auto \
		--out-type=jpeg \
		--compression=100 \
		--out-path processed \
		--size 2000x2000 \
		$i &
done
