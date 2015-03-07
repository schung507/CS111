#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include "ospfs.h"

main(int argc, char **argv){
  int fd;
  // int fd2;
  printf("%d",  atoi(argv[1]));
  fd = open("./test/world.txt", O_RDWR);
  //fd2 = open("./test/thelink", O_RDWR | O_CREAT);

  ioctl(fd, IOCTL_NWRITES_TO_CRASH, atoi(argv[1]));
  // write(fd, "yas yas yas", 12);
  //link("./test/world.txt", "./test/thelink");
 
  close(fd);
}
