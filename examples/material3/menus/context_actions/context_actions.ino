// Opens a context menu from a copied point near the display edge.

#include "examples/material3/menus/example_runtime.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_windows;

namespace {

class ContextButton final : public material3::Button {
 public:
  ContextButton(ApplicationContext& context, material3::Menu& menu)
      : Button(context, "Open at lower-right",
               material3::ButtonVariant::kOutlined),
        menu_(menu) {}

  void bind(Task& owner) { owner_ = &owner; }

  void onClicked() override {
    if (owner_ != nullptr) {
      // Context input can provide window coordinates directly. Edge-aware
      // placement constrains this lower-right point to the visible viewport.
      menu_.showFromRect(*owner_, Rect(300, 220, 300, 220),
                         material3::MenuPlacement::kBelowEnd);
    }
  }

 private:
  material3::Menu& menu_;
  Task* owner_ = nullptr;
};

class ContextActions final : public FlexLayout {
 public:
  explicit ContextActions(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Context-point placement",
               material3::text_style_title_large()),
        copy_(material3::StandardMenuItemInit{"Copy", {}}),
        details_(material3::StandardMenuItemInit{"Details", {}}),
        copy_row_(context),
        details_row_(context),
        group_(context),
        menu_(context),
        open_(context, menu_) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(12));
    // Persistent item/row/group objects make repeated opening allocation-light
    // and keep all non-owning bindings valid for the menu's lifetime.
    copy_row_.setMenuItem(copy_);
    details_row_.setMenuItem(details_);
    group_.add(copy_row_);
    group_.add(details_row_);
    menu_.addGroup(group_);
    add(title_);
    add(open_);
  }

  void bind(Task& owner) { open_.bind(owner); }

 private:
  TextLabel title_;
  material3::StandardMenuItem copy_;
  material3::StandardMenuItem details_;
  material3::MenuEntry copy_row_;
  material3::MenuEntry details_row_;
  material3::MenuGroup group_;
  material3::Menu menu_;
  ContextButton open_;
};

}  // namespace

ContextActions catalog(material3_menu_example::app.context());
Task& task = material3_menu_example::app.addTaskFullScreen(catalog);

void setup() {
  catalog.bind(task);
  material3_menu_example::Start();
}

void loop() {}
