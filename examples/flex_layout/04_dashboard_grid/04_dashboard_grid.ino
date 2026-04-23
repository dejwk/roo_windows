// Example 4: Dashboard card grid
//
// A 2×N grid of metric cards. Each card is a column-direction flex container
// with a large value label (grow) and a small caption below.  The outer
// container wraps cards row-by-row.  Cards have equal flex-basis (0 + grow=1)
// so each row divides the available width exactly in half.
//
// Demonstrates:
//   FlexWrap::kWrap with flex-basis 0 + flex-grow for equal-width columns
//   JustifyContent::kSpaceBetween   — two columns per row, no gaps at edges
//   AlignContent::kFlexStart        — rows packed to the top
//   AlignItems::kStretch            — cards stretch to match the tallest in row
//   Per-item flex_basis = FlexBasis::kZero

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_windows;

// ─── A single metric card ────────────────────────────────────────────────────

class MetricCard : public FlexLayout {
 public:
  MetricCard(const Environment& env, const char* value, const char* caption)
      : FlexLayout(env, FlexDirection::kColumn),
        value_(env, value, font_h5()),
        caption_(env, caption, font_caption()) {
    setAlignItems(AlignItems::kStretch);
    setJustifyContent(JustifyContent::kCenter);
    setPadding(Padding(PaddingSize::REGULAR));

    add(value_, {.flex_grow = 0});
    add(caption_, {.flex_grow = 0});
  }

 private:
  TextLabel value_;
  TextLabel caption_;
};

// ─── Dashboard grid ──────────────────────────────────────────────────────────

class DashboardGrid : public FlexLayout {
 public:
  DashboardGrid(const Environment& env)
      : FlexLayout(env, FlexDirection::kRow),
        temp_(env, "23 °C", "Temperature"),
        humidity_(env, "61 %", "Humidity"),
        pressure_(env, "1013 hPa", "Pressure"),
        co2_(env, "412 ppm", "CO₂"),
        wind_(env, "14 km/h", "Wind"),
        uv_(env, "3", "UV index") {
    setFlexWrap(FlexWrap::kWrap);
    setJustifyContent(JustifyContent::kSpaceBetween);
    setAlignContent(AlignContent::kFlexStart);
    setAlignItems(AlignItems::kStretch);
    setRowGap(Scaled(8));

    // flex-basis 0 + flex-grow 1: every card in the same row gets exactly half
    // the container width (two cards per row).
    FlexLayout::Params card_params{
        .flex_grow = 1,
        .flex_shrink = 1,
        .flex_basis = FlexBasis::kZero,
    };

    add(temp_, card_params);
    add(humidity_, card_params);
    add(pressure_, card_params);
    add(co2_, card_params);
    add(wind_, card_params);
    add(uv_, card_params);
  }

 private:
  MetricCard temp_;
  MetricCard humidity_;
  MetricCard pressure_;
  MetricCard co2_;
  MetricCard wind_;
  MetricCard uv_;
};
