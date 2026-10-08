// Standalone target-ABI probe. Inspect named symbols with nm -S.
#if defined(ROO_WINDOWS_STANDALONE_ABI_PROBE)
#include "benchmarks/standalone_abi_logging_stub.h"
#endif

#include "roo_windows/core/surface_widget.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/utilities/dropdown_button.h"

#define ROO_WINDOWS_DROPDOWN_SIZE_PROBE(type, name) \
  [[gnu::used]] unsigned char roo_windows_sizeof_##name[sizeof(type)] = {}

ROO_WINDOWS_DROPDOWN_SIZE_PROBE(roo_windows::SurfaceWidget, surface_widget);
ROO_WINDOWS_DROPDOWN_SIZE_PROBE(roo_windows::material3::Button,
                                material3_button);
ROO_WINDOWS_DROPDOWN_SIZE_PROBE(roo_windows::material3::DropdownButton,
                                material3_dropdown_button);
