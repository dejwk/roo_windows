#include "roo_windows/core/task_panel.h"

#include "roo_logging.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/task.h"

namespace roo_windows {
TaskPanel::~TaskPanel() {
  if (content_ != nullptr) clearContent();
}

void TaskPanel::setContent(Widget& content, const roo_display::Box& bounds) {
  CHECK(content_ == nullptr);
  CHECK(content.parent() == nullptr);
  content_ = &content;
  attachChild(WidgetRef(content), bounds);
}

void TaskPanel::clearContent() {
  CHECK(content_ != nullptr);
  Widget* content = content_;
  content_ = nullptr;
  detachChild(content);
}

bool TaskPanel::fillTouchTargetPath(XDim x, YDim y,
                                    std::vector<Widget*>& path) {
  if (!isVisible() || !isEnabled() || !bounds().contains(x, y)) return false;
  path.push_back(this);
  if (content_ != nullptr) {
    content_->fillTouchTargetPath(x - content_->offsetLeft(),
                                  y - content_->offsetTop(), path);
  }
  return true;
}

namespace {
// Only scroll ancestors of the editor participate in resize avoidance.
bool HasScrollAncestor(const Widget* target, const Widget& boundary) {
  if (target == nullptr) return false;
  for (const Widget* current = target->parent();
       current != nullptr && current != &boundary;
       current = current->parent()) {
    if ((current->dragAxis() == DragAxis::kVertical ||
         current->dragAxis() == DragAxis::kBoth))
      return true;
  }
  return false;
}

// Projects a descendant through all intervening layout and scroll offsets.
Rect DescendantBounds(const Widget& target, const Widget& ancestor) {
  Rect rect = target.parent_bounds();
  for (const Widget* current = target.parent();
       current != nullptr && current != &ancestor;
       current = current->parent()) {
    rect = rect.translate(current->offsetLeft(), current->offsetTop());
  }
  return rect;
}
}  // namespace

Dimensions TaskPanel::onMeasure(WidthSpec width, HeightSpec height) {
  if (content_ != nullptr) {
    Rect viewport = task_.application().textEditorViewport(task_);
    Widget* target = task_.textFieldEditor().editedWidget();
    // Reducing the viewport extends the normal scroll range, including when
    // the contents previously fit the screen without any scrolling.
    if (!viewport.empty() && HasScrollAncestor(target, *this)) {
      height = HeightSpec::Exactly(viewport.height());
    }
    content_->measure(width, height);
  }
  return Dimensions(this->width(), this->height());
}

void TaskPanel::onLayout(bool changed, const Rect& rect) {
  if (content_ == nullptr) return;
  Rect viewport = task_.application().textEditorViewport(task_);
  Widget* target = task_.textFieldEditor().editedWidget();
  bool scrollable = HasScrollAncestor(target, *this);
  Rect content_bounds = !viewport.empty() && scrollable ? viewport : bounds();
  content_->layout(content_bounds);
  // Layout can end a session through focus/presentation callbacks.
  if (task_.textFieldEditor().editedWidget() != target || target == nullptr ||
      viewport.empty() || viewport == bounds())
    return;
  if (scrollable) {
    // Nested scrollers each reveal the target in their own coordinates.
    for (Widget* ancestor = target->parent();
         ancestor != nullptr && ancestor != this;
         ancestor = ancestor->parent()) {
      ancestor->revealFocusedDescendant(*target);
    }
  } else {
    // Static forms keep their measured size. Pan within the original task's
    // clipping boundary without another editor or a copied text buffer.
    Rect field = DescendantBounds(*target, *this);
    int dy = 0;
    if (field.yMax() > viewport.yMax()) dy = viewport.yMax() - field.yMax();
    if (field.yMin() + dy < viewport.yMin())
      dy = viewport.yMin() - field.yMin();
    if (dy != 0) content_->layout(content_->parent_bounds().translate(0, dy));
  }
}
}  // namespace roo_windows
