#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include "ospfs.h"


/*Main must be called in order to crash the OSPFS File System.
  The user space call of ioctl will set nwrites_to_crash.*/
int main(int argc, char **argv){
  
int fd;
  
  fd = open("./test/crash.txt", O_RDONLY | O_CREAT);

  //set nwrites_to_crash to the first argument on the command line
  ioctl(fd, IOCTL_NWRITES_TO_CRASH, atoi(argv[1]));
 
  close(fd);
}
