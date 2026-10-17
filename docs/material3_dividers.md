# Material 3 dividers

Use `material3::HorizontalDivider` between stacked sections and
`material3::VerticalDivider` between adjacent panes. Include
`roo_windows/material3/divider/divider.h`.

Each divider paints a uniform stripe in the live Material 3
`color.outlineVariant`. Its thickness is `max(1, Scaled(1))` physical pixels:
one pixel at 75%, 100%, and 150% zoom, and two pixels at 200%. Density does not
change it. The horizontal divider prefers the parent's width and exact stroke
height; the vertical divider prefers exact stroke width and the parent's height.

Dividers derive directly from `Widget`, paint over the inherited surface, and
have no padding or margins. Use parent layout padding for inset separators and
layout gaps for space alongside the stripe. The
[status-panel example](../examples/material3/divider/status_panel/status_panel.ino)
shows both orientations with generic `FlexLayout` containers:

```cpp
auto section = std::make_unique<roo_windows::FlexLayout>(
    context, roo_windows::FlexDirection::kColumn);
section->setPadding(roo_windows::Padding(roo_windows::Scaled(16)));
section->add(std::make_unique<roo_windows::material3::HorizontalDivider>(context));
```

## Overrides and updates

Subclass the virtual hooks to customize color or thickness. Thickness overrides
must return a positive number of physical pixels:

```cpp
class StrongDivider : public roo_windows::material3::HorizontalDivider {
 public:
  using HorizontalDivider::HorizontalDivider;

  roo_display::Color dividerColor() const override {
    return theme().material3Theme().color.outline;
  }

  int16_t dividerThickness() const override {
    return 2;
  }
};
```

After changing a color override, call `invalidateInterior()`. After changing a
thickness override, call both `requestLayout()` and `invalidateInterior()`.
For a shared palette change, call `app.invalidateTheme()` on each affected
application. The default divider has no retained appearance state; palette
queries are live. Painting through `PaintContext` composes alpha ink and inherited
effects with the background in one output write per pixel.

## Dialog integration

Material 3 dialogs use these dividers at their scrolling body's edges. The top
stripe appears when content remains above the viewport; the bottom stripe
appears when content remains below it. Both use the resolved thickness and retain
the body's horizontal insets. Material 2 dividers keep their existing appearance.

## Validation and footprint

`material3_divider_test` covers both orientations, layout insets, color/thickness
overrides, palette replacement, dirty/full repaint equivalence, single-write
rendering, and basic/full-screen dialog scroll boundaries. It also runs with
`--copt=-DROO_WINDOWS_ZOOM=75` to check the minimum one-pixel stripe.
`material3_dialog_golden_test` covers the changed basic-dialog stripes and
full-screen top/middle/bottom scroll positions.

ESP32-C3 GCC target-ABI measurement with exceptions and RTTI disabled:

| Type | Bytes |
| --- | ---: |
| `Widget` | 24 |
| `HorizontalDivider` | 24 |
| `VerticalDivider` | 24 |

The probe in `benchmarks/material3_divider_size_probe.cpp` exports named size
symbols for inspection with the target toolchain's `nm -S`. Compile-time
assertions also enforce that neither divider adds fields beyond `Widget`.
