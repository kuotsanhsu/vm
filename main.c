#include <Hypervisor/Hypervisor.h>
#include <err.h>
#include <inttypes.h>
#include <mach/mach_error.h>
#include <stdio.h>
#include <stdlib.h>

void guard_hv_return(hv_return_t error_value);
[[nodiscard]] uint32_t get_max_vcpu_count();

int main() {
  // hv_vm_config_t config = hv_vm_config_create();
  // os_release(config);
  const uint32_t max_vcpu_count = get_max_vcpu_count();

  // `hv_vm_create` requires the `com.apple.security.hypervisor` entitlement
  // else it returns `fae94007`. Always pass `NULL` to `hv_vm_create`.
  guard_hv_return(hv_vm_create(NULL));
  guard_hv_return(hv_vm_destroy());
}

/*
Task {
  hv_vm_create -> hv_vm_map -> pthread_create ->
  Thread {
    hv_vcpu_create -> hv_vcpu_run <-> Handle VMEXIT -> hv_vcpu_destroy
  } -> pthread_join -> hv_vm_unmap -> hv_vm_destroy
}
*/

void guard_hv_return(hv_return_t error_value) {
  if (error_value == HV_SUCCESS) {
    return;
  }
  // errx(EXIT_FAILURE, "%s\n", mach_error_string(hv_return));
  mach_error("[ERROR]", error_value);
  exit(EXIT_FAILURE);
}

uint32_t get_max_vcpu_count() {
  uint32_t max_vcpu_count;
  guard_hv_return(hv_vm_get_max_vcpu_count(&max_vcpu_count));
  fprintf(stderr, "[INFO] max_vcpu_count=%" PRIu32 "\n", max_vcpu_count);
  return max_vcpu_count;
}
