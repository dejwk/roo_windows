// Standalone target-ABI probe. Inspect named symbols with nm -S.
#if defined(ROO_WINDOWS_STANDALONE_ABI_PROBE)
#include "benchmarks/standalone_abi_logging_stub.h"
#endif

#include "roo_windows/core/container.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/button/split_button.h"
#include "roo_windows/material3/menu/menu.h"

#define ROO_WINDOWS_SPLIT_SIZE_PROBE(type, name) \
  [[gnu::used]] unsigned char roo_windows_sizeof_##name[sizeof(type)] = {}

ROO_WINDOWS_SPLIT_SIZE_PROBE(roo_windows::Widget, widget);
ROO_WINDOWS_SPLIT_SIZE_PROBE(roo_windows::Container, container);
ROO_WINDOWS_SPLIT_SIZE_PROBE(roo_windows::material3::Button, button);
ROO_WINDOWS_SPLIT_SIZE_PROBE(roo_windows::material3::SplitButton, split_button);
ROO_WINDOWS_SPLIT_SIZE_PROBE(roo_windows::material3::Menu, menu);
