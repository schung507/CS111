#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "ospfs.h"

int main () {
  
  int fd = open("/test/crashing", O_CREAT | O_RDONLY);
  ioctl_func(fd, IOCTL_NWRITES_TO_CRASH, 1);
  close(fd);

}
