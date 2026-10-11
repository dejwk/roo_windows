// Compile-only ABI probe. Symbol sizes are widget object sizes in bytes.
#include "benchmarks/standalone_abi_logging_stub.h"
#include "roo_windows/material3/fab/fab.h"

#define ROO_FAB_SIZE_PROBE(type, name) \
  [[gnu::used]] unsigned char roo_windows_sizeof_##name[sizeof(type)] = {}

ROO_FAB_SIZE_PROBE(roo_windows::SurfaceWidget, surface_widget);
ROO_FAB_SIZE_PROBE(roo_windows::material3::FloatingActionButton, fab);
ROO_FAB_SIZE_PROBE(roo_windows::material3::ExtendedFloatingActionButton,
                   extended_fab);
