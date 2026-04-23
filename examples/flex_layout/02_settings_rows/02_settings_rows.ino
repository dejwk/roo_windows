// Example 2: Settings list row
//
// A column of identical setting rows. Each row is a horizontal flex container
// with:
//   • An icon (fixed, cross-axis centered)
//   • A vertical flex sub-container holding a primary label and a secondary
//     caption — growing to fill remaining width
//   • A Switch widget at the far right (fixed, cross-axis centered)
//
// The outer column has no grow/shrink; its height wraps content.
//
// Demonstrates:
//   FlexDirection::kColumn   — outer list
//   FlexDirection::kRow      — each row
//   flex-grow on the text column
//   Nested FlexLayout containers
//   AlignItems::kCenter and AlignItems::kFlexStart mixed in a hierarchy

#include "roo_icons/filled/action/24/bluetooth.h"
#include "roo_icons/filled/action/24/wifi.h"
#include "roo_icons/filled/device/24/brightness_medium.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/switch.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_windows;

// ─── A single labelled setting row ───────────────────────────────────────────

class SettingRow : public FlexLayout {
 public:
  SettingRow(const Environment& env, const roo_display::Pictogram& icon_def,
             const char* primary, const char* secondary,
             bool initial_on = false)
      : FlexLayout(env, FlexDirection::kRow),
        icon_(env, icon_def),
        labels_(env, FlexDirection::kColumn),
        primary_(env, primary, font_body1()),
        secondary_(env, secondary, font_caption()),
        toggle_(env, initial_on) {
    setAlignItems(AlignItems::kCenter);
    setPadding(Padding(PaddingSize::REGULAR, PaddingSize::SMALL));
    setGap(Scaled(12));

    // Icon — fixed, vertically centered (inherited from align-items).
    add(WidgetRef(icon_), {.flex_grow = 0, .flex_shrink = 0});

    // Label column — grows to absorb free space; aligns its own children
    // to the flex-start (top) cross edge.
    labels_.setAlignItems(AlignItems::kFlexStart);
    labels_.add(WidgetRef(primary_), {.flex_grow = 0});
    labels_.add(WidgetRef(secondary_), {.flex_grow = 0});
    add(WidgetRef(labels_), {.flex_grow = 1, .flex_shrink = 1});

    // Toggle — fixed, vertically centered.
    add(WidgetRef(toggle_), {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  Icon icon_;
  FlexLayout labels_;
  TextLabel primary_;
  TextLabel secondary_;
  Switch toggle_;
};

// ─── Settings screen ─────────────────────────────────────────────────────────

class SettingsScreen : public FlexLayout {
 public:
  SettingsScreen(const Environment& env)
      : FlexLayout(env, FlexDirection::kColumn),
        wifi_(env, ic_filled_action_24_wifi(), "Wi-Fi", "Connected to HomeNet",
              true),
        div1_(env),
        bt_(env, ic_filled_action_24_bluetooth(), "Bluetooth", "Off", false),
        div2_(env),
        brightness_(env, ic_filled_device_24_brightness_medium(), "Brightness",
                    "70 %", true) {
    add(WidgetRef(wifi_));
    add(WidgetRef(div1_));
    add(WidgetRef(bt_));
    add(WidgetRef(div2_));
    add(WidgetRef(brightness_));
  }

 private:
  SettingRow wifi_;
  HorizontalDivider div1_;
  SettingRow bt_;
  HorizontalDivider div2_;
  SettingRow brightness_;
};
