#include "roo_windows/composites/menu/title.h"

#include "roo_icons/outlined/navigation.h"
#include "roo_windows/core/task.h"

namespace roo_windows {
namespace menu {

Title::Title(ApplicationContext& context, std::string title)
    : FlexLayout(context, FlexDirection::kRow),
      back_(context, SCALED_ROO_ICON(outlined, navigation_arrow_back)),
      label_(context, std::move(title), material2::text_style_h6(),
             kGravityLeft | kGravityMiddle) {
  setAlignItems(AlignItems::kCenter);
  setPadding(Padding(PaddingSize::kSmall, PaddingSize::kTiny));
  setGap(Scaled(8));
  label_.setMargins(MarginSize::kNone);
  label_.setPadding(PaddingSize::kNone, PaddingSize::kTiny);
  add(back_, {.flex_grow = 0, .flex_shrink = 0});
  add(label_, {.flex_grow = 1, .flex_shrink = 1});
  back_.setOnInteractiveChange(
      [&]() { getTask()->requestBack(BackSource::kNavigationButton); });
}

}  // namespace menu
}  // namespace roo_windows
