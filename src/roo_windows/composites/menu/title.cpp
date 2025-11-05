#include "roo_windows/composites/menu/title.h"

#include "roo_icons/outlined/navigation.h"
#include "roo_windows/core/task.h"

namespace roo_windows {
namespace menu {

Title::Title(const roo_windows::Environment& env, std::string title)
    : HorizontalLayout(env),
      back_(env, SCALED_ROO_ICON(outlined, navigation_arrow_back)),
      label_(env, std::move(title), roo_windows::font_h6(),
             roo_display::kLeft | roo_display::kMiddle) {
  setGravity(roo_windows::kGravityMiddle);
  label_.setMargins(roo_windows::MARGIN_NONE);
  label_.setPadding(roo_windows::PADDING_TINY);
  add(back_);
  add(label_, { weight : 1 });
  back_.setOnInteractiveChange([&]() { getTask()->exitActivity(); });
}

}  // namespace menu
}  // namespace roo_windows
