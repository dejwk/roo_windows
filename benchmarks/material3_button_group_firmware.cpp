// Equivalent mixed-content scenes for linked-section acceptance.
// ROO_GROUP_ACCEPTANCE_SCENE: 0 standalone, 1 standard, 2 connected.
#include <Arduino.h>
#include <SPI.h>

#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/containers/flex_layout.h"
#if ROO_GROUP_ACCEPTANCE_SCENE == 0
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/button/icon_button.h"
#else
#include "roo_windows/material3/button/button_group.h"
#endif
using namespace roo_windows;
using namespace roo_windows::material3;
roo_display::Ili9341spi<7, 2, 3> screen;
roo_display::Display display(screen);
roo_scheduler::SchedulingService scheduler;
Environment env(scheduler);
Application app(&env, display);
#if ROO_GROUP_ACCEPTANCE_SCENE == 0
using LabelEntry = roo_windows::material3::Button;
using IconEntry = roo_windows::material3::IconButton;
using Row = FlexLayout;
#elif ROO_GROUP_ACCEPTANCE_SCENE == 1
using LabelEntry = ButtonGroupButton;
using IconEntry = ButtonGroupIconButton;
using Row = ButtonGroup;
#else
using LabelEntry = ButtonGroupButton;
using IconEntry = ButtonGroupIconButton;
using Row = ConnectedButtonGroup;
#endif
class Scene : public FlexLayout {
 public:
  explicit Scene(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        capture_(context, "Capture"),
        save_(context, "Save"),
        history_(context, ic_outlined_24_action_home()
#if ROO_GROUP_ACCEPTANCE_SCENE == 0
                              ,
                 IconButtonStyle::kFilled
#endif
                 ),
#if ROO_GROUP_ACCEPTANCE_SCENE == 0
        row_(context, FlexDirection::kRow) {
#else
        row_(context) {
#endif
    row_.add(capture_);
    row_.add(save_);
    row_.add(history_);
#if ROO_GROUP_ACCEPTANCE_SCENE == 2
    row_.setOnInteractiveChange([this] { ++actions_; });
#else
    capture_.setOnInteractiveChange([this] { ++actions_; });
#endif
    add(row_);
  }

 private:
  LabelEntry capture_;
  LabelEntry save_;
  IconEntry history_;
  Row row_;
  volatile int actions_ = 0;
};

Scene scene(app.context());
void setup() {
  SPI.begin(4, 5, 6);
  display.init();
  app.addTaskFullScreen(scene);
  app.start();
  scheduler.run();
}

void loop() {}
