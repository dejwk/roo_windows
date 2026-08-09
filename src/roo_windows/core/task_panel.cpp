#include "roo_windows/core/task_panel.h"

#include "roo_logging.h"

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

Dimensions TaskPanel::onMeasure(WidthSpec width, HeightSpec height) {
  if (content_ != nullptr) content_->measure(width, height);
  return Dimensions(this->width(), this->height());
}

void TaskPanel::onLayout(bool changed, const Rect& rect) {
  if (content_ != nullptr) {
    content_->layout(Rect(0, 0, width() - 1, height() - 1));
  }
}
}  // namespace roo_windows
