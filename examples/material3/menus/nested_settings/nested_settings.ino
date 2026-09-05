// Submenus cascade on wide displays and replace their parent on compact ones.

#include "examples/material3/menus/example_runtime.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_windows;

namespace {

class SettingsSubmenu final : public material3::StandardMenuItem {
 public:
  explicit SettingsSubmenu(ApplicationContext& context)
      : StandardMenuItem(material3::StandardMenuItemInit{"Display", {}}),
        context_(context) {}

  bool hasSubmenu() const override { return true; }

  void populateSubmenu(material3::MenuLevelBuilder& builder) override {
    // Child rows are presentation-scoped. The builder adopts them
    // synchronously and releases the level when it closes or is replaced.
    auto group = std::make_unique<material3::MenuGroup>(context_);
    material3::StandardMenuItemInit brightness;
    brightness.headline = "Brightness";
    material3::StandardMenuItemInit theme;
    theme.headline = "Color theme";
    group->add(
        std::make_unique<material3::MenuRow<material3::StandardMenuItem>>(
            context_, brightness));
    group->add(
        std::make_unique<material3::MenuRow<material3::StandardMenuItem>>(
            context_, theme));
    builder.addGroup(std::move(group));
  }

 private:
  ApplicationContext& context_;
};

class SettingsButton final : public material3::Button {
 public:
  SettingsButton(ApplicationContext& context, material3::Menu& menu)
      : Button(context, "Settings", material3::ButtonVariant::kFilledTonal),
        menu_(menu) {}
  void bind(Task& owner) { owner_ = &owner; }
  void onClicked() override {
    if (owner_ != nullptr) menu_.show(*owner_, *this);
  }

 private:
  material3::Menu& menu_;
  Task* owner_ = nullptr;
};

class NestedSettingsCatalog final : public FlexLayout {
 public:
  explicit NestedSettingsCatalog(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Nested settings", material3::text_style_title_large()),
        display_(context),
        connectivity_(material3::StandardMenuItemInit{"Connectivity", {}}),
        display_row_(context),
        connectivity_row_(context),
        group_(context),
        menu_(context),
        open_(context, menu_) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(12));
    display_row_.setMenuItem(display_);
    connectivity_row_.setMenuItem(connectivity_);
    // A submenu item appears beside its sibling like any other MenuItem; its
    // hasSubmenu()/populateSubmenu() hooks add the forward indicator and level.
    group_.add(display_row_);
    group_.add(connectivity_row_);
    menu_.addGroup(group_);
    add(title_);
    add(open_);
  }
  void bind(Task& owner) { open_.bind(owner); }

 private:
  TextLabel title_;
  SettingsSubmenu display_;
  material3::StandardMenuItem connectivity_;
  material3::MenuEntry display_row_;
  material3::MenuEntry connectivity_row_;
  material3::MenuGroup group_;
  material3::Menu menu_;
  SettingsButton open_;
};

}  // namespace

NestedSettingsCatalog catalog(material3_menu_example::app.context());
Task& task = material3_menu_example::app.addTaskFullScreen(catalog);

void setup() {
  catalog.bind(task);
  material3_menu_example::Start();
}

void loop() {}
