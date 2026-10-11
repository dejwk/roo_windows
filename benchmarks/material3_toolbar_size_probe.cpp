// Compile with the supported target compiler and inspect symbol sizes with nm.
#include "benchmarks/standalone_abi_logging_stub.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows/material3/toolbar/toolbar.h"

#define ROO_WINDOWS_TOOLBAR_SIZE_PROBE(type, name) \
  [[gnu::used]] unsigned char roo_windows_sizeof_##name[sizeof(type)] = {}

ROO_WINDOWS_TOOLBAR_SIZE_PROBE(roo_windows::material3::Material3Container,
                               material3_container);
ROO_WINDOWS_TOOLBAR_SIZE_PROBE(roo_windows::material3::DockedToolbar,
                               material3_docked_toolbar);
ROO_WINDOWS_TOOLBAR_SIZE_PROBE(roo_windows::material3::FloatingToolbar,
                               material3_floating_toolbar);
ROO_WINDOWS_TOOLBAR_SIZE_PROBE(roo_windows::material3::ComponentTheme,
                               material3_component_theme);
ROO_WINDOWS_TOOLBAR_SIZE_PROBE(roo_windows::material3::Material3Theme,
                               material3_theme);
