#include "./my_cdev.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int fd;
  int n;
  char buf[1024] = "hello world";

  fd = open("/dev/my_dev0", O_RDWR);
  if (fd < 0) {
    perror("fail to open");
    return -1;
  }
  printf("open successful,fd = %d\n", fd);
  n = write(fd, buf, strlen(buf));
  if (n < 0) {
    perror("fail to write");
    return -1;
  }
  printf("write %d bytes!\n", n);

  if (ioctl(fd, MY_CDEV_CLEAN) < 0) {
    perror("ioctl CLEAN");
  } else {
    printf("CLEAN ok\n");
  }

  int set_val = 10;
  if (ioctl(fd, MY_CDEV_SETVALUE, set_val) < 0) {
    perror("ioctl SETVALUE");
  } else {
    printf("SETVALUE = %d ok\n", set_val);
  }

  int get_val;
  if (ioctl(fd, MY_CDEV_GETVALUE, &get_val) < 0) {
    perror("ioctl GETVALUE");
  } else {
    printf("GETVALUE = %d\n", get_val);
  }

  return 0;
}
