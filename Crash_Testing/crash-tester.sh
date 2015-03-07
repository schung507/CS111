#!/bin/sh

./set_crash 3 | \
cd test | \
echo 'hello' > world.txt | \
mkdir CS111-design-problem | \
echo 'yellow' > color.txt | \
#Things should start crashing now
ln world.txt jelly.txt | \
cp world.txt color.txt | \
rmdir CS111-design-problem | \
./set_crash -1 | \
#Now if you try to access things...disaster!!
cat jelly.txt | \
cd CS111-design-problem | \
cd .. | \
cat color.txt
