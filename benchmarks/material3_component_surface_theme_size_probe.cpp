// Target-ABI probe only. Inspect these named symbols with nm; this
// translation unit intentionally has no executable behavior.
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/button/toggle_icon_button.h"
#include "roo_windows/material3/card/flex_card.h"
#include "roo_windows/material3/list/list.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/navigation_bar/navigation_bar.h"
#include "roo_windows/material3/navigation_rail/navigation_rail.h"
#include "roo_windows/material3/switch/switch.h"
#include "roo_windows/material3/tabs/tabs.h"
#include "roo_windows/material3/text_field/text_field.h"
#include "roo_windows/material3/theme.h"

#define ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(type, name) \
  [[gnu::used]] unsigned char roo_windows_sizeof_##name[sizeof(type)] = {}

ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::ComponentTheme,
                                         material3_component_theme);
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::Material3Theme,
                                         material3_theme);

// The slots above are shared by these existing widgets; this probe makes it
// possible to verify on the supported target that no widget grows with them.
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::FlexCard,
                                         material3_flex_card);
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::AppBar,
                                         material3_app_bar);
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::SearchBar,
                                         material3_search_bar);
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::NavigationBar,
                                         material3_navigation_bar);
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::NavigationRail,
                                         material3_navigation_rail);
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::Tabs,
                                         material3_tabs);
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::ListEntry,
                                         material3_list_entry);
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::MenuEntry,
                                         material3_menu_entry);
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::TextField,
                                         material3_text_field);
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::Button,
                                         material3_button);
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(
    roo_windows::material3::ToggleIconButton, material3_toggle_icon_button);
ROO_WINDOWS_COMPONENT_SURFACE_SIZE_PROBE(roo_windows::material3::Switch,
                                         material3_switch);
