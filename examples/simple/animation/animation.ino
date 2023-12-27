#include "Arduino.h"
#include "roo_display.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_scheduler;
using namespace roo_windows;
using namespace roo_time;

// Select the driver to match your display device.
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"

// Set your configuration for the driver.
static constexpr int kCsPin = 5;
static constexpr int kDcPin = 17;
static constexpr int kRstPin = 27;
static constexpr int kBlPin = 16;

static constexpr int kTouchCsPin = 2;

// Uncomment if you have connected the BL pin to GPIO.

#include "roo_display/backlit/esp32_ledc.h"
LedcBacklit backlit(kBlPin, /* ledc channel */ 0);

Ili9341spi<kCsPin, kDcPin, kRstPin> screen(Orientation().rotateLeft());
TouchXpt2046<kTouchCsPin> touch;

Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

#include "roo_display/color/color_mode_indexed.h"
#include "roo_display/core/raster.h"
#include "roo_display/shape/smooth.h"
#include "roo_display/ui/tile.h"
#include "roo_logging.h"
#include "roo_material_icons/filled/action.h"
#include "roo_material_icons/filled/alert.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/image.h"

Scheduler scheduler;
Environment env;
Application app(&env, display, scheduler);

// Animated image using a generic drawable (in this case, smooth thick arc).
class AnimatedArc : public Image {
 public:
  AnimatedArc(const Environment& env, roo_scheduler::Scheduler& scheduler)
      : Image(env),
        // Initial condition; doesn't matter much (as long as we're using the
        // correct extents) since we're updating it quickly.
        arc_(SmoothShape(), arc_extents(), kNoAlign),
        updater_(
            scheduler, [this]() { update(); }, Millis(20)) {
    setImage(&arc_);
    updater_.startInstantly();
  }

 private:
  constexpr Box arc_extents() const { return Box(-25, -25, 24, 24); }

  // Called periodically to generate subsequent image frames.
  void update() {
    static int64_t period_1 = 1700;
    static int64_t period_2 = 1000;
    static float period_rel = 1.0f / ((1.0f / period_2) - (1.0f / period_1));
    int64_t now = roo_time::Uptime::Now().inMillis() % (period_1 * period_2);
    int64_t remainder_1 = now % period_1;
    int64_t remainder_2 = now % period_2;
    float angle1 = 2.0 * M_PI * ((float)remainder_1 / period_1);
    float angle2 = 2.0 * M_PI * ((float)remainder_2 / period_2);
    bool reversed = fmodf(now / period_rel, 2.0f) >= 1;
    if (reversed) {
      std::swap(angle1, angle2);
    }
    if (angle2 < angle1) angle2 += 2.0f * M_PI;
    arc_ =
        MakeTileOf(SmoothThickArc({0, 0}, 15, 14, angle1, angle2,
                                  theme().color.primaryVariant, ENDING_ROUNDED),
                   arc_extents(), kNoAlign);
    invalidateInterior();
  }

  // We use the tile to put the center of the arc's curvature at an absolute
  // point.
  TileOf<SmoothShape> arc_;
  roo_scheduler::RepetitiveTask updater_;
};

// Custom, small two-bitmap sprite, using 2 bits per pixel, tiled into a larger
// horizontal bar, and 'moving' along it. See
// https://github.com/dejwk/roo_display/blob/master/doc/programming_guide.md#built-in-image-format
class Ghost : public Drawable {
 public:
  Ghost() : step_(0) {}

  Box extents() const override { return Box(0, 0, 300, 27); }

  // Advances the ghost to the next animation frame.
  void next() { step_ = (step_ + 1) % 100; }

