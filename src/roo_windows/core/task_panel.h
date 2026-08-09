#pragma once

#include "roo_windows/core/panel.h"

namespace roo_windows {
class Task;

/// Surface-owning container for a Task's one borrowed content widget.
class TaskPanel : public Panel {
 public:
  TaskPanel(ApplicationContext& context, Task& task) : Panel(context), task_(task) {}
  ~TaskPanel();

  Task* getTask() override { return &task_; }
  const Task* getTask() const override { return &task_; }
 bool fillTouchTargetPath(XDim x, YDim y, std::vector<Widget*>& path) override;

 protected:
  int getChildrenCount() const override { return content_ == nullptr ? 0 : 1; }
  const Widget& getChild(int) const override { return *content_; }
  Widget& getChild(int) override { return *content_; }
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  friend class Task;
  void setContent(Widget& content, const roo_display::Box& bounds);
  void clearContent();
  Task& task_;
  Widget* content_ = nullptr;
};
}  // namespace roo_windows
