// Learning goal: compare complete scrolling with opt-in background trails on
// a slow display, and see the log become fully current when movement pauses.
// This offline example accepts temporary lag in row decoration and pending
// foreground. It is experimental: judge readability on your actual display.

// *************** EMULATOR SETUP BEGIN

#ifdef ROO_TESTING

#include "roo_testing/devices/display/ili9341/ili9341spi.h"
#include "roo_testing/devices/touch/xpt2046/xpt2046spi.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/transducers/ui/viewport/flex_viewport.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"

using roo_testing_transducers::FlexViewport;
using roo_testing_transducers::FltkViewport;

/// Connects emulated SPI display and touch devices to the sketch pins.
struct Emulator {
  FltkViewport viewport;
  FlexViewport flex_viewport;
  FakeIli9341Spi display;
  FakeXpt2046Spi touch;

  /// Creates the rotated viewport and attaches both emulated devices.
  Emulator()
      : viewport({.noise_bits = 6}),
        flex_viewport(viewport, 1, FlexViewport::kRotationRight),
        display(flex_viewport),
        // Match the physical touch calibration in the rotated viewport.
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

#endif

// *************** DISPLAY SETUP BEGIN

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"

using namespace roo_display;

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

/// Initializes the shared SPI bus, display and touch controller.
void InitDisplay() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.enableTurbo();
  display.init();
}

// *************** SCROLLING COMPARISON

#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/containers/accelerated_scrollable_panel.h"
#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_windows;

namespace {

/// Presents a log entry on a contrasting rounded surface.
// Rows deliberately differ from the viewport surface. Their normal rounded
// outlines/shadows remain in the compositor when an optional fill skips.
class LogRow : public FlexLayout {
 public:
  /// Creates entry @p sequence using @p context and alternating row colors.
  LogRow(ApplicationContext& context, int sequence)
      : FlexLayout(context, FlexDirection::kColumn),
        surface_(sequence % 2 == 0 ? Color(0xFFE0F2E9) : Color(0xFFF1E7DB)) {
    setPadding(Padding(Scaled(10), Scaled(8)));
    add(std::make_unique<TextLabel>(context,
                                    "Cycle " + std::to_string(sequence + 1),
                                    material3::text_style_title_small()));
    add(std::make_unique<TextLabel>(
        context, sequence % 2 == 0 ? "Pump running" : "Temperature sampled",
        material3::text_style_body_small()));
  }

  /// Supplies the alternating opaque surface under this entry.
  Color background() const override { return surface_; }

  /// Gives the entry an ordinary rounded outline.
  BorderStyle getBorderStyle() const override { return BorderStyle(12, 1); }

  /// Adds a compositor-managed card shadow.
  uint8_t getElevation() const override { return 1; }

  /// Keeps children inside this surface's rounded boundary.
  bool clipsChildrenToRoundedBounds() const override { return true; }

 private:
  Color surface_;
};

/// Arranges an offline log as a scrollable column of contrasting cards.
class LogRows : public FlexLayout {
 public:
  /// Creates forty log entries in @p context.
  explicit LogRows(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn) {
    setPadding(Padding(Scaled(8)));
    setGap(Scaled(10));
    for (int i = 0; i < 40; ++i) add(std::make_unique<LogRow>(context, i));
  }

  /// Matches the viewport width and measures the full log height.
  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::WrapContentHeight()};
  }

  /// Supplies a dark surface between log cards.
  Color background() const override { return Color(0xFF334155); }
};

// An enclosing rounded viewport demonstrates that clipping remains intact,
// including the top/bottom content and fractional boundary colors.
/// Clips both comparison views to the same rounded viewport.
class RoundedViewport : public AlignedLayout {
 public:
  /// Uses the normal AlignedLayout constructors for the rounded viewport.
  using AlignedLayout::AlignedLayout;

  /// Keeps children inside this surface's rounded boundary.
  bool clipsChildrenToRoundedBounds() const override { return true; }

  /// Rounds the viewport and gives it a thin outline.
  BorderStyle getBorderStyle() const override { return BorderStyle(20, 1); }
};

/// Lets the reader choose complete or accelerated scrolling of the same log.
class ScrollComparison : public FlexLayout {
 public:
  /// Creates the comparison controls and both scroll views in @p context.
  explicit ScrollComparison(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        complete_(context, std::make_unique<LogRows>(context)),
        accelerated_(context, std::make_unique<LogRows>(context)),
        complete_button_(context, "Complete", Button::TEXT),
        accelerated_button_(context, "Accelerated", Button::TEXT),
        controls_(context, FlexDirection::kRow),
        viewport_(context),
        mode_(context, "Complete redraw", material3::text_style_body_small()) {
    setPadding(Padding(Scaled(8)));
    setGap(Scaled(6));
    controls_.add(complete_button_, {.flex_grow = 1});
    controls_.add(accelerated_button_, {.flex_grow = 1});
    viewport_.add(complete_);
    viewport_.add(accelerated_);
    accelerated_.setVisibility(Visibility::kInvisible);
    add(controls_);
    add(mode_);
    add(viewport_, {.flex_grow = 1});

    // Both views retain independent scroll positions so you can repeat a drag
    // or fling in each mode. Switching modes requests an ordinary complete
    // reveal.
    complete_button_.setOnInteractiveChange([this]() { select(false); });
    accelerated_button_.setOnInteractiveChange([this]() { select(true); });
  }

  /// Detaches borrowed members before their destruction.
  ~ScrollComparison() override { removeAll(); }

 private:
  void select(bool accelerated) {
    complete_.setVisibility(accelerated ? Visibility::kInvisible
                                        : Visibility::kVisible);
    accelerated_.setVisibility(accelerated ? Visibility::kVisible
                                           : Visibility::kInvisible);
    mode_.setText(accelerated ? "Accelerated (16 ms)" : "Complete redraw");
  }

  // Declare borrowed children before their containing member so destruction
  // detaches them while they are still alive.
  SimpleScrollablePanel complete_;
  AcceleratedScrollablePanel accelerated_;
  SimpleButton complete_button_;
  SimpleButton accelerated_button_;
  FlexLayout controls_;
  RoundedViewport viewport_;
  TextLabel mode_;
};

}  // namespace

roo_scheduler::SchedulingService scheduler;
Environment env(scheduler);
Application app(&env, display);
ScrollComparison comparison(app.context());
Task& task = app.addTaskFullScreen(comparison);

void setup() {
  InitDisplay();
  // This single allowance includes animation, layout and drawing. Only the
  // explicitly accelerated view may omit eligible background operations.
  app.window().setAdvisoryPaintBudget(roo_time::Millis(16));
  app.start();
  scheduler.run();
}

void loop() {}
