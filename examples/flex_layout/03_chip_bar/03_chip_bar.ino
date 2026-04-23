// Example 3: Wrapping chip bar
//
// A bar of selectable filter chips. Chips are allowed to wrap onto additional
// lines when they no longer fit horizontally. Chips have no grow/shrink — they
// keep their intrinsic (wrap-content) size. The container uses
// JustifyContent::kFlexStart within each line and
// AlignContent::kFlexStart to pack lines toward the top.
//
// Demonstrates:
//   FlexWrap::kWrap
//   JustifyContent::kFlexStart
//   AlignContent::kFlexStart
//   column-gap / row-gap

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/widgets/button.h"

using namespace roo_windows;

// ─── ChipBar ─────────────────────────────────────────────────────────────────

class ChipBar : public FlexLayout {
 public:
  ChipBar(const Environment& env)
      : FlexLayout(env, FlexDirection::kRow),
        chips_{
            SimpleButton(env, "All", Button::OUTLINED),
            SimpleButton(env, "Nearby", Button::OUTLINED),
            SimpleButton(env, "Open now", Button::OUTLINED),
            SimpleButton(env, "Top rated", Button::OUTLINED),
            SimpleButton(env, "Price", Button::OUTLINED),
            SimpleButton(env, "Food", Button::OUTLINED),
            SimpleButton(env, "Hotels", Button::OUTLINED),
            SimpleButton(env, "Activities", Button::OUTLINED),
        } {
    setFlexWrap(FlexWrap::kWrap);
    setJustifyContent(JustifyContent::kFlexStart);
    setAlignContent(AlignContent::kFlexStart);
    setAlignItems(AlignItems::kCenter);
    setPadding(Padding(PaddingSize::SMALL));
    setColumnGap(Scaled(8));
    setRowGap(Scaled(8));

    for (auto& chip : chips_) {
      // Chips never grow or shrink — keep intrinsic size.
      add(WidgetRef(chip), {.flex_grow = 0, .flex_shrink = 0});
    }
  }

 private:
  // Fixed-size array so everything lives in the same allocation.
  SimpleButton chips_[8];
};
