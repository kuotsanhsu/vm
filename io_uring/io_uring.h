#ifndef IO_URING_H
#define IO_URING_H

#include <linux/io_uring.h>

struct io_uring {
  int fd;
  unsigned features;
  unsigned wq_fd;
  struct io_uring_sqe *sqes;
  struct io_uring_cqe *cqes;
  struct {
    unsigned *head;
    unsigned *tail;
    unsigned *ring_mask;
    unsigned *ring_entries;
    unsigned *flags;
    unsigned *dropped;
    unsigned *array;
  } sq;
  struct {
    unsigned *head;
    unsigned *tail;
    unsigned *ring_mask;
    unsigned *ring_entries;
    unsigned *flags;
    unsigned *overflow;
  } cq;
};

int io_uring_init(struct io_uring *ring, unsigned entries, unsigned flags,
                  unsigned sq_thread_cpu, unsigned sq_thread_idle);

#endif // IO_URING_H