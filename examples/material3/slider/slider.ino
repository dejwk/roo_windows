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

#include <functional>

#include "Arduino.h"
#include "roo_display.h"
#include "roo_icons/outlined/24/device.h"
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

// Uncomment if you have connected the BL pin to GPIO.

// #include "roo_display/backlit/esp32_ledc.h"
// LedcBacklit backlit(kBlPin, /* ledc channel */ 0);

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
#include "roo_windows/containers/holder.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/slider/range_slider.h"
#include "roo_windows/material3/slider/slider.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/text_label.h"

namespace {

int PercentFromUnitValue(float value) {
  if (value < 0.0f) value = 0.0f;
  if (value > 1.0f) value = 1.0f;
  return (int)lroundf(value * 100.0f);
}

float UnitValueFromPercent(int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  return 0.01f * (float)percent;
}

class SliderRow : public FlexLayout {
 public:
  SliderRow(const Environment& env, const char* primary, const char* secondary,
            int initial_percent)
      : FlexLayout(env, FlexDirection::kColumn),
        header_(env, FlexDirection::kRow),
        labels_(env, FlexDirection::kColumn),
        primary_(env, primary, font_body1()),
        secondary_(env, secondary, font_caption()),
        value_(env, "", font_body1()),
        slider_(env, material3::SliderRange{},
                UnitValueFromPercent(initial_percent)) {
    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(8));

    header_.setAlignItems(AlignItems::kCenter);
    header_.setGap(Scaled(12));

    labels_.setAlignItems(AlignItems::kFlexStart);
    labels_.add(primary_, {.flex_grow = 0, .flex_shrink = 0});
    labels_.add(secondary_, {.flex_grow = 0, .flex_shrink = 0});

    header_.add(labels_, {.flex_grow = 1, .flex_shrink = 1});
    header_.add(value_, {.flex_grow = 0, .flex_shrink = 0});

    add(header_, {.flex_grow = 0, .flex_shrink = 0});
    add(slider_, {.flex_grow = 0, .flex_shrink = 1});

    slider_.setOnInteractiveChange([this]() { handleInteractiveChange(); });
    updateValue();
  }

  void setOnInteractiveChange(std::function<void()> handler) {
    on_change_ = std::move(handler);
  }

  int percent() const { return PercentFromUnitValue(slider_.value()); }

 private:
  void updateValue() { value_.setTextf("%d%%", percent()); }

  void handleInteractiveChange() {
    updateValue();
    if (on_change_ != nullptr) {
      on_change_();
    }
  }

  FlexLayout header_;
  FlexLayout labels_;
  TextLabel primary_;
  TextLabel secondary_;
  TextLabel value_;
  material3::Slider slider_;
  std::function<void()> on_change_;
};

using SliderValueFormatter =
    std::function<void(TextLabel&, const material3::Slider&)>;

// Slider subclass that lets the demo install a custom formatLabel function
// to render the value indicator bubble (e.g. "3x", "+25") shown above the
// thumb during interaction.
class DemoSlider : public material3::Slider {
 public:
  using LabelFormatter = std::function<roo::string_view(float, char*, size_t)>;

  DemoSlider(const Environment& env, material3::SliderRange range,
             float initial_value, material3::SliderVariant variant,
             material3::SliderStyle style)
      : material3::Slider(env, range, initial_value, variant, style) {}

  void setLabelFormatter(LabelFormatter formatter) {
    label_formatter_ = std::move(formatter);
  }

  roo::string_view formatLabel(float value, char* scratch,
                               size_t scratch_size) const override {
    if (label_formatter_ != nullptr) {
      return label_formatter_(value, scratch, scratch_size);
    }
    return material3::Slider::formatLabel(value, scratch, scratch_size);
  }

 private:
  LabelFormatter label_formatter_;
};

class DemoIconSlider : public material3::SliderWithInsetIcon {
 public:
  using LabelFormatter = DemoSlider::LabelFormatter;

  DemoIconSlider(const Environment& env, material3::SliderRange range,
                 float initial_value, material3::SliderVariant variant,
                 material3::SliderStyle style)
      : material3::SliderWithInsetIcon(env, range, initial_value, variant,
                                       style) {}

  void setLabelFormatter(LabelFormatter formatter) {
    label_formatter_ = std::move(formatter);
  }

