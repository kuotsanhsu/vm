#include "io_uring.h"
#include "io_uring/syscall.h"
#include <err.h>
#include <error.h>
#include <linux/io_uring.h>
#include <stdatomic.h>

int main() {
  const unsigned entries = 1;
  struct io_uring ring;
  const int ret = io_uring_init(&ring, entries, 0, 0, 0);
  if (ret < 0) {
    error(1, -ret, "failed to set up io_uring");
  } else if (ret == 1) {
    err(1, "failed to map the submission queue entries");
  } else if (ret == 2) {
    err(1, "failed to map the submission queue");
  } else if (ret != 0) {
    errx(1, "unknown failure in io_uring_init");
  }

  const unsigned tail = *ring.sq.tail;
  const unsigned index = tail & *ring.sq.ring_mask;
  struct io_uring_sqe *const sqe = &ring.sqes[index];
  sqe->opcode = IORING_OP_WRITE;
  sqe->fd = 0;
  sqe->addr = (unsigned long long)"hello\n";
  sqe->user_data = sqe->len = 6;
  sqe->off = 0;
  ring.sq.array[index] = index;
  atomic_store_explicit(ring.sq.tail, tail + entries, memory_order_release);
  const int tasks =
      io_uring_enter(ring.fd, entries, entries, IORING_ENTER_GETEVENTS, NULL);
  if (tasks < 0) {
    error(1, -tasks, "io_uring_enter failed");
  } else if (tasks != entries) {
    errx(1, "submitted %d entries but io_uring_enter only completed %d tasks",
         entries, tasks);
  }

  const unsigned head =
      atomic_load_explicit(ring.cq.head, memory_order_acquire);
  if (head + entries != *ring.cq.tail) {
    errx(1, "completion queue does not contain exactly %u entries", entries);
  }
  const struct io_uring_cqe *const cqe =
      ring.cqes + (head & *ring.cq.ring_mask);
  if (cqe->res < 0) {
    error(1, -cqe->res, "failed to write");
  } else if (cqe->res != cqe->user_data) {
    errx(1, "only %d bytes out of %llu were written", cqe->res, cqe->user_data);
  }
  atomic_store_explicit(ring.cq.head, head + entries, memory_order_release);
}
