#ifndef IO_URING_SYSCALL_H
#define IO_URING_SYSCALL_H

#include <linux/io_uring.h>
#include <signal.h>
#include <sys/syscall.h>
#include <unistd.h>

// https://man7.org/linux/man-pages/man2/io_uring_setup.2.html
static inline int io_uring_setup(unsigned entries, struct io_uring_params *p) {
  return syscall(__NR_io_uring_setup, entries, p);
}

// https://man7.org/linux/man-pages/man2/io_uring_enter.2.html
static inline int io_uring_enter2(unsigned fd, unsigned to_submit,
                                  unsigned min_complete, unsigned flags,
                                  sigset_t *sig, size_t sz) {
  return syscall(__NR_io_uring_enter, fd, to_submit, min_complete, flags, sig,
                 sz);
}

// https://man7.org/linux/man-pages/man2/io_uring_enter.2.html
static inline int io_uring_enter(unsigned fd, unsigned to_submit,
                                 unsigned min_complete, unsigned flags,
                                 sigset_t *sig) {
  return io_uring_enter2(fd, to_submit, min_complete, flags, sig, _NSIG / 8);
}

// https://man7.org/linux/man-pages/man2/io_uring_register.2.html
static inline int io_uring_register(unsigned fd, unsigned opcode,
                                    const void *arg, unsigned nr_args) {
  return syscall(__NR_io_uring_register, fd, opcode, arg, nr_args);
}

#endif // IO_URING_SYSCALL_H
