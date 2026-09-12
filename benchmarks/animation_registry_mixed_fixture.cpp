// Link fixture used by the animation-registry target acceptance comparison.
//
// The function deliberately constructs and exercises every implemented
// migration consumer. Target builds call it from the same settings-shell
// application before measuring linked text and read-only data.
#include <memory>

#include "roo_icons/outlined/24/action.h"
#include "roo_windows/containers/horizontal_page_host.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/button/toggle_icon_button.h"
#include "roo_windows/material3/list/list.h"
#include "roo_windows/material3/snackbar/snackbar.h"
#include "roo_windows/material3/switch/switch.h"
#include "roo_windows/material3/tabs/tabs.h"
#include "roo_windows/widgets/progress_bar.h"
#include "roo_windows/widgets/switch.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows::test {

[[gnu::noinline]] int ExerciseAnimationConsumerLinkFixture(
    ApplicationContext& context) {
  material3::ExpandablePanel expandable(context);
  expandable.setExpanded(true);

  HorizontalPageHost pages(context);
  pages.setCurrentIndex(0);

  material3::Tabs tabs(context);
  tabs.addTab(std::make_unique<material3::Tab>(context, "Fixed one"));
  tabs.addTab(std::make_unique<material3::Tab>(context, "Fixed two"));
  tabs.setSelectedIndex(1);

  material3::ScrollableTabs scrollable_tabs(context);
  scrollable_tabs.addTab(
      std::make_unique<material3::Tab>(context, "Scrollable one"));
  scrollable_tabs.addTab(
      std::make_unique<material3::Tab>(context, "Scrollable two"));
  scrollable_tabs.setSelectedIndex(1);

  SimpleScrollablePanel scroll_panel(context);
  scroll_panel.scrollTo(1, 1);

  material3::Switch material_switch(context);
  material_switch.setOn();
  Switch legacy_switch(context);
  legacy_switch.setOn();

  material3::ToggleIconButton toggle(context, ic_outlined_24_action_done());
  toggle.setSelected(true);

  ProgressBar progress(context);
  progress.setIndeterminate();

  TextField text_field(context, font_body1(), "Value",
                       roo_display::kLeft | roo_display::kMiddle,
                       TextField::UNDERLINE);
  text_field.setContent("linked");

  material3::SnackbarHost snackbar(context);
  material3::SnackbarRequest request;
  request.configure("Linked animation consumer fixture");
  snackbar.snackbars().show(request);

  return tabs.selectedIndex() + scrollable_tabs.selectedIndex() +
         pages.currentIndex() + material_switch.isOn() + legacy_switch.isOn() +
         toggle.isSelected() + progress.isIndeterminate() +
         snackbar.snackbars().isShowing();
}

}  // namespace roo_windows::test
