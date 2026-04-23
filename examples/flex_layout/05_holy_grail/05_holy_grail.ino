// Example 5: Full-screen "Holy Grail" layout
//
// A classic web layout adapted to an embedded screen:
//
//   ┌──────────────────────────────────┐
//   │           header (fixed)         │
//   ├────────┬─────────────┬───────────┤
//   │  nav   │   content   │  sidebar  │
//   │(fixed) │  (flex: 1)  │ (fixed)   │
//   ├────────┴─────────────┴───────────┤
//   │           footer (fixed)         │
//   └──────────────────────────────────┘
//
// The outer container is a column; the middle band is a row.
// The centre column inside the row consumes all leftover width via flex-grow.
// The outer column's middle row consumes all leftover height via flex-grow.
//
// Demonstrates:
//   Bidirectional nesting (column → row)
//   flex-grow = 1 for space-filling in both axes
//   AlignItems::kStretch so the side panels match the content height

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/widgets/blank.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_windows;

// ─── Placeholder panels ──────────────────────────────────────────────────────

// A solid-colour panel with a centred label, used as a stand-in for real
// content.  Stretches to fill whatever space it is given.
class NamedPanel : public FlexLayout {
 public:
  NamedPanel(const Environment& env, const char* name)
      : FlexLayout(env, FlexDirection::kRow),
        label_(env, name, font_body1()) {
    setJustifyContent(JustifyContent::kCenter);
    setAlignItems(AlignItems::kCenter);
    add(WidgetRef(label_));
  }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::MatchParentHeight());
  }

 private:
  TextLabel label_;
};

// ─── Holy Grail ──────────────────────────────────────────────────────────────

class HolyGrail : public FlexLayout {
 public:
  HolyGrail(const Environment& env)
      : FlexLayout(env, FlexDirection::kColumn),
        header_(env, "Header"),
        middle_(env, FlexDirection::kRow),
        nav_(env,     "Nav"),
        content_(env, "Content"),
        sidebar_(env, "Sidebar"),
        footer_(env, "Footer") {
    // Outer column: fill the whole screen.
    setAlignItems(AlignItems::kStretch);

    // Header and footer: fixed intrinsic height, full width.
    add(WidgetRef(header_), {.flex_grow = 0, .flex_shrink = 0});

    // Middle band: grows to absorb all remaining vertical space.
    middle_.setAlignItems(AlignItems::kStretch);

    // Nav: fixed 60 px wide, full height of the band.
    nav_.setMinimumDimensions(Dimensions(Scaled(60), 0));
    middle_.add(WidgetRef(nav_),
                {.flex_grow = 0, .flex_shrink = 0});

    // Content: takes up all leftover width.
    middle_.add(WidgetRef(content_), {.flex_grow = 1, .flex_shrink = 1});

    // Sidebar: fixed 80 px wide.
    sidebar_.setMinimumDimensions(Dimensions(Scaled(80), 0));
    middle_.add(WidgetRef(sidebar_),
                {.flex_grow = 0, .flex_shrink = 0});

    add(WidgetRef(middle_), {.flex_grow = 1, .flex_shrink = 1});

    add(WidgetRef(footer_), {.flex_grow = 0, .flex_shrink = 0});
  }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::MatchParentHeight());
  }

 private:
  NamedPanel header_;
  FlexLayout middle_;
  NamedPanel nav_;
  NamedPanel content_;
  NamedPanel sidebar_;
  NamedPanel footer_;
};
