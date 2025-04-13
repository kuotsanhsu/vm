#include <Hypervisor/Hypervisor.h>
#include <err.h>
#include <inttypes.h>
#include <mach/mach_error.h>
#include <stdio.h>
#include <stdlib.h>

void guard_hv_return(hv_return_t error_value);
[[nodiscard]] uint32_t get_max_vcpu_count();
/** nestedVirtualizationSupported */
[[nodiscard]] bool get_el2_supported();
/** nestedVirtualizationEnabled */
[[nodiscard]] bool get_el2_enabled(hv_vm_config_t config);
/** nestedVirtualizationEnabled */
void set_el2_enabled(hv_vm_config_t config, bool el2_enabled);

int main() {
  hv_vm_config_t config = hv_vm_config_create();
  set_el2_enabled(config, get_el2_supported());

  // `hv_vm_create` requires the `com.apple.security.hypervisor` entitlement
  // else it returns `fae94007`.
  guard_hv_return(hv_vm_create(config));
  guard_hv_return(hv_vm_destroy());
  os_release(config);
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

bool get_el2_supported() {
  bool el2_supported;
  guard_hv_return(hv_vm_config_get_el2_supported(&el2_supported));
  fprintf(stderr, "[INFO] el2_supported=%d\n", el2_supported);
  return el2_supported;
}

bool get_el2_enabled(hv_vm_config_t config) {
  bool el2_enabled;
  guard_hv_return(hv_vm_config_get_el2_enabled(config, &el2_enabled));
  fprintf(stderr, "[INFO] el2_enabled=%d\n", el2_enabled);
  return el2_enabled;
}

void set_el2_enabled(hv_vm_config_t config, bool el2_enabled) {
  guard_hv_return(hv_vm_config_set_el2_enabled(config, el2_enabled));
  fprintf(stderr, "[INFO] el2_enabled=%d\n", el2_enabled);
}
