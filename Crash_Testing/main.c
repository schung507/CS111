#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include "ospfs.h"

main(int argc, char **argv){
  int fd;
  // int fd2;
  printf("%d",  atoi(argv[1]));
  fd = open("./test/world.txt", O_RDWR | O_APPEND);
  //fd2 = open("./test/thelink", O_RDWR | O_CREAT);

  ioctl(fd, IOCTL_CRASH, atoi(argv[1]));
  write(fd, "yas yas yas", 12);
  write(fd, "this should crash", 17);
  write(fd, "this should crash", 17);
  close(fd);
}