  roo::string_view formatLabel(float value, char* scratch,
                               size_t scratch_size) const override {
    if (label_formatter_ != nullptr) {
      return label_formatter_(value, scratch, scratch_size);
    }
    return material3::SliderWithInsetIcon::formatLabel(value, scratch,
                                                       scratch_size);
  }

 private:
  LabelFormatter label_formatter_;
};

class SemanticSliderRow : public FlexLayout {
 public:
  using LabelFormatter = DemoSlider::LabelFormatter;

  SemanticSliderRow(
      const Environment& env, const char* primary, const char* secondary,
      material3::SliderRange range, float initial_value,
      SliderValueFormatter formatter,
      material3::SliderVariant variant = material3::SliderVariant::kStandard,
      material3::SliderStyle style = {},
      LabelFormatter label_formatter = nullptr)
      : FlexLayout(env, FlexDirection::kColumn),
        header_(env, FlexDirection::kRow),
        labels_(env, FlexDirection::kColumn),
        primary_(env, primary, font_body1()),
        secondary_(env, secondary, font_caption()),
        value_(env, "", font_body1()),
        slider_(env, range, initial_value, variant, style),
        formatter_(std::move(formatter)) {
    if (label_formatter != nullptr) {
      slider_.setLabelFormatter(std::move(label_formatter));
    }
    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(8));

    header_.setAlignItems(AlignItems::kCenter);
    header_.setGap(Scaled(12));

    labels_.setAlignItems(AlignItems::kFlexStart);
    labels_.add(primary_, {.flex_grow = 0, .flex_shrink = 0});
    labels_.add(secondary_, {.flex_grow = 0, .flex_shrink = 0});

    header_.add(labels_, {.flex_grow = 1, .flex_shrink = 1});
    header_.add(value_, {.flex_grow = 0, .flex_shrink = 0});

    add(header_, {.flex_grow = 0, .flex_shrink = 0});
    add(slider_, {.flex_grow = 0, .flex_shrink = 1});

    slider_.setOnInteractiveChange([this]() { handleInteractiveChange(); });
    updateValue();
  }

 private:
  void updateValue() { formatter_(value_, slider_); }

  void handleInteractiveChange() { updateValue(); }

  FlexLayout header_;
  FlexLayout labels_;
  TextLabel primary_;
  TextLabel secondary_;
  TextLabel value_;
  DemoSlider slider_;
  SliderValueFormatter formatter_;
};

class IconSliderRow : public FlexLayout {
 public:
  using LabelFormatter = DemoIconSlider::LabelFormatter;

  IconSliderRow(const Environment& env, const char* primary,
                const char* secondary, material3::SliderRange range,
                float initial_value, SliderValueFormatter formatter,
                const material3::InsetIcon* inset_icon,
                material3::SliderStyle style = {},
                LabelFormatter label_formatter = nullptr)
      : FlexLayout(env, FlexDirection::kColumn),
        header_(env, FlexDirection::kRow),
        labels_(env, FlexDirection::kColumn),
        primary_(env, primary, font_body1()),
        secondary_(env, secondary, font_caption()),
        value_(env, "", font_body1()),
        slider_(env, range, initial_value, material3::SliderVariant::kStandard,
                style),
        formatter_(std::move(formatter)) {
    if (inset_icon != nullptr) {
      slider_.setIcon(inset_icon->icon, inset_icon->anchor);
    }
    if (label_formatter != nullptr) {
      slider_.setLabelFormatter(std::move(label_formatter));
    }
    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(8));

    header_.setAlignItems(AlignItems::kCenter);
    header_.setGap(Scaled(12));

    labels_.setAlignItems(AlignItems::kFlexStart);
    labels_.add(primary_, {.flex_grow = 0, .flex_shrink = 0});
    labels_.add(secondary_, {.flex_grow = 0, .flex_shrink = 0});

    header_.add(labels_, {.flex_grow = 1, .flex_shrink = 1});
    header_.add(value_, {.flex_grow = 0, .flex_shrink = 0});

    add(header_, {.flex_grow = 0, .flex_shrink = 0});
    add(slider_, {.flex_grow = 0, .flex_shrink = 1});

