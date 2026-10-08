// Learning goal: select one operating mode through a button and replace the
// borrowed choice table when the available modes change.

// *************** EMULATOR AND PHYSICAL DISPLAY SETUP
// This shared runtime configures the same display, touch pins, and optional
// emulator keyboard for both paths. Adapt its pin mapping for another board.
#include "examples/material3/menus/example_runtime.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/material3/utilities/dropdown_button.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_windows;

namespace {

// Both pointer arrays and every string literal outlive the selector. Do not
// use a temporary initializer list for a borrowed choice table.
static constexpr const char* kModes[] = {"Off", "Automatic", "Scheduled"};
static constexpr const char* kServiceModes[] = {"Off", "Automatic", "Scheduled",
                                                "Maintenance"};

class OperatingModeScreen final : public FlexLayout {
 public:
  explicit OperatingModeScreen(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        caption_(context, "Operating mode",
                 material3::text_style_title_large()),
        selected_(context, "Automatic", material3::text_style_body_medium()),
        mode_(context, kModes, material3::ButtonVariant::kOutlined),
        service_(context, "Add maintenance",
                 material3::ButtonVariant::kFilledTonal) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(12));
    mode_.setSize(material3::ButtonSize::kSmall);
    mode_.setShape(material3::ButtonShape::kSquare);
    mode_.setDensityOverride(
        material3::DensityOverride::Explicit(material3::Density::kMinus2));
    mode_.setSelectedIndex(1);
    mode_.setOnInteractiveChange([this] {
      // The callback sees the new committed index after menu detachment.
      selected_.setText(mode_.selectedText());
    });
    service_.setOnInteractiveChange([this] {
      enableMaintenanceMode();
      service_.setEnabled(false);
    });
    add(caption_);
    add(mode_);
    add(selected_);
    add(service_);
  }

  // Call from application policy when maintenance mode becomes available.
  // The new table is static here; a runtime-built table must remain alive
  // until another successful replacement or this widget's destruction.
  void enableMaintenanceMode() { mode_.setItems(kServiceModes, 2); }

 private:
  TextLabel caption_;
  TextLabel selected_;
  material3::DropdownButton mode_;
  material3::Button service_;
};

}  // namespace

OperatingModeScreen screen(material3_menu_example::app.context());
Task& task = material3_menu_example::app.addTaskFullScreen(screen);

void setup() { material3_menu_example::Start(); }

void loop() {}
