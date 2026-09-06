// Single-select menus commit one mode and dismiss after invocation.

#include "examples/material3/menus/example_runtime.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_windows;

namespace {

class ModeItem final : public material3::StandardMenuItem {
 public:
  ModeItem(roo::string_view label, TextLabel& status, bool selected)
      : StandardMenuItem(material3::StandardMenuItemInit{
            label, {}, nullptr,
            material3::StandardMenuItemFlags::kSelectable |
                (selected ? material3::StandardMenuItemFlags::kSelected
                          : material3::StandardMenuItemFlags::kDefault)}),
        label_(label),
        status_(status) {}
  // Selection is updated by Menu before this semantic action is delivered.
  void onInvoked() override { status_.setText(label_); }

 private:
  roo::string_view label_;
  TextLabel& status_;
};

class MenuButton final : public material3::Button {
 public:
  MenuButton(ApplicationContext& context, material3::Menu& menu)
      : Button(context, "Choose operating mode",
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

class OperatingModeCatalog final : public FlexLayout {
 public:
  explicit OperatingModeCatalog(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Operating mode", material3::text_style_title_large()),
        status_(context, "Automatic", material3::text_style_body_large()),
        automatic_("Automatic", status_, true),
        service_("Service", status_, false),
        automatic_row_(context),
        service_row_(context),
        group_(context),
        menu_(context),
        open_(context, menu_) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(12));
    automatic_row_.setMenuItem(automatic_);
    service_row_.setMenuItem(service_);
    group_.add(automatic_row_);
    group_.add(service_row_);
    // Single selection deselects the other mode and dismisses after invoking
    // the chosen item. The item itself remains the source of selected state.
    material3::MenuPolicy policy;
    policy.selection_mode = material3::SelectionMode::kSingle;
    menu_.setPolicy(policy);
    menu_.addGroup(group_);
    add(title_);
    add(open_);
    add(status_);
  }
  void bind(Task& owner) { open_.bind(owner); }

 private:
  TextLabel title_;
  TextLabel status_;
  ModeItem automatic_;
  ModeItem service_;
  material3::MenuEntry automatic_row_;
  material3::MenuEntry service_row_;
  material3::MenuGroup group_;
  material3::Menu menu_;
  MenuButton open_;
};

}  // namespace

OperatingModeCatalog catalog(material3_menu_example::app.context());
Task& task = material3_menu_example::app.addTaskFullScreen(catalog);

void setup() {
  catalog.bind(task);
  material3_menu_example::Start();
}

void loop() {}