    slider_.setOnInteractiveChange([this]() { handleInteractiveChange(); });
    updateValue();
  }

 private:
  void updateValue() { formatter_(value_, slider_); }

  void handleInteractiveChange() { updateValue(); }

  FlexLayout header_;
  FlexLayout labels_;
  TextLabel primary_;
  TextLabel secondary_;
  TextLabel value_;
  DemoIconSlider slider_;
  SliderValueFormatter formatter_;
};

class DemoRangeSlider : public material3::RangeSlider {
 public:
  using UpdateHandler = std::function<void()>;

  DemoRangeSlider(const Environment& env, material3::SliderRange range,
                  float start_value, float end_value,
                  material3::SliderStyle style)
      : material3::RangeSlider(env, range, start_value, end_value, style) {}

  // Render bubble values as "08:00" / "18:00" rather than the default decimal.
  roo::string_view formatLabel(float value, char* scratch,
                               size_t scratch_size) const override {
    int hours = (int)value;
    int n = snprintf(scratch, scratch_size, "%02d:00", hours);
    if (n < 0) n = 0;
    if ((size_t)n >= scratch_size) n = (int)scratch_size - 1;
    return roo::string_view(scratch, (size_t)n);
  }

  void setOnStateChange(UpdateHandler handler) {
    on_state_change_ = std::move(handler);
  }

  void onValueChange(float start, float end, int active_thumb,
                     bool from_user) override {
    (void)start;
    (void)end;
    (void)active_thumb;
    (void)from_user;
    notifyStateChange();
  }

  void onInteractionStart(int active_thumb) override {
    (void)active_thumb;
    notifyStateChange();
  }

  void onInteractionEnd(float start, float end) override {
    (void)start;
    (void)end;
    notifyStateChange();
  }

 private:
  void notifyStateChange() {
    if (on_state_change_ != nullptr) {
      on_state_change_();
    }
  }

  UpdateHandler on_state_change_;
};

class RangeSliderRow : public FlexLayout {
 public:
  RangeSliderRow(const Environment& env, const char* primary,
                 const char* secondary, material3::SliderRange range,
                 float start_value, float end_value, float min_separation,
                 material3::SliderStyle style = {})
      : FlexLayout(env, FlexDirection::kColumn),
        header_(env, FlexDirection::kRow),
        labels_(env, FlexDirection::kColumn),
        primary_(env, primary, font_body1()),
        secondary_(env, secondary, font_caption()),
        value_(env, "", font_body1()),
        slider_(env, range, start_value, end_value, style),
        status_(env, "", font_caption()),
        min_separation_(min_separation) {
    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(8));

    header_.setAlignItems(AlignItems::kCenter);
    header_.setGap(Scaled(12));

    labels_.setAlignItems(AlignItems::kFlexStart);
    labels_.add(primary_, {.flex_grow = 0, .flex_shrink = 0});
    labels_.add(secondary_, {.flex_grow = 0, .flex_shrink = 0});

    header_.add(labels_, {.flex_grow = 1, .flex_shrink = 1});
    header_.add(value_, {.flex_grow = 0, .flex_shrink = 0});

    add(header_, {.flex_grow = 0, .flex_shrink = 0});
    add(slider_, {.flex_grow = 0, .flex_shrink = 1});
    add(status_, {.flex_grow = 0, .flex_shrink = 0});

    slider_.setMinSeparation(min_separation);
    slider_.setOnStateChange([this]() { updateLabels(); });
    slider_.setOnInteractiveChange([this]() { updateLabels(); });
    updateLabels();
  }

 private:
  const char* ActiveThumbLabel() const {
    int active_thumb = slider_.activeThumbIndex();
    if (active_thumb == 0) return "adjusting start thumb";
    if (active_thumb == 1) return "adjusting end thumb";
    return "idle";
  }

  void updateLabels() {
    value_.setTextf("%02d:00-%02d:00", (int)slider_.startValue(),
                    (int)slider_.endValue());
    status_.setTextf("%s · minimum gap %dh", ActiveThumbLabel(),
                     (int)min_separation_);
  }

  FlexLayout header_;
  FlexLayout labels_;
  TextLabel primary_;
  TextLabel secondary_;
  TextLabel value_;
  DemoRangeSlider slider_;
  TextLabel status_;
  float min_separation_;
};

