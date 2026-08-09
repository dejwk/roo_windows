#pragma once

#include "roo_windows/core/panel.h"

namespace roo_windows {
class UiTask;

/// Surface-owning container for a UiTask's one borrowed content widget.
class TaskPanel : public Panel {
 public:
  TaskPanel(ApplicationContext& context, UiTask& ui_task)
      : Panel(context), ui_task_(ui_task) {}
  ~TaskPanel();

  UiTask* getUiTask() override { return &ui_task_; }
  const UiTask* getUiTask() const override { return &ui_task_; }
 bool fillTouchTargetPath(XDim x, YDim y, std::vector<Widget*>& path) override;

 protected:
  int getChildrenCount() const override { return content_ == nullptr ? 0 : 1; }
  const Widget& getChild(int) const override { return *content_; }
  Widget& getChild(int) override { return *content_; }
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  friend class UiTask;
  void setContent(Widget& content, const roo_display::Box& bounds);
  void clearContent();
  UiTask& ui_task_;
  Widget* content_ = nullptr;
};
}  // namespace roo_windows
