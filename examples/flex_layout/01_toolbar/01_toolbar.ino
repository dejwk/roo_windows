// Example 1: Toolbar
//
// A horizontal row at the bottom of the screen with a navigation back-button
// on the left, a centered title label that stretches to fill the available
// space, and a row of three icon action-buttons on the right — all aligned to
// the center of the cross axis.
//
// Demonstrates:
//   FlexDirection::kRow
//   JustifyContent::kFlexStart  (items from left)
//   AlignItems::kCenter         (cross-axis centering)
//   flex-grow on the spacer label so it absorbs all free space

#include "roo_icons/filled/action/24/search.h"
#include "roo_icons/filled/action/24/settings.h"
#include "roo_icons/filled/navigation/24/arrow_back.h"
#include "roo_icons/filled/navigation/24/more_vert.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_windows;

// ─── Toolbar view ────────────────────────────────────────────────────────────

class Toolbar : public FlexLayout {
 public:
  Toolbar(const Environment& env)
      : FlexLayout(env, FlexDirection::kRow),
        back_(env, ic_filled_navigation_24_arrow_back()),
        title_(env, "My screen", font_body1()),
        search_(env, ic_filled_action_24_search()),
        settings_(env, ic_filled_action_24_settings()),
        overflow_(env, ic_filled_navigation_24_more_vert()) {
    setAlignItems(AlignItems::kCenter);
    setPadding(Padding(PaddingSize::SMALL, PaddingSize::NONE));
    setGap(Scaled(4));

    // Back button: fixed size, no grow/shrink.
    add(WidgetRef(back_), {.flex_grow = 0, .flex_shrink = 0});

    // Title: grows to fill the remaining space.
    add(WidgetRef(title_), {.flex_grow = 1, .flex_shrink = 1});

    // Action icons: fixed size.
    add(WidgetRef(search_), {.flex_grow = 0, .flex_shrink = 0});
    add(WidgetRef(settings_), {.flex_grow = 0, .flex_shrink = 0});
    add(WidgetRef(overflow_), {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  Icon back_;
  TextLabel title_;
  Icon search_;
  Icon settings_;
  Icon overflow_;
};

// ─── Boilerplate ─────────────────────────────────────────────────────────────

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);

// Replace with your display initialisation.
// roo_display::Display display(...);
// Application app(&env, display);

// Task* task;
// Toolbar toolbar(env);

// void setup() {
//   task = app.addTaskFullScreen();
//   task->enter(SingletonActivity(app, toolbar));
//   app.start();
// }
//
// void loop() {
//   app.run();
// }
