#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int fd;
  int n;
  char buf[1024] = "hello world";

  fd = open("/dev/my_cdev", O_RDWR);
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
  return 0;
}
