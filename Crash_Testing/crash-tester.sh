#!/bin/sh

#set_crash sets n_writes_to_crash
./set_crash 4

cd test

#For each of the following commands, nwrites_to_crash decrements by 1
#But all commands are executed normally

echo 'hello' >> world.txt
 
echo 'yellow' > color.txt

ln world.txt copy_of_world.txt

ln -s hello.txt symlink_to_hello

#Anything following this line should silently FAIL i.e. none
#of these commands execute properly, causing an incorrect state

ln world.txt jelly.txt

rm copy_of_world.txt

echo 'red blue green' >> color.txt

unlink symlink_to_hello

cd ..

#Restore the OSPFS system so that we can use the file system normally

./set_crash -1

cd test 

#Now if you try to access things added or removed during the incorrect
#state...disaster!!

#jelly.txt does not exist
ln jelly.txt peanut.txt

#cd copy_of_world.txt still exists, even though it should have been removed
cat copy_of_world.txt

#Only contains 'yellow'
cat color.txt

