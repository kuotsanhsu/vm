#include <err.h>
#include <error.h>
#include <linux/io_uring.h>
#include <signal.h>
#include <stdatomic.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

// https://man7.org/linux/man-pages/man2/io_uring_setup.2.html
int io_uring_setup(unsigned entries, struct io_uring_params *p) {
  return syscall(__NR_io_uring_setup, entries, p);
}

// https://man7.org/linux/man-pages/man2/io_uring_enter.2.html
int io_uring_enter2(unsigned fd, unsigned to_submit, unsigned min_complete,
                    unsigned flags, sigset_t *sig, size_t sz) {
  return syscall(__NR_io_uring_enter, fd, to_submit, min_complete, flags, sig,
                 sz);
}

// https://man7.org/linux/man-pages/man2/io_uring_enter.2.html
int io_uring_enter(unsigned fd, unsigned to_submit, unsigned min_complete,
                   unsigned flags, sigset_t *sig) {
  return io_uring_enter2(fd, to_submit, min_complete, flags, sig, _NSIG / 8);
}

// https://man7.org/linux/man-pages/man2/io_uring_register.2.html
int io_uring_register(unsigned fd, unsigned opcode, const void *arg,
                      unsigned nr_args) {
  return syscall(__NR_io_uring_register, fd, opcode, arg, nr_args);
}

#define QUEUE_DEPTH 1
#define BLOCK_SZ 1024

int ring_fd;
unsigned *sring_tail, *sring_mask, *sring_array, *cring_head, *cring_tail,
    *cring_mask;
struct io_uring_sqe *sqes;
struct io_uring_cqe *cqes;
char buff[BLOCK_SZ];
off_t offset;

int app_setup_uring();

// Read from completion queue.
// In this function, we read completion events from the completion queue. We
// dequeue the CQE, update and head and return the result of the operation.
int read_from_cq();

// Submit a read or a write request to the submission queue.
int submit_to_sq(int fd, int opcode);

int main() {
  if (app_setup_uring()) {
    errx(1, "Unable to setup uring!\n");
  }
  // A while loop that reads from stdin and writes to stdout. Breaks on EOF.
  while (1) {
    // Initiate read from stdin and wait for it to complete
    submit_to_sq(STDIN_FILENO, IORING_OP_READ);
    // Read completion queue entry
    const int res = read_from_cq();
    if (res > 0) { // Read successful. Write to stdout.
      submit_to_sq(STDOUT_FILENO, IORING_OP_WRITE);
      read_from_cq();
    } else if (res == 0) { // reached EOF
      break;
    } else if (res < 0) { // Error reading file
      error(0, -res, "Error reading file");
      break;
    }
    offset += res;
  }
}

void *uring_mmap(size_t len, off_t offset) {
  return mmap(0, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
              ring_fd, offset);
}

int app_setup_uring() {
  struct io_uring_params p = {};
  ring_fd = io_uring_setup(QUEUE_DEPTH, &p);
  if (ring_fd < 0) {
    // On error, a negative error code is returned. The caller should not rely
    // on errno variable.
    error(0, -ring_fd, "io_uring_setup");
    return 1;
  }

  // io_uring communication happens via 2 shared kernel-user space ring buffers,
  // which can be jointly mapped with a single mmap() call in kernels >= 5.4.
  int sring_sz = p.sq_off.array + p.sq_entries * sizeof(unsigned);
  int cring_sz = p.cq_off.cqes + p.cq_entries * sizeof(struct io_uring_cqe);

  void *cq_ptr, *sq_ptr;
  // Rather than check for kernel version, the recommended way is to check the
  // features field of the io_uring_params structure, which is a bitmask. If
  // IORING_FEAT_SINGLE_MMAP is set, we can do away with the second mmap() call
  // to map in the completion ring separately.
  if (p.features & IORING_FEAT_SINGLE_MMAP) {
    // sring_sz = cring_sz = max(sring_sz, cring_sz);
    if (cring_sz > sring_sz) {
      sring_sz = cring_sz;
    }
    // Map in the submission and completion queue ring buffers.
    sq_ptr = uring_mmap(sring_sz, IORING_OFF_SQ_RING);
    if (sq_ptr == MAP_FAILED) {
      warn("mmap sq_ptr");
      return 1;
    }
    cq_ptr = sq_ptr;
  } else {
    // Map in the completion queue ring buffer in older kernels separately
    cq_ptr = uring_mmap(cring_sz, IORING_OFF_CQ_RING);
    if (cq_ptr == MAP_FAILED) {
      warn("mmap cq_ptr");
      return 1;
    }
    // Kernels < 5.4 only map in the submission queue, though.
    sq_ptr = uring_mmap(sring_sz, IORING_OFF_SQ_RING);
    if (sq_ptr == MAP_FAILED) {
      warn("mmap sq_ptr");
      return 1;
    }
  }

  // Save useful fields for later easy reference
  sring_tail = sq_ptr + p.sq_off.tail;
  sring_mask = sq_ptr + p.sq_off.ring_mask;
  sring_array = sq_ptr + p.sq_off.array;

  // Map in the submission queue entries array
  sqes =
      uring_mmap(p.sq_entries * sizeof(struct io_uring_sqe), IORING_OFF_SQES);
  if (sqes == MAP_FAILED) {
    warn("mmap sqes");
    return 1;
  }

  // Save useful fields for later easy reference
  cring_head = cq_ptr + p.cq_off.head;
  cring_tail = cq_ptr + p.cq_off.tail;
  cring_mask = cq_ptr + p.cq_off.ring_mask;
  cqes = cq_ptr + p.cq_off.cqes;

  return 0;
}

int read_from_cq() {
  const unsigned head = atomic_load_explicit(cring_head, memory_order_acquire);
  // Remember, this is a ring buffer. If head == tail, it means that the buffer
  // is empty.
  if (head == *cring_tail) {
    return -1;
  }
  // Get the entry
  const int res = cqes[head & *cring_mask].res;
  if (res < 0) {
    error(0, -res, "Error: read_from_cq");
  }
  atomic_store_explicit(cring_head, head + 1, memory_order_release);
  return res;
}

int submit_to_sq(int fd, int opcode) {
  // Add our submission queue entry to the tail of the SQE ring buffer
  const unsigned tail = *sring_tail;
  const unsigned index = tail & *sring_mask;
  struct io_uring_sqe *sqe = &sqes[index];
  // Fill in the parameters required for the read or write operation
  sqe->opcode = opcode;
  sqe->fd = fd;
  sqe->addr = (unsigned long)buff;
  if (opcode == IORING_OP_READ) {
    memset(buff, 0, sizeof(buff));
    sqe->len = BLOCK_SZ;
  } else {
    sqe->len = strlen(buff);
  }
  sqe->off = offset;

  sring_array[index] = index;
  atomic_store_explicit(sring_tail, tail + 1, memory_order_release);

  // Tell the kernel we have submitted events with the io_uring_enter() system
  // call. We also pass in the IOURING_ENTER_GETEVENTS flag which causes the
  // io_uring_enter() call to wait until min_complete (the 3rd param) events
  // complete.
  const int ret = io_uring_enter(ring_fd, 1, 1, IORING_ENTER_GETEVENTS, NULL);
  if (ret < 0) {
    // Errors that occur not on behalf of a submission queue entry are returned
    // via the system call directly. On such an error, a negative error code is
    // returned. The caller should not rely on errno variable.
    error(0, -ret, "io_uring_enter");
    return -1;
  }

  return ret;
}
