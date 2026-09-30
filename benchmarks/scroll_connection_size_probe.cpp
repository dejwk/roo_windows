// Compile-only symbols for comparing target ABI before and after connections.
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/app_bar/app_bar.h"
extern "C" {
unsigned char scroll_size_widget[sizeof(roo_windows::Widget)];
unsigned char scroll_size_panel[sizeof(roo_windows::SimpleScrollablePanel)];
unsigned char scroll_size_bar[sizeof(roo_windows::material3::AppBar)];
unsigned char scroll_size_search[sizeof(roo_windows::material3::SearchAppBar)];
unsigned char scroll_size_context[sizeof(roo_windows::ApplicationContext)];
}
