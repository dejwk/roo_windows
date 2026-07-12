// *************** EMULATOR SETUP BEGIN

#ifdef ROO_TESTING

#include "roo_testing/devices/display/ili9341/ili9341spi.h"
#include "roo_testing/devices/touch/xpt2046/xpt2046spi.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/transducers/ui/viewport/flex_viewport.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"

using roo_testing_transducers::FlexViewport;
using roo_testing_transducers::FltkViewport;

struct Emulator {
  FltkViewport viewport;
  FlexViewport flexViewport;

  FakeIli9341Spi display;
  FakeXpt2046Spi touch;

  Emulator()
      : viewport(),
        flexViewport(viewport, 1, FlexViewport::kRotationRight),
        display(flexViewport),
        touch(flexViewport, FakeXpt2046Spi::Calibration(269, 249, 3829, 3684,
                                                        true, false, false)) {
    FakeEsp32().attachSpiDevice(display, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(7, display.cs());
    FakeEsp32().gpio.attachOutput(2, display.dc());
    FakeEsp32().gpio.attachOutput(3, display.rst());
    FakeEsp32().attachSpiDevice(touch, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(1, touch.cs());
  }
} emulator;

#endif

// *************** DISPLAY SETUP BEGIN

#include <algorithm>

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/ui/text_label.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

// Select the driver to match your display device.
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"

// Set your configuration for the driver.
static constexpr int kCsPin = 7;
static constexpr int kDcPin = 2;
static constexpr int kRstPin = 3;
static constexpr int kBlPin = 20;

static constexpr int kSpiSckPin = 4;
static constexpr int kSpiMisoPin = 5;
static constexpr int kSpiMosiPin = 6;

static constexpr int kTouchCsPin = 1;

Ili9341spi<kCsPin, kDcPin, kRstPin> screen(Orientation().rotateLeft());
TouchXpt2046<kTouchCsPin> touch;

Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

void initDisplay() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.enableTurbo();
  display.init();
}

// *************** EXAMPLE STARTS HERE

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/switch/badged_switch.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/text_label.h"

namespace {

using roo_display::StringViewLabel;

Rect EmptyRect() { return Rect(0, 0, -1, -1); }

int16_t CornerInset(uint8_t radius) { return radius - (181 * radius) / 256; }

Rect InnerRoundedRect(const Rect& bounds, uint8_t radius) {
  if (bounds.empty()) return EmptyRect();
  int16_t inset = CornerInset(radius);
  return Rect(bounds.xMin() + inset, bounds.yMin() + inset,
              bounds.xMax() - inset, bounds.yMax() - inset);
}

void PaintRoundedSurface(PaintContext& ctx, const Rect& bounds, uint8_t radius,
                         roo_display::Color color) {
  Rect inner = InnerRoundedRect(bounds, radius);
  if (!inner.empty()) {
    PaintContext sub = ctx.clipped(inner);
    if (!sub.empty()) {
      sub.fillRect(inner, color);
    }
  }

  PaintDecoration decoration;
  decoration.bounds = bounds;
  decoration.background = color;
  decoration.corner_radii = {radius, radius, radius, radius};
  ctx.addDecoration(decoration);
}

Insets InsetsFromEnvelope(const Rect& logical_bounds, const Rect& envelope) {
  if (logical_bounds.empty() || envelope.empty()) {
    return Insets::Zero();
  }
  Rect combined = Rect::Extent(logical_bounds, envelope);
  return Insets(combined.xMin() - logical_bounds.xMin(),
                combined.yMin() - logical_bounds.yMin(),
                logical_bounds.xMax() - combined.xMax(),
                logical_bounds.yMax() - combined.yMax());
}

class SwitchBadgeRow : public FlexLayout {
 public:
  SwitchBadgeRow(ApplicationContext& context, const char* primary,
                 const char* secondary, bool on)
      : FlexLayout(context, FlexDirection::kRow),
        labels_(context, FlexDirection::kColumn),
        primary_(context, primary, font_body1()),
        secondary_(context, secondary, font_caption()),
        sw_(context, on) {
    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(12));
    setAlignItems(AlignItems::kCenter);

    labels_.setGap(Scaled(2));
    labels_.setAlignItems(AlignItems::kFlexStart);
    labels_.add(primary_, {.flex_grow = 0, .flex_shrink = 0});
    labels_.add(secondary_, {.flex_grow = 0, .flex_shrink = 0});

