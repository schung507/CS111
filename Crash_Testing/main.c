#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include "ospfs.h"

main(){
  int fd;
  int fd2;
  fd = open("./test/world.txt", O_RDWR);
  fd2 = open("./test/thelink", O_RDWR | O_CREAT);

  link("./test/world.txt", "./test/thehardlink");
  ioctl(fd, IOCTL_CRASH, 5);
  write(fd, "yas yas yas", 12);
  close(fd);
}
