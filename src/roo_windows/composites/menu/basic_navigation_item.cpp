#include "roo_windows/composites/menu/basic_navigation_item.h"

namespace roo_windows {
namespace menu {

BasicNavigationItem::BasicNavigationItem(const roo_windows::Environment& env,
                                         const roo_display::Pictogram& icon,
                                         roo_display::StringView text,
                                         roo_windows::Activity& target)
    : roo_windows::HorizontalLayout(env),
      icon_(env, icon),
      label_(env, text, roo_windows::font_subtitle1()),
      target_(target) {
  setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                  roo_windows::kVerticalGravityMiddle));
  add(icon_, HorizontalLayout::Params());
  label_.setMargins(roo_windows::MARGIN_NONE);
  label_.setPadding(roo_windows::PADDING_TINY);
  add(label_, HorizontalLayout::Params().setWeight(1));

  setOnInteractiveChange([&]() { getTask()->enterActivity(&target_); });
}

}  // namespace menu
}  // namespace roo_toolkit