    add(labels_, {.flex_grow = 1, .flex_shrink = 1});
    add(sw_, {.flex_grow = 0, .flex_shrink = 0});
  }

  material3::BadgedSwitch& control() { return sw_; }

 private:
  FlexLayout labels_;
  roo_windows::TextLabel primary_;
  roo_windows::TextLabel secondary_;
  material3::BadgedSwitch sw_;
};

class BadgeCard : public Widget {
 public:
  BadgeCard(ApplicationContext& context, const char* glyph,
    bool unclipped = false)
  : Widget(context), glyph_(glyph) {
    if (unclipped) {
      setParentClipMode(ParentClipMode::kUnclipped);
    }
  }

  void setBadgeDot() {
    badge_.setDot();
    requestLayout();
    setDirty();
  }

  void setBadgeText(roo::string_view text) {
    badge_.setText(text);
    requestLayout();
    setDirty();
  }

  void setBadgeValue(unsigned int value) {
    badge_.setValue(value);
    requestLayout();
    setDirty();
  }

  void setBadgePlacement(material3::BadgePlacement placement) {
    placement_ = placement;
    requestLayout();
    setDirty();
  }

  Insets getInkInsets() const override {
    if (!badge_.visible()) return Insets::Zero();
    Rect conservative = material3::Badge::ConservativeBounds(
        anchorBounds(), placement_,
        badge_.mode() == material3::BadgeMode::kText);
    return InsetsFromEnvelope(bounds(), conservative);
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(Scaled(88), Scaled(72));
  }

  void paint(PaintContext& ctx) const override {
    // Settle the badge first; later card/icon paint is lower-z content.
    badge_.paint(ctx, theme());

    uint8_t outer_radius = static_cast<uint8_t>(Scaled(10));
    PaintRoundedSurface(ctx, bounds(), outer_radius,
                        theme().material3Theme().color.primaryContainer);

    Rect anchor = anchorBounds();
    uint8_t inner_radius = static_cast<uint8_t>(Scaled(8));
    PaintRoundedSurface(ctx, anchor, inner_radius,
                        theme().material3Theme().color.secondaryContainer);

    Rect anchor_inner = InnerRoundedRect(anchor, inner_radius);
    if (!anchor_inner.empty()) {
      PaintContext label_ctx = ctx.clipped(anchor_inner);
      if (!label_ctx.empty()) {
        label_ctx.setBgcolor(theme().material3Theme().color.secondaryContainer);
        label_ctx.drawTiled(StringViewLabel(glyph_, font_h5(),
                                            theme().material3Theme().color.onSecondaryContainer),
                            anchor,
                            roo_display::kCenter | roo_display::kMiddle);
      }
    }
  }

 protected:
  void onLayout(bool changed, const Rect& rect) override {
    (void)changed;
    (void)rect;
    badge_.layout(anchorBounds(), placement_);
  }

  Rect getDirectPaintExclusionBounds() const override {
    return InnerRoundedRect(bounds(), static_cast<uint8_t>(Scaled(10)));
  }

 private:
  Rect anchorBounds() const {
    Rect local = bounds();
    if (local.empty()) return Rect(0, 0, -1, -1);
    int16_t side = std::min<int16_t>(local.width() - Scaled(24),
                                     local.height() - Scaled(20));
    side = std::max<int16_t>(side, Scaled(28));
    int16_t left = local.xMin() + (local.width() - side) / 2;
    int16_t top = local.yMin() + (local.height() - side) / 2;
    return Rect(left, top, left + side - 1, top + side - 1);
  }

  material3::Badge badge_;
  material3::BadgePlacement placement_;
  const char* glyph_;
};

class BadgeCardColumn : public FlexLayout {
 public:
  BadgeCardColumn(ApplicationContext& context, const char* title,
                  bool unclipped = false)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, title, font_caption()),
        card_(context, "M3", unclipped) {
    setGap(Scaled(6));
    setAlignItems(AlignItems::kCenter);
    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(card_, {.flex_grow = 0, .flex_shrink = 0});
  }

  BadgeCard& card() { return card_; }

 private:
  roo_windows::TextLabel title_;
  BadgeCard card_;
};

