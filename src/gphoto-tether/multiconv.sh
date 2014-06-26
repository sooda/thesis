for i in 00*; do convert $i -resize 25% thum/$i ;sleep 0.1; done &
for i in 01*; do convert $i -resize 25% thum/$i ;sleep 0.1; done &
for i in 02*; do convert $i -resize 25% thum/$i ;sleep 0.1; done &
for i in 03*; do convert $i -resize 25% thum/$i ;sleep 0.1; done &

wait
