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
#include "roo_icons/outlined/24/action.h"
#include "roo_icons/outlined/24/social.h"
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
#include "roo_windows/material3/theme.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/text_label.h"

namespace {

Rect EmptyRect() { return Rect(0, 0, -1, -1); }

roo_display::Alignment IconBadgeAlignment(material3::BadgeMode mode) {
  return mode == material3::BadgeMode::kDot
             ? roo_display::kRight | roo_display::kTop
             : roo_display::kLeft.toRight().shiftBy(Scaled(-12)) |
                   roo_display::kBottom.toTop().shiftBy(Scaled(14));
}

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

class BadgeCard : public Widget {
 public:
  BadgeCard(ApplicationContext& context, const roo_display::Pictogram& icon,
            bool unclipped = false)
      : Widget(context), icon_(icon) {
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

  void setBadgeAlignment(roo_display::Alignment alignment) {
    alignment_ = alignment;
    use_icon_placement_ = false;
    requestLayout();
    setDirty();
  }

  Insets getInkInsets() const override {
    if (!badge_.visible()) return Insets::Zero();
    Rect conservative = material3::Badge::ConservativeBounds(
        anchorBounds(),
        use_icon_placement_ ? IconBadgeAlignment(badge_.mode()) : alignment_,
        badge_.mode() == material3::BadgeMode::kText);
    return InsetsFromEnvelope(bounds(), conservative);
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(Scaled(52), Scaled(44));
  }

  void paint(PaintContext& ctx) const override {
    // Settle the front-most badge and icon before painting the card behind
    // them. Their exclusions protect those pixels from the later surfaces.
    badge_.paint(ctx, theme());

    Rect anchor = anchorBounds();
    PaintContext icon_context = ctx.clipped(anchor);
    icon_context.setBgcolor(theme().material3Theme().color.secondaryContainer);
    roo_display::Pictogram icon(icon_);
    icon.color_mode().setColor(roo_display::AlphaBlend(
        icon_context.bgcolor(),
        theme().material3Theme().color.onSecondaryContainer));
    icon_context.drawTiled(icon, anchor,
                           roo_display::kCenter | roo_display::kMiddle,
                           isInvalidated());
    ctx.addExclusion(anchor);

    uint8_t outer_radius = static_cast<uint8_t>(Scaled(10));
    PaintRoundedSurface(ctx, bounds(), outer_radius,
                        theme().material3Theme().color.primaryContainer);

    uint8_t inner_radius = static_cast<uint8_t>(Scaled(8));
    PaintRoundedSurface(ctx, anchor, inner_radius,
                        theme().material3Theme().color.secondaryContainer);
  }

 protected:
  void onLayout(bool changed, const Rect& rect) override {
    (void)changed;
    (void)rect;
    if (use_icon_placement_) {
      badge_.layoutForIcon(anchorBounds());
    } else {
      badge_.layout(anchorBounds(), alignment_);
    }
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
  roo_display::Alignment alignment_ = roo_display::kRight | roo_display::kTop;
  bool use_icon_placement_ = true;
  const roo_display::Pictogram& icon_;
};

class BadgeCardColumn : public FlexLayout {
 public:
  BadgeCardColumn(ApplicationContext& context, const char* title,
                  const roo_display::Pictogram& icon,
                  bool unclipped = false)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, title, material2::text_style_caption()),
        card_(context, icon, unclipped) {
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

class BadgeIconRow : public FlexLayout {
 public:
  BadgeIconRow(ApplicationContext& context, const char* primary,
               const char* secondary, const roo_display::Pictogram& icon)
      : FlexLayout(context, FlexDirection::kRow),
        labels_(context, FlexDirection::kColumn),
        primary_(context, primary, material2::text_style_body1()),
        secondary_(context, secondary, material2::text_style_caption()),
        icon_(context, icon) {
    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(12));
    setAlignItems(AlignItems::kCenter);

    labels_.setGap(Scaled(2));
    labels_.setAlignItems(AlignItems::kFlexStart);
    labels_.add(primary_, {.flex_grow = 0, .flex_shrink = 0});
    labels_.add(secondary_, {.flex_grow = 0, .flex_shrink = 0});

    add(labels_, {.flex_grow = 1, .flex_shrink = 1});
    add(icon_, {.flex_grow = 0, .flex_shrink = 0});
  }

  BadgeCard& icon() { return icon_; }

 private:
  FlexLayout labels_;
  roo_windows::TextLabel primary_;
  roo_windows::TextLabel secondary_;
  BadgeCard icon_;
};

class BadgeScreen : public ScrollablePanel {
 public:
  explicit BadgeScreen(ApplicationContext& context)
      : ScrollablePanel(context),
        content_(context, FlexDirection::kColumn),
        title_(context, "Material 3 badges", material2::text_style_h6()),
        subtitle_(
            context,
            "Step 3 - dot, text, numbers, icon placement, and unclipped "
            "overflow",
            material2::text_style_caption()),
        icons_heading_(context, "Badged icons", material2::text_style_body1()),
        icons_summary_(context,
                       "Raw Badge helper owned by small roo icons",
                       material2::text_style_caption()),
        dot_row_(context, "Dot badge", "setBadgeDot()",
                 ic_outlined_24_social_notifications()),
        text_row_(context, "Text badge", "setBadgeText(\"NEW\")",
                  ic_outlined_24_action_bookmark()),
        value_row_(context, "Number badge", "setBadgeValue(42)",
                   ic_outlined_24_action_bookmark_added()),
        truncation_row_(context, "Truncation", "setBadgeValue(1000) -> 999+",
                        ic_outlined_24_social_notifications_none()),
        overlap_divider_(context),
        overlap_heading_(context, "Custom icon placement",
                         material2::text_style_body1()),
        overlap_summary_(context,
                         "Badge::layoutForIcon() places dot and text badges "
                         "using Material icon offsets",
                         material2::text_style_caption()),
        overlap_card_(context, ic_outlined_24_social_notifications()),
        overflow_divider_(context),
        overflow_heading_(context, "Overflow and clipping",
                          material2::text_style_body1()),
        overflow_summary_(context,
                          "Same overhang: clipped on the left, visible on the "
                          "right with kUnclipped",
                          material2::text_style_caption()),
        overflow_row_(context, FlexDirection::kRow),
        clipped_(context, "Clipped", ic_outlined_24_social_notifications()),
        unclipped_(context, "Unclipped",
                   ic_outlined_24_social_notifications(), true) {
    content_.setPadding(Padding(Scaled(12), Scaled(8)));
    content_.setGap(Scaled(6));

    overflow_row_.setGap(Scaled(12));
    overflow_row_.setPadding(Padding(Scaled(0), Scaled(4)));

    dot_row_.icon().setBadgeDot();
    text_row_.icon().setBadgeText("NEW");
    value_row_.icon().setBadgeValue(42);
    truncation_row_.icon().setBadgeValue(1000);
    overlap_card_.setBadgeText("NEW");

    const roo_display::Alignment overhang =
        (roo_display::kRight | roo_display::kTop).shiftBy(16, -20);
    clipped_.card().setBadgeValue(1000);
    clipped_.card().setBadgeAlignment(overhang);
    unclipped_.card().setBadgeValue(1000);
    unclipped_.card().setBadgeAlignment(overhang);

    content_.add(title_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(icons_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(icons_summary_, {.flex_grow = 0, .flex_shrink = 0});
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
  roo_windows::TextLabel icons_heading_;
  roo_windows::TextLabel icons_summary_;
  BadgeIconRow dot_row_;
  BadgeIconRow text_row_;
  BadgeIconRow value_row_;
  BadgeIconRow truncation_row_;
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