class FixedSizeHolder : public Holder {
 public:
  FixedSizeHolder(const Environment& env, XDim width, YDim height,
                  XDim left_inset = 0, YDim top_inset = 0)
      : Holder(env),
        width_(width),
        height_(height),
        left_inset_(left_inset),
        top_inset_(top_inset) {}

 protected:
  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::ExactWidth(width_),
                         PreferredSize::ExactHeight(height_));
  }

  void onLayout(bool changed, const Rect& rect) override {
    (void)changed;
    Widget* c = contents();
    if (c == nullptr) return;
    Margins m = c->getMargins();
    Padding p = getPadding();
    XDim w = rect.width() - m.left() - m.right() - p.left() - p.right() -
             left_inset_;
    YDim h = rect.height() - m.top() - m.bottom() - p.top() - p.bottom() -
             top_inset_;
    if (w < 0) w = 0;
    if (h < 0) h = 0;
    Rect bounds(left_inset_, top_inset_, left_inset_ + w - 1,
                top_inset_ + h - 1);
    bounds = bounds.translate(m.left() + p.left(), m.top() + p.top());
    c->layout(bounds);
  }

 private:
  XDim width_;
  YDim height_;
  XDim left_inset_;
  YDim top_inset_;
};

class VerticalSliderShowcase : public FlexLayout {
 public:
  using LabelFormatter = DemoSlider::LabelFormatter;

  VerticalSliderShowcase(const Environment& env, const char* primary,
                         const char* secondary, const char* detail,
                         material3::SliderRange range, float initial_value,
                         SliderValueFormatter formatter,
                         material3::SliderStyle style = {},
                         LabelFormatter label_formatter = nullptr)
      : FlexLayout(env, FlexDirection::kColumn),
        header_(env, FlexDirection::kRow),
        labels_(env, FlexDirection::kColumn),
        primary_(env, primary, font_body1()),
        secondary_(env, secondary, font_caption()),
        value_(env, "", font_body1()),
        body_(env, FlexDirection::kRow),
        slider_slot_(env, Scaled(104), Scaled(168), Scaled(56)),
        slider_(env, range, initial_value, material3::SliderVariant::kStandard,
                style),
        detail_(env, detail, font_caption()),
        formatter_(std::move(formatter)) {
    if (label_formatter != nullptr) {
      slider_.setLabelFormatter(std::move(label_formatter));
    }

    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(8));

    header_.setAlignItems(AlignItems::kCenter);
    header_.setGap(Scaled(12));

    labels_.setAlignItems(AlignItems::kFlexStart);
    labels_.add(primary_, {.flex_grow = 0, .flex_shrink = 0});
    labels_.add(secondary_, {.flex_grow = 0, .flex_shrink = 0});

    header_.add(labels_, {.flex_grow = 1, .flex_shrink = 1});
    header_.add(value_, {.flex_grow = 0, .flex_shrink = 0});

    body_.setAlignItems(AlignItems::kCenter);
    body_.setGap(Scaled(16));
    slider_slot_.setContents(WidgetRef(slider_));
    body_.add(slider_slot_, {.flex_grow = 0, .flex_shrink = 0});
    body_.add(detail_, {.flex_grow = 1, .flex_shrink = 1});

    add(header_, {.flex_grow = 0, .flex_shrink = 0});
    add(body_, {.flex_grow = 0, .flex_shrink = 0});

    slider_.setOnInteractiveChange([this]() { updateValue(); });
    updateValue();
  }

 private:
  void updateValue() { formatter_(value_, slider_); }

  FlexLayout header_;
  FlexLayout labels_;
  TextLabel primary_;
  TextLabel secondary_;
  TextLabel value_;
  FlexLayout body_;
  FixedSizeHolder slider_slot_;
  DemoSlider slider_;
  TextLabel detail_;
  SliderValueFormatter formatter_;
};

class FullWidthColumn : public FlexLayout {
 public:
  explicit FullWidthColumn(const Environment& env)
      : FlexLayout(env, FlexDirection::kColumn) {}

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }
};

// Style builders kept out-of-line so they can be passed by value into row
// constructors.
constexpr material3::SliderStyle FanSpeedStyle() {
  material3::SliderStyle s{};
  s.size = material3::SliderSize::kLarge;
  s.tick_mode = material3::SliderTickMode::kShowTicks;
  s.value_indicator =
      material3::SliderValueIndicatorBehavior::kShowOnInteraction;
  return s;
}

