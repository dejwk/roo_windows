#include "roo_windows/core/task.h"

#include "roo_logging.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/display_window.h"
#include "roo_windows/core/transient_surface_host.h"

namespace roo_windows {
namespace {
bool IsInSubtree(const Widget& candidate, const Widget& subtree) {
  for (const Widget* current = &candidate; current != nullptr;
       current = current->parent()) {
    if (current == &subtree) return true;
  }
  return false;
}
}  // namespace

Task::Task(Application& app, DisplayWindow& window,
           const roo_display::Box& bounds, bool popup, Widget& content)
    : app_(app),
      window_(window),
      panel_(app.context(), *this),
      focus_(&panel_),
      editor_(app, app.env().scheduler()),
      popup_(popup),
      presentation_available_(false) {
  CHECK(content.parent() == nullptr);
  panel_.setContent(content, roo_display::Box(0, 0, -1, -1));
  if (popup) {
    window_.root().addPopup(panel_, bounds);
  } else {
    window_.root().addTask(panel_, bounds);
  }
  presentation_available_ = true;
}

Task::Task(Application& app, DisplayWindow& window,
           const roo_display::Box& bounds, bool popup,
           NavigationHost& navigation)
    : app_(app),
      window_(window),
      panel_(app.context(), *this),
      focus_(&panel_),
      editor_(app, app.env().scheduler()),
      popup_(popup),
      presentation_available_(false),
      navigation_(&navigation) {
  navigation.install(*this);
  if (popup) {
    window_.root().addPopup(panel_, bounds);
  } else {
    window_.root().addTask(panel_, bounds);
  }
  presentation_available_ = true;
}

Task::~Task() {
  // Reentrant completion must observe an unavailable owner before any task
  // service or structural state starts to disappear.
  presentation_available_ = false;
  internal::GetTransientSurfaceHost(*this).interactionOwnerUnavailable(*this);
  back_callback_ = {};
  editor_.cancel();
  focus_.onSubtreeDetaching(panel_);
  if (navigation_ != nullptr) {
    navigation_->disconnect();
  } else if (panel_.content_ != nullptr) {
    panel_.clearContent();
  }
  if (popup_) {
    window_.root().removePopup(panel_);
  } else {
    window_.root().removeTask(panel_);
  }
}

void Task::setBackCallback(BackCallback callback) {
  back_callback_ = std::move(callback);
}

void Task::setVisible(bool visible) {
  panel_.setVisibility(visible ? Visibility::kVisible : Visibility::kGone);
}

BackResult Task::requestBack(BackSource source) {
  if (window_.root().transient_presentation_slot().requestBack(source) ==
      BackResult::kHandled) {
    return BackResult::kHandled;
  }
  if (navigation_ != nullptr) return navigation_->requestBack(source);
  return requestTaskBackCallback(source);
}

void Task::attachNavigationContent(Widget& content) {
  CHECK(navigation_ != nullptr);
  panel_.setContent(content, roo_display::Box(0, 0, -1, -1));
}

void Task::detachNavigationContent() {
  CHECK(navigation_ != nullptr);
  // A transient belongs to the task, but its anchor belongs to the outgoing
  // destination. End it before detaching that content, blocking re-admission
  // from its completion callback throughout the structural change.
  const bool available = presentation_available_;
  presentation_available_ = false;
  internal::GetTransientSurfaceHost(*this).interactionOwnerUnavailable(*this);
  panel_.clearContent();
  presentation_available_ = available;
}

BackResult Task::requestTaskBackCallback(BackSource source) {
  return back_callback_ == nullptr ? BackResult::kUnhandled
                                   : back_callback_(source);
}

void Task::dispatchKeyEvent(const KeyEvent& event) {
  internal::TransientSurfaceHost& host = window_.root().transient_surface_host_;
  bool hosted = host.isActive();
  if (hosted && !host.isInputEnabled()) return;
  bool is_back_down =
      event.phase == KeyPhase::kDown &&
      (event.code == KeyCode::kBack || event.code == KeyCode::kEscape);
  if (hosted && is_back_down) {
    TransientPresentationRegistration* registration = host.activeRegistration();
    BackSource source = event.code == KeyCode::kBack ? BackSource::kBackKey
                                                     : BackSource::kEscapeKey;
    if (window_.root().transient_presentation_slot().requestBack(source) ==
        BackResult::kHandled) {
      return;
    }
    // A declining hook may have finished or destroyed its presenter. The
    // event was still covered at dispatch entry and must not leak below it.
    if (host.activeRegistration() != registration) return;
  }
  if (hosted && !host.isInteractionOwner(*this)) return;

  Widget* scope_root = focus_.scopeRoot();
  bool presenter_scope_active = scope_root != &panel_;
  Widget* focused = focus_.focused();
  // An explicit presenter scope is a complete key boundary even when it has no
  // focused descendant. Only the implicit task scope retains the legacy
  // Application::add() context-focus fallback.
  if (focused == nullptr && !presenter_scope_active) {
    focused = app_.context().focus().focused();
  }
  if (focused != nullptr) {
    if (focused->onKeyEvent(event)) return;
    for (Widget* ancestor = focused->parent(); ancestor != nullptr;
         ancestor = ancestor->parent()) {
      // Ordinary task routing stops before TaskPanel. Presenter routing
      // includes its explicit root, then stops before the structural host.
      if (!presenter_scope_active && ancestor == scope_root) break;
      if (ancestor->onKeyEvent(event)) return;
      if (ancestor == scope_root) break;
    }
  }
  if (event.phase == KeyPhase::kDown &&
      (event.code == KeyCode::kBack || event.code == KeyCode::kEscape)) {
    // Hosted Back was already offered before focused-widget routing. Do not
    // offer it to the registration twice or continue into task-local state.
    if (hosted) return;
    BackSource source = event.code == KeyCode::kBack ? BackSource::kBackKey
                                                     : BackSource::kEscapeKey;
    if (requestBack(source) == BackResult::kHandled) return;
  }
  if ((event.phase == KeyPhase::kDown || event.phase == KeyPhase::kRepeat) &&
      event.code == KeyCode::kTab) {
    focus_.moveFocus(*scope_root, (event.modifiers & kKeyModifierShift) != 0);
    return;
  }
  if (event.phase == KeyPhase::kDown || event.phase == KeyPhase::kRepeat) {
    FocusDirection direction;
    bool is_directional = true;
    switch (event.code) {
      case KeyCode::kUp:
        direction = FocusDirection::kUp;
        break;
      case KeyCode::kDown:
        direction = FocusDirection::kDown;
        break;
      case KeyCode::kLeft:
        direction = FocusDirection::kLeft;
        break;
      case KeyCode::kRight:
        direction = FocusDirection::kRight;
        break;
      default:
        is_directional = false;
        break;
    }
    if (is_directional && focus_.moveFocusDirection(*scope_root, direction)) {
      return;
    }
  }
  if (event.code != KeyCode::kEnter && event.code != KeyCode::kSpace) return;
  if (focused == nullptr) return;
  if (event.phase == KeyPhase::kDown && focused->isClickable() &&
      focused->isEnabled()) {
    if (armed_key_widget_ != nullptr) onWidgetFocusLost(*armed_key_widget_);
    armed_key_widget_ = focused;
    armed_key_ = event.physical_key;
    focused->Widget::onShowPress(focused->width() / 2, focused->height() / 2);
  } else if (event.phase == KeyPhase::kUp && armed_key_widget_ == focused &&
             armed_key_ == event.physical_key) {
    armed_key_widget_ = nullptr;
    armed_key_ = PhysicalKey::kNone;
    focused->onSingleTapUp(focused->width() / 2, focused->height() / 2);
  }
}

void Task::onWidgetFocusLost(Widget& widget) {
  if (armed_key_widget_ != &widget) return;
  armed_key_widget_->setPressed(false);
  armed_key_widget_ = nullptr;
  armed_key_ = PhysicalKey::kNone;
}

void Task::cancelKeyActivation() {
  // The router calls this before unlinking a source so a pressed fallback
  // control cannot remain visually armed after its producing switch vanishes.
  if (armed_key_widget_ != nullptr) onWidgetFocusLost(*armed_key_widget_);
}

void Task::onSubtreeDetaching(Widget& subtree) {
  if (armed_key_widget_ != nullptr &&
      IsInSubtree(*armed_key_widget_, subtree)) {
    onWidgetFocusLost(*armed_key_widget_);
  }
  if (editor_.targetInSubtree(subtree)) editor_.cancel();
  focus_.onSubtreeDetaching(subtree);
}

}  // namespace roo_windows
