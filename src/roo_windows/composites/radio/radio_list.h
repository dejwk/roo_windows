#pragma once

#include "roo_windows/containers/holder.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/widgets/radio_button.h"

namespace roo_windows {

class RadioListModel;

class RadioListItem : public HorizontalLayout {
 public:
  RadioListItem(const Environment& env, std::unique_ptr<Widget> item,
                std::function<void(int idx)> on_click)
      : HorizontalLayout(env),
        button_(env),
        item_(std::move(item)),
        on_click_(on_click) {
    init();
  }

  void set(int idx, bool is_on) {
    idx_ = idx;
    if (is_on) {
      button_.setOn();
    } else {
      button_.setOff();
    }
  }

  // Consider optionally enabling by configuration.
  // Enabling this allows the entire list item to be clicked, which may be fine
  // for short lists. For long scrollable lists, it might get annoying vis-a-vis
  // scroll gestures.
  //
  // bool isClickable() const override { return true; }

  void onClicked() override { on_click_(idx_); }

  Widget& item() { return *item_; }

  const Widget& item() const { return *item_; }

 private:
  friend class RadioListModel;

  void init() {
    // item().setMargins(MarginSize::NONE);
    // item().setPadding(PaddingSize::NONE);
    setGravity(kGravityMiddle);
    setPadding(Padding(PaddingSize::TINY, PaddingSize::NONE));
    button_.setMargins(MarginSize::NONE, MarginSize::SMALL);
    add(button_);
    add(item(), {weight : 1});
    button_.setOnInteractiveChange([this]() { on_click_(idx_); });
  }

  int idx_;
  RadioButton button_;
  std::unique_ptr<Widget> item_;
  std::function<void(int idx)> on_click_;
};

class RadioListModel : public ListModel {
 public:
  RadioListModel() : model_(nullptr), selected_(-1) {}
  RadioListModel(ListModel& model) : model_(&model), selected_(-1) {}

  int elementCount() const override {
    return model_ == nullptr ? 0 : model_->elementCount();
  }

  void set(int idx, Widget& dest) const override {
    // Note: set is not expected tobe called when model_ is nullptr, because we
    // report zero element count in such case.
    model_->set(idx, ((RadioListItem&)dest).item());
    ((RadioListItem&)dest).set(idx, idx == selected_);
  }

  void setModel(ListModel& model) {
    model_ = &model;
    selected_ = -1;
  }

  void clearModel() {
    model_ = nullptr;
    selected_ = -1;
  }

  bool setSelected(int idx) {
    if (idx == selected_) return false;
    selected_ = idx;
    return true;
  }

  int selected() const { return selected_; }

 private:
  const ListModel* model_;
  int selected_;
};

class RadioList : public Holder {
 public:
  RadioList(const Environment& env,
            std::function<std::unique_ptr<Widget>()> prototype_fn)
      : Holder(env),
        list_model_(),
        list_(env, list_model_, [&, prototype_fn]() {
          return std::make_unique<RadioListItem>(
              env, prototype_fn(), [this](int idx) { elementSelected(idx); });
        }) {}

  RadioList(const Environment& env, RadioListModel& model,
            const std::function<std::unique_ptr<Widget>()>& prototype_fn)
      : RadioList(env, prototype_fn) {
    setModel(model);
  }

  int selected() const { return list_model_.selected(); }

  void setModel(RadioListModel& model) {
    list_model_.setModel(model);
    setContents(list_);
  }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }

  void reset() {
    list_model_.setSelected(-1);
    modelChanged();
  }

  void modelChanged() {
    list_.modelChanged();
    triggerInteractiveChange();
  }

  void setSelected(int selected) {
    int selected_old = list_model_.selected();
    if (list_model_.selected() == selected_old) return;
    list_model_.setSelected(selected);
    if (selected_old >= 0) list_.modelItemChanged(selected_old);
    if (selected >= 0) list_.modelItemChanged(selected);
  }

 private:
  void elementSelected(int idx) {
    int old = list_model_.selected();
    if (list_model_.setSelected(idx)) {
      if (idx >= 0) list_.modelItemChanged(idx);
      triggerInteractiveChange();
    }
    if (old >= 0) list_.modelItemChanged(old);
  }

  RadioListModel list_model_;
  ListLayout list_;
};

}  // namespace roo_windows
