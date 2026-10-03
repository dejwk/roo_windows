#include "roo_windows/keyboard/editor_destination.h"

#include <algorithm>
#include <utility>

#include "roo_icons.h"
#include "roo_icons/outlined/navigation.h"
#include "roo_windows/config.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/text_field/text_field.h"

namespace roo_windows {

EditorDestination::EditedTextField::EditedTextField(
    ApplicationContext& context, const std::string& hint,
    EditorDestination& destination)
    : TextField(context, font_body1(), hint,
                roo_display::kLeft | roo_display::kMiddle, UNDERLINE),
      destination_(destination) {}

void EditorDestination::EditedTextField::onEditFinished(bool confirmed) {
  if (confirmed) destination_.confirm();
}

EditorDestination::EditorDestination(ApplicationContext& context,
                                     const std::string& hint)
    : main_pane_(context),
      content_pane_(context),
      back_(context, SCALED_ROO_ICON(outlined, navigation_arrow_back),
            Button::TEXT),
      text_(context, hint, *this),
      enter_(context, SCALED_ROO_ICON(outlined, navigation_check)),
      editing_(false),
      confirmed_(false),
      completing_(false) {
  main_pane_.add(content_pane_, VerticalLayout::Params());
  content_pane_.add(back_, {gravity : kGravityMiddle});
  content_pane_.add(text_, {gravity : kGravityMiddle, weight : 1});
  content_pane_.add(enter_, {gravity : kGravityMiddle});
  back_.setContentColor(
      context.theme().framework.color.resolve(FrameworkColorRole::kContent));
  back_.setOnInteractiveChange(
      [this]() { getTask()->requestBack(BackSource::kNavigationButton); });
  enter_.setOnInteractiveChange([this]() { confirm(); });
}

BackResult EditorDestination::onBackRequested(BackSource source) {
  (void)source;
  if (!editing_) return BackResult::kUnhandled;
  exit();
  return BackResult::kHandled;
}

void EditorDestination::onResume() {
  if (editing_) text_.edit();
}

void EditorDestination::confirm() {
  if (!editing_) return;
  confirmed_ = true;
  exit();
}

void EditorDestination::onRemoved() {
  editing_ = false;
  completing_ = true;
  bool confirmed = confirmed_;
  confirmed_ = false;
  std::function<void(const std::string&)> callback = std::move(enter_fn_);
  std::string value = text_.content();
  text_.setContent("");
  if (source_ != nullptr) {
    if (confirmed) source_->setText(std::move(value));
    // A value-change callback can destroy the source; its destructor clears
    // source_ before we deliver the second notification.
    material3::TextField* source = source_;
    source_ = nullptr;
    if (source != nullptr) source->onEditFinished(confirmed);
  }
  completing_ = false;
  if (confirmed && callback != nullptr) callback(value);
}

void EditorDestination::triggerEdit(
    NavigationHost& navigation, const std::string& initial,
    const std::string& hint, std::function<void(const std::string&)> enter_fn,
    bool obscure) {
  CHECK(getNavigationHost() == nullptr);
  CHECK(!completing_);
  editing_ = true;
  confirmed_ = false;
  text_.setContent(initial);
  text_.setHint(hint);
  text_.setStarred(obscure);
  enter_fn_ = std::move(enter_fn);
  navigation.push(*this);
  if (getNavigationHost() == nullptr) {
    editing_ = false;
    source_ = nullptr;
    enter_fn_ = nullptr;
  }
}

void EditorDestination::triggerEditField(TextField& field) {
  Task* task = field.getTask();
  CHECK(task != nullptr);
  triggerEdit(
      task->navigation(), field.content(), field.hint(),
      [&field](const std::string& value) { field.setContent(value); },
      field.obscureText());
}

bool EditorDestination::NeedsExtraction(material3::TextField& field) {
  Task* task = field.getTask();
  if (task == nullptr || !task->navigation().isAvailable()) return false;
  const Widget* ancestor = &field;
  while (ancestor != nullptr && ancestor != &task->panel_) {
    ancestor = ancestor->parent();
  }
  if (ancestor == nullptr) return false;
  const Widget* keyboard_panel =
      task->application().keyboard().getContents().parent();
  if (keyboard_panel == nullptr) return false;
  int available_height = std::max(
      0, std::min<int>(task->panel_.height(),
                       keyboard_panel->offsetTop() - task->panel_.offsetTop()));
  Dimensions minimum = field.getSuggestedMinimumDimensions();
  return std::max<int>(field.height(), minimum.height()) > available_height ||
         field.width() > task->panel_.width();
}

bool EditorDestination::triggerEditField(material3::TextField& field) {
  if (getNavigationHost() != nullptr || completing_) return false;
  Task* task = field.getTask();
  bool obscure = field.obscureText();
  source_ = &field;
  triggerEdit(task->navigation(), field.text(), std::string(field.label()),
              nullptr, obscure);
  return true;
}

void EditorDestination::forgetSource(material3::TextField& field) {
  if (source_ == &field) source_ = nullptr;
}

}  // namespace roo_windows
