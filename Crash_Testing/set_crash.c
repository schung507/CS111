#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include "ospfs.h"

main(int argc, char **argv){
  int fd;
  
  // printf("%d",  atoi(argv[1]));
  fd = open("./test/crash.txt", O_RDONLY | O_CREAT);

  //set nwrites_to_crash
  ioctl(fd, IOCTL_NWRITES_TO_CRASH, atoi(argv[1]));
  // write(fd, "yas yas yas", 12);
  //link("./test/world.txt", "./test/thelink");
 
  close(fd);
}
