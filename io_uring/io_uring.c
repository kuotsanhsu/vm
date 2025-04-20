// https://git.kernel.dk/cgit/liburing/tree/src/setup.c

#include "io_uring.h"
#include "io_uring/syscall.h"
#include <sys/mman.h>

static inline size_t max(size_t a, size_t b) { return a >= b ? a : b; }

static inline void *io_uring_mmap(int ring_fd, off_t offset, size_t len) {
  return mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
              ring_fd, offset);
}

int io_uring_init(struct io_uring *ring, unsigned entries, unsigned flags,
                  unsigned sq_thread_cpu, unsigned sq_thread_idle) {
  struct io_uring_params p = {
      .flags = flags,
      .sq_thread_cpu = sq_thread_cpu,
      .sq_thread_idle = sq_thread_idle,
  };
  ring->fd = io_uring_setup(1, &p);
  if (ring->fd < 0) {
    return ring->fd; // error(1, -ring->fd, "failed to set up io_uring");
  }
  ring->features = p.features;
  ring->wq_fd = p.wq_fd;

  const size_t sqes_len = p.sq_entries * sizeof(struct io_uring_sqe);
  ring->sqes = io_uring_mmap(ring->fd, IORING_OFF_SQES, sqes_len);
  if (ring->sqes == MAP_FAILED) {
    return 1; // err(1, "failed to map the submission queue entries");
  }

  const size_t sq_len = p.sq_off.array + p.sq_entries * sizeof(unsigned);
  const size_t cq_len =
      p.cq_off.cqes + p.cq_entries * sizeof(struct io_uring_cqe);
  void *const q =
      io_uring_mmap(ring->fd, IORING_OFF_SQ_RING, max(sq_len, cq_len));
  if (q == MAP_FAILED) {
    return 2; // err(1, "failed to map the submission queue");
  }

  ring->sq.head = q + p.sq_off.head;
  ring->sq.tail = q + p.sq_off.tail;
  ring->sq.ring_mask = q + p.sq_off.ring_mask;
  ring->sq.ring_entries = q + p.sq_off.ring_entries;
  ring->sq.flags = q + p.sq_off.flags;
  ring->sq.dropped = q + p.sq_off.dropped;
  ring->sq.array = q + p.sq_off.array;

  ring->cq.head = q + p.cq_off.head;
  ring->cq.tail = q + p.cq_off.tail;
  ring->cq.ring_mask = q + p.cq_off.ring_mask;
  ring->cq.ring_entries = q + p.cq_off.ring_entries;
  ring->cq.flags = q + p.cq_off.flags;
  ring->cq.overflow = q + p.cq_off.overflow;
  ring->cqes = q + p.cq_off.cqes;

  return 0;
}
