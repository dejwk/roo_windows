// Opens an anchored overflow menu and retains the trigger's pressed paint.

#include "examples/material3/menus/example_runtime.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_windows;

namespace {

class OpenMenuButton final : public material3::Button {
 public:
  OpenMenuButton(ApplicationContext& context, material3::Menu& menu)
      : Button(context, "Equipment actions",
               material3::ButtonVariant::kFilledTonal),
        menu_(menu) {}

  void bind(Task& owner) { owner_ = &owner; }

  void onClicked() override {
    if (owner_ == nullptr) return;
    material3::MenuTriggerPaintSource trigger{*this, Scaled(20), 0xFF000000,
                                               20};
    menu_.show(*owner_, *this, material3::MenuPlacement::kBelowEnd, &trigger);
  }

 private:
  material3::Menu& menu_;
  Task* owner_ = nullptr;
};

class EquipmentActions final : public FlexLayout {
 public:
  explicit EquipmentActions(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Pump controller", material3::text_style_title_large()),
        inspect_(material3::StandardMenuItemInit{"Inspect", {}}),
        restart_(material3::StandardMenuItemInit{"Restart", {}}),
        inspect_row_(context),
        restart_row_(context),
        group_(context),
        menu_(context),
        open_(context, menu_) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(12));
    inspect_row_.setMenuItem(inspect_);
    restart_row_.setMenuItem(restart_);
    group_.add(inspect_row_);
    group_.add(restart_row_);
    menu_.addGroup(group_);
    add(title_);
    add(open_);
  }

  void bind(Task& owner) { open_.bind(owner); }

 private:
  TextLabel title_;
  material3::StandardMenuItem inspect_;
  material3::StandardMenuItem restart_;
  material3::MenuEntry inspect_row_;
  material3::MenuEntry restart_row_;
  material3::MenuGroup group_;
  material3::Menu menu_;
  OpenMenuButton open_;
};

}  // namespace

EquipmentActions catalog(material3_menu_example::app.context());
Task& task = material3_menu_example::app.addTaskFullScreen(catalog);

void setup() {
  catalog.bind(task);
  material3_menu_example::Start();
}

void loop() {}
