// Learning goal: own and position the lightweight Badge helper inside a
// custom-painted widget when no standard badged component fits the task.

// *************** EMULATOR SETUP BEGIN

#ifdef ROO_TESTING

#include "roo_testing/devices/display/ili9341/ili9341spi.h"
#include "roo_testing/devices/touch/xpt2046/xpt2046spi.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/transducers/ui/viewport/flex_viewport.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"
#include "roo_windows/fake/fltk_key_source.h"

using roo_testing_transducers::FlexViewport;
using roo_testing_transducers::FltkViewport;

struct Emulator {
  FltkViewport viewport;
  FlexViewport flex_viewport;
  FakeIli9341Spi display;
  FakeXpt2046Spi touch;

  Emulator()
      : viewport(),
        flex_viewport(viewport, 1, FlexViewport::kRotationRight),
        display(flex_viewport),
        touch(flex_viewport, FakeXpt2046Spi::Calibration(269, 249, 3829, 3684,
                                                         true, false, false)) {
    FakeEsp32().attachSpiDevice(display, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(7, display.cs());
    FakeEsp32().gpio.attachOutput(2, display.dc());
    FakeEsp32().gpio.attachOutput(3, display.rst());
    FakeEsp32().attachSpiDevice(touch, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(1, touch.cs());
  }
} emulator;

roo_windows::fake::FltkKeySource emulator_keys;

#endif

// *************** DISPLAY SETUP BEGIN

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_icons/outlined/24/social.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

// Change these pins and the touch calibration for your display board.
static constexpr int kCsPin = 7;
static constexpr int kDcPin = 2;
static constexpr int kRstPin = 3;
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
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows/widgets/text_block.h"

namespace {

/// Custom equipment tile that owns a badge and anchors it to its icon.
class AlertTile : public Widget {
 public:
  /// Creates a tile showing the specified notification count.
  AlertTile(ApplicationContext& context, unsigned int alert_count)
      : Widget(context), icon_(ic_outlined_24_social_notifications()) {
    badge_.setValue(alert_count);
  }

  /// Reserves enough room for the tile, icon, and icon-corner badge.
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(Scaled(96), Scaled(80));
  }

  /// Paints the badge and icon above the custom tile surface.
  void paint(PaintContext& ctx) const override {
    // Paint front-most content first; its exclusions protect it when the tile
    // background is painted later by roo_windows' reverse paint model.
    badge_.paint(ctx, theme());

    Rect anchor = iconBounds();
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
    ctx.fillRect(bounds(), theme().material3Theme().color.primaryContainer);
  }

 protected:
  void onLayout(bool changed, const Rect& rect) override {
    (void)changed;
    (void)rect;
    // layoutForIcon applies Material's standard icon-corner offset. Use
    // Badge::layout(anchor, alignment) for a deliberately custom offset.
    badge_.layoutForIcon(iconBounds());
  }

 private:
  // Returns the icon square that also acts as the badge anchor.
  Rect iconBounds() const {
    const int16_t side = Scaled(40);
    const int16_t left = bounds().xMin() + (bounds().width() - side) / 2;
    const int16_t top = bounds().yMin() + (bounds().height() - side) / 2;
    return Rect(left, top, left + side - 1, top + side - 1);
  }

  material3::Badge badge_;
  const roo_display::Pictogram& icon_;
};

/// Screen demonstrating the advanced custom-owner badge pattern.
class CustomBadgeAnchor : public FlexLayout {
 public:
  /// Creates a custom alert tile with three outstanding notifications.
  explicit CustomBadgeAnchor(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Chemical feeder", material3::text_style_title_large()),
        guidance_(context, "Custom tile · 3 maintenance alerts",
                  material3::text_style_body_medium(), kTop | kLeft),
        tile_(context, 3) {
    setPadding(Padding(Scaled(16), Scaled(16)));
    setGap(Scaled(12));
    setAlignItems(AlignItems::kCenter);
    add(title_);
    add(guidance_);
    add(tile_);
  }

 private:
  TextLabel title_;
  TextBlock guidance_;
  AlertTile tile_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
CustomBadgeAnchor custom_badge_anchor(app.context());
Task& task = app.addTaskFullScreen(custom_badge_anchor);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