constexpr material3::SliderStyle BalanceStyle() {
  material3::SliderStyle s{};
  s.value_indicator = material3::SliderValueIndicatorBehavior::kWithinBounds;
  return s;
}

constexpr material3::SliderStyle QuietHoursStyle() {
  material3::SliderStyle s{};
  s.value_indicator =
      material3::SliderValueIndicatorBehavior::kShowOnInteraction;
  return s;
}

constexpr material3::SliderStyle TankLevelStyle() {
  material3::SliderStyle s{};
  s.orientation = material3::SliderOrientation::kVertical;
  s.direction = material3::SliderDirection::kInverted;
  s.tick_mode = material3::SliderTickMode::kShowTicks;
  s.value_indicator =
      material3::SliderValueIndicatorBehavior::kShowOnInteraction;
  return s;
}

constexpr material3::SliderStyle TemperatureIconStyle() {
  material3::SliderStyle s{};
  s.size = material3::SliderSize::kLarge;
  s.value_indicator =
      material3::SliderValueIndicatorBehavior::kShowOnInteraction;
  return s;
}

constexpr material3::SliderStyle BrightnessIconStyle() {
  material3::SliderStyle s{};
  s.size = material3::SliderSize::kMedium;
  s.value_indicator =
      material3::SliderValueIndicatorBehavior::kShowOnInteraction;
  return s;
}

constexpr material3::SliderStyle XLInvertedContinuousStyle() {
  material3::SliderStyle s{};
  s.size = material3::SliderSize::kExtraLarge;
  s.direction = material3::SliderDirection::kInverted;
  s.value_indicator =
      material3::SliderValueIndicatorBehavior::kShowOnInteraction;
  return s;
}

const material3::InsetIcon* ThermostatIcon() {
  static const material3::InsetIcon icon = {
      &ic_outlined_24_device_thermostat()};
  return &icon;
}

const material3::InsetIcon* BrightnessIcon() {
  static const material3::InsetIcon icon = {
      &ic_outlined_24_device_brightness_high(),
  };
  return &icon;
}

