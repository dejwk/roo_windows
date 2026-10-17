#include "roo_windows/material3/divider/divider.h"

// Named target-ABI sizes; compile for the target and inspect with nm -S.
extern "C" {
char divider_size_widget[sizeof(roo_windows::Widget)];
char divider_size_horizontal[sizeof(roo_windows::material3::HorizontalDivider)];
char divider_size_vertical[sizeof(roo_windows::material3::VerticalDivider)];
}

static_assert(sizeof(roo_windows::Widget) ==
              sizeof(roo_windows::material3::HorizontalDivider));
static_assert(sizeof(roo_windows::Widget) ==
              sizeof(roo_windows::material3::VerticalDivider));