  virtual void drawTo(const Surface& s) const {
    static const uint8_t ghost2[] PROGMEM = {
        0x00, 0x0F, 0xF0, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x03, 0xFF, 0xFF, 0xC0,
        0x0F, 0xD7, 0xF5, 0xF0, 0x0F, 0x55, 0xD5, 0x70, 0x0F, 0x5A, 0xD6, 0xB0,
        0x3F, 0x5A, 0xD6, 0xBC, 0x3F, 0xD7, 0xF5, 0xFC, 0x3F, 0xFF, 0xFF, 0xFC,
        0x3F, 0xFF, 0xFF, 0xFC, 0x3F, 0xFF, 0xFF, 0xFC, 0x3F, 0xFF, 0xFF, 0xFC,
        0x33, 0xFC, 0xFF, 0x3C, 0x00, 0xF0, 0x3C, 0x0C};

    static const uint8_t ghost1[] PROGMEM = {
        0x00, 0x0F, 0xF0, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x03, 0xFF, 0xFF, 0xC0,
        0x0F, 0xD7, 0xF5, 0xF0, 0x0F, 0x55, 0xD5, 0x70, 0x0F, 0x5A, 0xD6, 0xB0,
        0x3F, 0x5A, 0xD6, 0xBC, 0x3F, 0xD7, 0xF5, 0xFC, 0x3F, 0xFF, 0xFF, 0xFC,
        0x3F, 0xFF, 0xFF, 0xFC, 0x3F, 0xFF, 0xFF, 0xFC, 0x3F, 0xFF, 0xFF, 0xFC,
        0x3F, 0xCF, 0xF3, 0xFC, 0x0F, 0x03, 0xC0, 0xF0,
    };

    static Color colors[] PROGMEM = {color::Transparent, color::LightGray,
                                     color::Black, color::HotPink};

    static Palette palette = Palette::ReadOnly(colors, 4);

    // The ghost sprite, in one of two shapes.
    auto raster = ProgMemRaster<Indexed2>(
        16, 14, step_ % 2 == 0 ? ghost1 : ghost2, &palette);
    // We magnify the ghost 2x in both directions.
    TransformedDrawable magnified(Transformation().scale(2, 2), &raster);
    // Now embed the ghost shape into a horizontal tile.
    Tile tiled(&magnified, extents(), kLeft.shiftBy(step_ * 5 - 30));
    // Now draw the tile.
    // Note: the surface will be called with FILL_MODE_RECTANGLE. Because of
    // that, the background is drawn, so that the previous image of the ghost is
    // correctly overwritten. Due to the background fill optimization, the large
    // blank area of the tile is not actually drawn, saving time.
    s.drawObject(tiled);
  }

 private:
  int step_;
};

// Animated image using a custom drawable.
class GhostImage : public Image {
 public:
  GhostImage(const Environment& env, roo_scheduler::Scheduler& scheduler)
      : Image(env),
        updater_(
            scheduler, [this] { update(); }, Seconds(0.04)) {
    setImage(&ghost_);
    updater_.start();
  }

 private:
  // Called periodically to generate subsequent image frames.
  void update() {
    ghost_.next();
    invalidateInterior();
  }

  Ghost ghost_;
  roo_scheduler::RepetitiveTask updater_;
};

// A simple vertical pane containing three animated objects. Two of them are
// Images (based off roo_display::Drawable), implemented as custom classes that
// fully encapsulate their animation logic. The other demonstrates a simpler and
// also more general case: any widget can be animated simply by changing some of
// its properties, and it can be done without subclassing the widget, but
// rather, by controlling the animation from its container.
class MyPane : public VerticalLayout {
 public:
  MyPane(const Environment& env, Scheduler& scheduler)
      : VerticalLayout(env),
        arc_(env, scheduler),
        ghost_(env, scheduler),
        warning_icon_(env, ic_filled_36_alert_warning(), color::DarkOrange),
        icon_updater_(
            scheduler, [this]() { updateIcon(); }, Seconds(0.25)) {
    add(arc_);
    add(ghost_);

    add(warning_icon_);
    icon_updater_.startInstantly();
  }

 private:
  // Called periodically to toggle icon visibility.
  void updateIcon() {
    // Toggle visibility.
    warning_icon_.setVisibility(warning_icon_.isVisible() ? INVISIBLE
                                                          : VISIBLE);
  }

  AnimatedArc arc_;
  GhostImage ghost_;

  // For the warning icon, we control animation directly from the container. (We
  // could have subclassed the Icon instead, but for simple cases it may be
  // easier to do things this way.)
  Icon warning_icon_;
  roo_scheduler::RepetitiveTask icon_updater_;
};

MyPane my_pane(env, scheduler);

SingletonActivity activity(app, my_pane);

void setup() {
  SPI.begin();
  display.init();
}

void loop() {
  app.tick();
  scheduler.executeEligibleTasks(1);
}