class SliderScreen : public SimpleScrollablePanel {
 public:
  SliderScreen(const Environment& env)
      : SimpleScrollablePanel(env),
        content_(env),
        title_(env, "Material 3 sliders", font_body1()),
        subtitle_(env,
                  "Unit-range, semantic, discrete, centered, vertical, "
                  "range, and iconized sliders, with custom value indicator "
                  "labels.",
                  font_caption()),
        divider_(env),
        migration_(env,
                   "The first row uses the default unit range. The rest use "
                   "custom semantic ranges.",
                   font_caption()),
        legacy_(env, "Default unit range",
                "0..100% mapped through the semantic value API", 72),
        fan_speed_(
            env, "Discrete fan speed",
            "Ticks + stop indicators with a custom \"Nx\" bubble",
            material3::SliderRange{0.0f, 5.0f, 1.0f}, 2.0f,
            [](TextLabel& label, const material3::Slider& slider) {
              label.setTextf("%.0fx", slider.value());
            },
            material3::SliderVariant::kStandard, FanSpeedStyle(),
            [](float value, char* scratch, size_t n) {
              int len = snprintf(scratch, n, "%.0fx", value);
              if (len < 0) len = 0;
              if ((size_t)len >= n) len = (int)n - 1;
              return roo::string_view(scratch, (size_t)len);
            }),
        balance_(
            env, "Centered balance",
            "kWithinBounds: bubble stays above the track and clamps horizontally",
            material3::SliderRange{-100.0f, 100.0f, 5.0f}, -20.0f,
            [](TextLabel& label, const material3::Slider& slider) {
              label.setTextf("%+.0f", slider.value());
            },
            material3::SliderVariant::kCentered, BalanceStyle(),
            [](float value, char* scratch, size_t n) {
              int len = snprintf(scratch, n, "%+.0f", value);
              if (len < 0) len = 0;
              if ((size_t)len >= n) len = (int)n - 1;
              return roo::string_view(scratch, (size_t)len);
            }),
        heating_(
            env, "Heating setpoint", "Large slider with a leading inset icon",
            material3::SliderRange{18.0f, 30.0f, 1.0f}, 24.0f,
            [](TextLabel& label, const material3::Slider& slider) {
              label.setTextf("%.0f C", slider.value());
            },
            ThermostatIcon(), TemperatureIconStyle(),
            [](float value, char* scratch, size_t n) {
              int len = snprintf(scratch, n, "%.0f C", value);
              if (len < 0) len = 0;
              if ((size_t)len >= n) len = (int)n - 1;
              return roo::string_view(scratch, (size_t)len);
            }),
        brightness_(
            env, "Desk lamp brightness",
            "Large slider with a leading inset brightness icon",
            material3::SliderRange{0.0f, 100.0f, 5.0f}, 65.0f,
            [](TextLabel& label, const material3::Slider& slider) {
              label.setTextf("%.0f%%", slider.value());
            },
            BrightnessIcon(), BrightnessIconStyle(),
            [](float value, char* scratch, size_t n) {
              int len = snprintf(scratch, n, "%.0f%%", value);
              if (len < 0) len = 0;
              if ((size_t)len >= n) len = (int)n - 1;
              return roo::string_view(scratch, (size_t)len);
            }),
        inverted_xl_(
            env, "XL inverted continuous",
            "Extra-large thumb with right-to-left continuous travel",
            material3::SliderRange{0.0f, 100.0f}, 35.0f,
            [](TextLabel& label, const material3::Slider& slider) {
              label.setTextf("%.0f%%", slider.value());
            },
            material3::SliderVariant::kStandard, XLInvertedContinuousStyle(),
            [](float value, char* scratch, size_t n) {
              int len = snprintf(scratch, n, "%.0f%%", value);
              if (len < 0) len = 0;
              if ((size_t)len >= n) len = (int)n - 1;
              return roo::string_view(scratch, (size_t)len);
            }),
        tank_level_(
            env, "Vertical tank level",
            "Explicit inverted direction with sparse 10% ticks",
            "Tap anywhere on the column to jump. The vertical slider keeps "
            "the same 0-100% domain, but uses larger 10% steps so the tick "
            "marks stay readable.",
            material3::SliderRange{0.0f, 100.0f, 10.0f}, 60.0f,
            [](TextLabel& label, const material3::Slider& slider) {
              label.setTextf("%.0f%%", slider.value());
            },
            TankLevelStyle(),
            [](float value, char* scratch, size_t n) {
              int len = snprintf(scratch, n, "%.0f%%", value);
              if (len < 0) len = 0;
              if ((size_t)len >= n) len = (int)n - 1;
              return roo::string_view(scratch, (size_t)len);
            }),
        quiet_hours_(env, "Quiet hours",
                     "Range slider: HH:00 indicator on the active thumb",
                     material3::SliderRange{0.0f, 24.0f, 1.0f}, 8.0f, 18.0f,
                     2.0f, QuietHoursStyle()),
        footer_divider_(env),
        note_(env,
              "Drag a thumb to see the value indicator bubble float next to "
              "the active thumb. Discrete sliders now show stop indicators by "
              "default, inset icons are available on larger standard sliders, "
              "and direction can be configured independently from orientation.",
              font_caption()) {
    content_.setPadding(Scaled(12));
    content_.setGap(Scaled(4));

    content_.add(title_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(migration_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(legacy_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(fan_speed_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(balance_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(heating_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(brightness_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(inverted_xl_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(tank_level_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(quiet_hours_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(footer_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(note_, {.flex_grow = 0, .flex_shrink = 0});

    setContents(content_);
  }

  FullWidthColumn content_;
  TextLabel title_;
  TextLabel subtitle_;
  HorizontalDivider divider_;
  TextLabel migration_;
  SliderRow legacy_;
  SemanticSliderRow fan_speed_;
  SemanticSliderRow balance_;
  IconSliderRow heating_;
  IconSliderRow brightness_;
  SemanticSliderRow inverted_xl_;
  VerticalSliderShowcase tank_level_;
  RangeSliderRow quiet_hours_;
  HorizontalDivider footer_divider_;
  TextLabel note_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
SliderScreen slider_screen(env);
SingletonActivity activity(app, slider_screen);

void setup() {
  initDisplay();
  app.start();

  // Never exits.
  scheduler.run();
}

void loop() {}