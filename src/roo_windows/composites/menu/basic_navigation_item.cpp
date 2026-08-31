#include "roo_windows/composites/menu/basic_navigation_item.h"

namespace roo_windows {
namespace menu {

BasicNavigationItem::BasicNavigationItem(ApplicationContext& context,
                                         const roo_display::Pictogram& icon,
                                         roo::string_view text,
                                         NavigationHost& navigation,
                                         Destination& target)
    : FlexLayout(context, FlexDirection::kRow),
      icon_(context, icon),
      label_(context, text, material2::text_style_subtitle1()),
      navigation_(navigation),
      target_(target) {
  setAlignItems(AlignItems::kCenter);
  setPadding(Padding(PaddingSize::kSmall, PaddingSize::kTiny));
  setGap(Scaled(8));
  add(icon_, {.flex_grow = 0, .flex_shrink = 0});
  label_.setMargins(MarginSize::kNone);
  label_.setPadding(PaddingSize::kNone, PaddingSize::kTiny);
  add(label_, {.flex_grow = 1, .flex_shrink = 1});

  setOnInteractiveChange([&]() { navigation_.push(target_); });
}

BasicNavigationItemWithSubtext::BasicNavigationItemWithSubtext(
    ApplicationContext& context, const roo_display::Pictogram& icon,
    roo::string_view label, roo::string_view subtext,
    NavigationHost& navigation, Destination& target)
    : FlexLayout(context, FlexDirection::kRow),
      icon_(context, icon),
      content_(context, FlexDirection::kColumn),
      label_(context, label, material2::text_style_subtitle1()),
      subtext_(context, subtext, material2::text_style_subtitle2()),
      navigation_(navigation),
      target_(target) {
  setAlignItems(AlignItems::kCenter);
  setPadding(Padding(PaddingSize::kSmall, PaddingSize::kTiny));
  setGap(Scaled(8));
  add(icon_, {.flex_grow = 0, .flex_shrink = 0});
  label_.setMargins(MarginSize::kNone);
  label_.setPadding(PaddingSize::kNone);
  subtext_.setMargins(MarginSize::kNone);
  subtext_.setPadding(PaddingSize::kNone);
  content_.add(label_);
  content_.add(subtext_);
  add(content_, {.flex_grow = 1, .flex_shrink = 1});

  setOnInteractiveChange([&]() { navigation_.push(target_); });
}

}  // namespace menu
}  // namespace roo_windows
