// Multi-select menu leaves the chain open while filters are toggled.

#include "examples/material3/menus/example_runtime.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_windows;

namespace {

class FilterButton final : public material3::Button {
 public:
  FilterButton(ApplicationContext& context, material3::Menu& menu)
      : Button(context, "Alert filters",
               material3::ButtonVariant::kFilledTonal),
        menu_(menu) {}
  void bind(Task& owner) { owner_ = &owner; }
  void onClicked() override {
    if (owner_ != nullptr) menu_.show(*owner_, *this);
  }

 private:
  material3::Menu& menu_;
  Task* owner_ = nullptr;
};

class AlertFiltersCatalog final : public FlexLayout {
 public:
  explicit AlertFiltersCatalog(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Visible alerts", material3::text_style_title_large()),
        warnings_(material3::StandardMenuItemInit{"Warnings", {}, nullptr,
                                                   true, true, true}),
        maintenance_(material3::StandardMenuItemInit{
            "Maintenance", {}, nullptr, true, true, false}),
        warnings_row_(context),
        maintenance_row_(context),
        group_(context),
        menu_(context),
        open_(context, menu_) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(12));
    warnings_row_.setMenuItem(warnings_);
    maintenance_row_.setMenuItem(maintenance_);
    group_.add(warnings_row_);
    group_.add(maintenance_row_);
    material3::MenuPolicy policy;
    policy.selection_mode = material3::SelectionMode::kMultiple;
    menu_.setPolicy(policy);
    menu_.addGroup(group_);
    add(title_);
    add(open_);
  }
  void bind(Task& owner) { open_.bind(owner); }

 private:
  TextLabel title_;
  material3::StandardMenuItem warnings_;
  material3::StandardMenuItem maintenance_;
  material3::MenuEntry warnings_row_;
  material3::MenuEntry maintenance_row_;
  material3::MenuGroup group_;
  material3::Menu menu_;
  FilterButton open_;
};

}  // namespace

AlertFiltersCatalog catalog(material3_menu_example::app.context());
Task& task = material3_menu_example::app.addTaskFullScreen(catalog);

void setup() {
  catalog.bind(task);
  material3_menu_example::Start();
}

void loop() {}