class BadgeScreen : public ScrollablePanel {
 public:
  explicit BadgeScreen(ApplicationContext& context)
      : ScrollablePanel(context),
        content_(context, FlexDirection::kColumn),
        title_(context, "Material 3 badges", font_h6()),
        subtitle_(
            context,
            "Step 3 - dot, text, numbers, overlap, and unclipped overflow",
            font_caption()),
        switches_heading_(context, "Badged switches", font_body1()),
        switches_summary_(context,
                          "Host-family example via BadgedSwitch with top-end "
                          "and top-start placement",
                          font_caption()),
        dot_row_(context, "Dot badge", "setBadgeDot(), default kTopEnd", false),
        text_row_(context, "Text badge", "setBadgeText(\"NEW\"), kTopStart", true),
        value_row_(context, "Number badge", "setBadgeValue(42)", true),
        truncation_row_(context, "Truncation", "setBadgeValue(1000) -> 999+",
                        false),
        overlap_divider_(context),
        overlap_heading_(context, "Owner-painted badge", font_body1()),
        overlap_summary_(context,
                         "Raw Badge helper settled before lower-z owner paint "
                         "in the same widget",
                         font_caption()),
        overlap_card_(context, "IN"),
        overflow_divider_(context),
        overflow_heading_(context, "Overflow and clipping", font_body1()),
        overflow_summary_(context,
                          "Same overhang: clipped on the left, visible on the "
                          "right with kUnclipped",
                          font_caption()),
        overflow_row_(context, FlexDirection::kRow),
        clipped_(context, "Clipped"),
        unclipped_(context, "Unclipped", true) {
    content_.setPadding(Padding(Scaled(12), Scaled(8)));
    content_.setGap(Scaled(6));

    overflow_row_.setGap(Scaled(12));
    overflow_row_.setPadding(Padding(Scaled(0), Scaled(4)));

    text_row_.control().setBadgePlacement(
        {.gravity = material3::BadgeGravity::kTopStart});

    dot_row_.control().setBadgeDot();
    text_row_.control().setBadgeText("NEW");
    value_row_.control().setBadgeValue(42);
    truncation_row_.control().setBadgeValue(1000);

    overlap_card_.setBadgeText("NEW");

    material3::BadgePlacement overhang;
    overhang.gravity = material3::BadgeGravity::kTopEnd;
    overhang.horizontal_offset = -16;
    overhang.vertical_offset = -20;
    clipped_.card().setBadgeValue(1000);
    clipped_.card().setBadgePlacement(overhang);
    unclipped_.card().setBadgeValue(1000);
    unclipped_.card().setBadgePlacement(overhang);

    content_.add(title_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(switches_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(switches_summary_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(dot_row_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(text_row_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(value_row_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(truncation_row_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(overlap_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(overlap_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(overlap_summary_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(overlap_card_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(overflow_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(overflow_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(overflow_summary_, {.flex_grow = 0, .flex_shrink = 0});

    overflow_row_.add(clipped_, {.flex_grow = 0, .flex_shrink = 0});
    overflow_row_.add(unclipped_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(overflow_row_, {.flex_grow = 0, .flex_shrink = 0});

    setContents(content_);
  }

 private:
  FlexLayout content_;
  roo_windows::TextLabel title_;
  roo_windows::TextLabel subtitle_;
  roo_windows::TextLabel switches_heading_;
  roo_windows::TextLabel switches_summary_;
  SwitchBadgeRow dot_row_;
  SwitchBadgeRow text_row_;
  SwitchBadgeRow value_row_;
  SwitchBadgeRow truncation_row_;
  HorizontalDivider overlap_divider_;
  roo_windows::TextLabel overlap_heading_;
  roo_windows::TextLabel overlap_summary_;
  BadgeCard overlap_card_;
  HorizontalDivider overflow_divider_;
  roo_windows::TextLabel overflow_heading_;
  roo_windows::TextLabel overflow_summary_;
  FlexLayout overflow_row_;
  BadgeCardColumn clipped_;
  BadgeCardColumn unclipped_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
BadgeScreen badge_screen(app.context());
SingletonActivity activity(app, badge_screen);

void setup() {
  initDisplay();
  app.start();

  // Never exits.
  scheduler.run();
}

void loop() {}
