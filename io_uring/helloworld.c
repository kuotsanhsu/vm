#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc == 1) {
    argv[0] = getenv("SHELL");
  } else {
    ++argv;
  }
  unshare(CLONE_NEWUSER | CLONE_NEWNS);
  int fd;
  fd = open("/proc/self/setgroups", O_WRONLY);
  write(fd, "deny", 4);
  close(fd);
  fd = open("/proc/self/uid_map", O_WRONLY);
  // write(fd, "0 99052 1", 9);
  // write(fd, "143098 143098 1", 15);
  close(fd);
  fd = open("/proc/self/gid_map", O_WRONLY);
  write(fd, "0 30 1", 6);
  close(fd);
  mount("none", "/", NULL, MS_REC | MS_PRIVATE, NULL);
  execvp(argv[0], argv);
}
