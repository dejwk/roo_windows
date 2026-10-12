// Standalone target-ABI probe; inspect named symbols with nm -S.
#if defined(ROO_WINDOWS_STANDALONE_ABI_PROBE)
#include "benchmarks/standalone_abi_logging_stub.h"
#endif
#include "roo_windows/core/cached_measure.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/button/button_group.h"
#include "roo_windows/material3/button/icon_button.h"
#include "roo_windows/material3/button/toggle_icon_button.h"
#define GROUP_SIZE(type, name) \
  [[gnu::used]] unsigned char roo_windows_sizeof_##name[sizeof(type)] = {}

GROUP_SIZE(roo_windows::Widget, widget);
GROUP_SIZE(roo_windows::SurfaceWidget, surface_widget);
GROUP_SIZE(roo_windows::Container, container);
GROUP_SIZE(roo_windows::CachedMeasure, cached_measure);
GROUP_SIZE(roo_windows::material3::Button, button);
GROUP_SIZE(roo_windows::material3::IconButton, icon_button);
GROUP_SIZE(roo_windows::material3::ToggleIconButton, toggle_icon_button);
GROUP_SIZE(roo_windows::material3::ButtonGroupButton, label_entry);
GROUP_SIZE(roo_windows::material3::ButtonGroupIconButton, icon_entry);
GROUP_SIZE(roo_windows::material3::ButtonGroup, standard_group);
GROUP_SIZE(roo_windows::material3::ConnectedButtonGroup, connected_group);
GROUP_SIZE(roo_windows::material3::ComponentTheme, component_theme);
GROUP_SIZE(roo_windows::material3::Material3Theme, material3_theme);
GROUP_SIZE(roo_windows::Theme, theme);

static_assert(sizeof(roo_windows::material3::Button) ==
              (sizeof(void*) == 4 ? 40 : 72));
static_assert(sizeof(roo_windows::material3::IconButton) ==
              (sizeof(void*) == 4 ? 32 : 56));
static_assert(sizeof(roo_windows::material3::ToggleIconButton) ==
              (sizeof(void*) == 4 ? 40 : 72));
static_assert(sizeof(roo_windows::material3::ButtonGroupButton) -
                  sizeof(roo_windows::SurfaceWidget) <=
              (sizeof(void*) == 4 ? 20 : 40));
static_assert(sizeof(roo_windows::material3::ButtonGroupIconButton) -
                  sizeof(roo_windows::SurfaceWidget) <=
              (sizeof(void*) == 4 ? 12 : 24));
static_assert(sizeof(roo_windows::material3::ButtonGroup) -
                  sizeof(roo_windows::Container) <=
              (sizeof(void*) == 4 ? 16 : 32));
static_assert(sizeof(roo_windows::material3::ConnectedButtonGroup) -
                  sizeof(roo_windows::Container) <=
              (sizeof(void*) == 4 ? 24 : 40));
static_assert(sizeof(roo_windows::material3::ComponentTheme) == 29);
static_assert(sizeof(roo_windows::CachedMeasure) == 16);
