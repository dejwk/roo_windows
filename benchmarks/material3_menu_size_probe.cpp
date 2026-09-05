// Target-ABI probe only. Inspect these named symbols with nm; this translation
// unit intentionally has no executable behavior.
//
// A standalone cross-compiler invocation can define the macro below to avoid
// pulling a platform logging backend into this header-only ABI measurement.
// No measured menu type contains this parsing-only stand-in.
#if defined(ROO_WINDOWS_STANDALONE_ABI_PROBE)
#include "benchmarks/standalone_abi_logging_stub.h"
#endif

#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/menu/menu_surface.h"
#include "roo_windows/widgets/text_label.h"

#define ROO_WINDOWS_MENU_SIZE_PROBE(type, name) \
  [[gnu::used]] unsigned char roo_windows_sizeof_##name[sizeof(type)] = {}

ROO_WINDOWS_MENU_SIZE_PROBE(roo_windows::material3::Menu, material3_menu);
ROO_WINDOWS_MENU_SIZE_PROBE(roo_windows::Container, container);
ROO_WINDOWS_MENU_SIZE_PROBE(roo_windows::material3::ListEntry, list_entry);
ROO_WINDOWS_MENU_SIZE_PROBE(roo_windows::material3::MenuEntry,
                            material3_menu_entry);
ROO_WINDOWS_MENU_SIZE_PROBE(roo_windows::material3::StandardMenuItem,
                            material3_standard_menu_item);
ROO_WINDOWS_MENU_SIZE_PROBE(
    roo_windows::material3::MenuRow<roo_windows::material3::StandardMenuItem>,
    material3_standard_menu_row);
ROO_WINDOWS_MENU_SIZE_PROBE(roo_windows::material3::MenuGroup,
                            material3_menu_group);
ROO_WINDOWS_MENU_SIZE_PROBE(roo_windows::material3::internal::MenuPanel,
                            material3_menu_panel);
ROO_WINDOWS_MENU_SIZE_PROBE(roo_windows::SimpleScrollablePanel,
                            simple_scrollable_panel);
ROO_WINDOWS_MENU_SIZE_PROBE(roo_windows::material3::internal::MenuOverlay,
                            material3_menu_overlay);
ROO_WINDOWS_MENU_SIZE_PROBE(roo_windows::material3::MenuPolicy,
                            material3_menu_policy);
ROO_WINDOWS_MENU_SIZE_PROBE(roo_windows::StringViewLabel, string_view_label);
ROO_WINDOWS_MENU_SIZE_PROBE(roo_windows::material3::Badge, material3_badge);
