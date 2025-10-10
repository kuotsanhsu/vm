#pragma once
#include <linux/io_uring.h>
#include <sys/types.h>
#include <syscall.h>

struct io_uring {
  int fd;
  unsigned features;
  unsigned wq_fd;
  struct io_uring_sqe *sqes;
  struct io_uring_cqe *cqes;
  struct {
    unsigned *head;
    unsigned *tail;
    unsigned ring_mask;
    unsigned ring_entries;
    unsigned *flags;
    unsigned *dropped;
    unsigned *array;
  } sq;
  struct {
    unsigned *head;
    unsigned *tail;
    unsigned ring_mask;
    unsigned ring_entries;
    unsigned *flags;
    unsigned *overflow;
  } cq;
};

int io_uring_init(struct io_uring *ring, unsigned entries, unsigned flags,
                  unsigned sq_thread_cpu, unsigned sq_thread_idle);

// SYS_pwrite64
static inline void io_uring_pwrite(struct io_uring_sqe *sqe, int fd,
                                   const void *buf, size_t count,
                                   off_t offset) {
  sqe->opcode = IORING_OP_WRITE;
  sqe->fd = fd;
  sqe->addr = (__u64)buf;
  sqe->len = count;
  sqe->off = offset;
}

// SYS_write
static inline void io_uring_write(struct io_uring_sqe *sqe, int fd,
                                  const void *buf, size_t count) {
  io_uring_pwrite(sqe, fd, buf, count, -1);
}