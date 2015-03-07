#!/bin/sh

./set_crash 1 | \
cd test | \
echo 'hello' > world.txt | \
#echo 'yellow' > color.txt | \
#Things should start crashing now
ln world.txt jelly.txt | \
cp world.txt color.txt | \
#kdir CS111-design-problem | \
./set_crash -1 | \
#Now if you try to access things...disaster!!
cat jelly.txt | \
#cd CS111-design-problem | \
#cd .. | \
cat color.txt
