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
#include "roo_icons/outlined/24/action.h"
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
#include "roo_windows/material3/button/button.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/text_label.h"

namespace {

void PrepareDemoButton(material3::Button& button) {
  // Remove the framework's default outer margins so the dense showcase fits on
  // screen and reflects the button tokens rather than container spacing.
  button.setMargins(MarginSize::kNone);
}

void PrepareIconDemoButton(material3::Button& button) {
  PrepareDemoButton(button);
  button.setIcon(&ic_outlined_24_action_done());
}

class ButtonRow : public FlexLayout {
 public:
  ButtonRow(ApplicationContext& context, const char* title,
            material3::ButtonVariant variant)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, title, font_caption()),
        button_(context, "Action", variant),
        button_with_icon_(context, "Action", variant),
        disabled_(context, "Disabled", variant) {
    setPadding(Padding(Scaled(12), Scaled(6)));
    setGap(Scaled(6));

    PrepareDemoButton(button_);
    PrepareIconDemoButton(button_with_icon_);
    PrepareDemoButton(disabled_);

    disabled_.setEnabled(false);

    button_.setOnInteractiveChange([this]() { handleClick(); });
    button_with_icon_.setOnInteractiveChange([this]() { handleClick(); });

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(button_, {.flex_grow = 0, .flex_shrink = 0});
    add(button_with_icon_, {.flex_grow = 0, .flex_shrink = 0});
    add(disabled_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  void handleClick() {
    counter_++;
    char buf[24];
    // Reuse the section caption as lightweight feedback so the example stays
    // self-contained and does not need extra status widgets.
    snprintf(buf, sizeof(buf), "clicked %d", counter_);
    title_.setText(buf);
  }

  TextLabel title_;
  material3::Button button_;
  material3::Button button_with_icon_;
  material3::Button disabled_;
  int counter_ = 0;
};

class SizeShowcase : public FlexLayout {
 public:
  explicit SizeShowcase(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Size tokens", font_caption()),
        extra_small_(context, "XS", material3::ButtonVariant::kFilled),
        small_(context, "Small", material3::ButtonVariant::kFilled),
        medium_(context, "Medium", material3::ButtonVariant::kFilled),
        large_(context, "Large", material3::ButtonVariant::kFilled),
        extra_large_(context, "XL", material3::ButtonVariant::kFilled) {
    setPadding(Padding(Scaled(12), Scaled(6)));
    setGap(Scaled(6));

    configure(extra_small_, material3::ButtonSize::kExtraSmall);
    configure(small_, material3::ButtonSize::kSmall);
    configure(medium_, material3::ButtonSize::kMedium);
    configure(large_, material3::ButtonSize::kLarge);
    configure(extra_large_, material3::ButtonSize::kExtraLarge);

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(extra_small_, {.flex_grow = 0, .flex_shrink = 0});
    add(small_, {.flex_grow = 0, .flex_shrink = 0});
    add(medium_, {.flex_grow = 0, .flex_shrink = 0});
    add(large_, {.flex_grow = 0, .flex_shrink = 0});
    add(extra_large_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  void configure(material3::Button& button, material3::ButtonSize size) {
    PrepareIconDemoButton(button);
    button.setSize(size);
  }

  TextLabel title_;
  material3::Button extra_small_;
  material3::Button small_;
  material3::Button medium_;
  material3::Button large_;
  material3::Button extra_large_;
};

class ShapeShowcase : public FlexLayout {
 public:
  explicit ShapeShowcase(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Shape tokens", font_caption()),
        round_(context, "Round", material3::ButtonVariant::kOutlined),
        square_(context, "Square", material3::ButtonVariant::kOutlined) {
    setPadding(Padding(Scaled(12), Scaled(6)));
    setGap(Scaled(6));

    PrepareIconDemoButton(round_);
    round_.setSize(material3::ButtonSize::kMedium);

    PrepareIconDemoButton(square_);
    square_.setSize(material3::ButtonSize::kMedium);
    square_.setShape(material3::ButtonShape::kSquare);

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(round_, {.flex_grow = 0, .flex_shrink = 0});
    add(square_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  material3::Button round_;
  material3::Button square_;
};

class SmallPaddingShowcase : public FlexLayout {
 public:
  explicit SmallPaddingShowcase(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Small-button padding", font_caption()),
        default_padding_(context, "24 dp",
                         material3::ButtonVariant::kFilledTonal),
        reduced_padding_(context, "16 dp",
                         material3::ButtonVariant::kFilledTonal) {
    setPadding(Padding(Scaled(12), Scaled(6)));
    setGap(Scaled(6));

    configure(default_padding_, material3::SmallButtonPadding::kDefault);
    configure(reduced_padding_, material3::SmallButtonPadding::kReduced);

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(default_padding_, {.flex_grow = 0, .flex_shrink = 0});
    add(reduced_padding_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  void configure(material3::Button& button,
                 material3::SmallButtonPadding padding) {
    PrepareIconDemoButton(button);
    // The padding selector only affects the small size, so pin the showcase to
    // that size to make the two configurations visually comparable.
    button.setSize(material3::ButtonSize::kSmall);
    button.setSmallButtonPadding(padding);
  }

  TextLabel title_;
  material3::Button default_padding_;
  material3::Button reduced_padding_;
};

class ButtonScreen : public ScrollablePanel {
 public:
  ButtonScreen(ApplicationContext& context)
      : ScrollablePanel(context),
        content_(context, FlexDirection::kColumn),
        title_(context, "Material 3 buttons", font_h6()),
        subtitle_(context, "Phase 1 - variants, sizes, shapes, and padding",
                  font_caption()),
        geometry_divider_(context),
        sizes_(context),
        shapes_(context),
        padding_(context),
        variant_divider_(context),
        elevated_(context, "Elevated", material3::ButtonVariant::kElevated),
        filled_(context, "Filled", material3::ButtonVariant::kFilled),
        tonal_(context, "Filled tonal", material3::ButtonVariant::kFilledTonal),
        outlined_(context, "Outlined", material3::ButtonVariant::kOutlined),
        text_(context, "Text", material3::ButtonVariant::kText) {
    content_.setPadding(Padding(Scaled(12), Scaled(8)));
    content_.setGap(Scaled(6));

    content_.add(title_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(geometry_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(sizes_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(shapes_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(padding_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(variant_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(elevated_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(filled_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(tonal_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(outlined_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(text_, {.flex_grow = 0, .flex_shrink = 0});

    setContents(content_);
  }

 private:
  FlexLayout content_;
  TextLabel title_;
  TextLabel subtitle_;
  HorizontalDivider geometry_divider_;
  SizeShowcase sizes_;
  ShapeShowcase shapes_;
  SmallPaddingShowcase padding_;
  HorizontalDivider variant_divider_;
  ButtonRow elevated_;
  ButtonRow filled_;
  ButtonRow tonal_;
  ButtonRow outlined_;
  ButtonRow text_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
ButtonScreen button_screen(app.context());
SingletonActivity activity(app, button_screen);

void setup() {
  initDisplay();
  app.start();

  // Never exits.
  scheduler.run();
}

void loop() {}
