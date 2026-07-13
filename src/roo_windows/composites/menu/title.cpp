#include "roo_windows/composites/menu/title.h"

#include "roo_icons/outlined/navigation.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/task.h"

namespace roo_windows {
namespace menu {

Title::Title(ApplicationContext& context, std::string title)
    : HorizontalLayout(context),
      back_(context, SCALED_ROO_ICON(outlined, navigation_arrow_back)),
      label_(context, std::move(title), font_h6(), kGravityLeft | kGravityMiddle) {
  setGravity(kGravityMiddle);
  label_.setMargins(MarginSize::kNone);
  label_.setPadding(PaddingSize::kTiny);
  add(back_);
  add(label_, {weight : 1});
  back_.setOnInteractiveChange([&]() {
    getTask()->getApplication().requestBack(*getTask(),
                                            BackSource::kNavigationButton);
  });
}

}  // namespace menu
}  // namespace roo_windows
