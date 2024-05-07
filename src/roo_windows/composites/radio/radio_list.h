#pragma once

#include "roo_windows/containers/holder.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/widgets/radio_button.h"

namespace roo_windows {

template <typename Model>
class RadioListModel;

template <typename Model>
class RadioListItem : public roo_windows::HorizontalLayout {
 public:
  RadioListItem(const roo_windows::Environment& env,
                std::function<void(int idx)> on_click)
      : HorizontalLayout(env), button_(env), item_(env), on_click_(on_click) {
    setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                    roo_windows::kVerticalGravityMiddle));
    button_.setMargins(roo_windows::MARGIN_NONE, roo_windows::MARGIN_SMALL);
    add(button_);
    item_.setMargins(roo_windows::MARGIN_NONE);
    item_.setPadding(roo_windows::PADDING_NONE);
    add(item_, HorizontalLayout::Params().setWeight(1));
  }

  RadioListItem(const RadioListItem& other)
      : HorizontalLayout(other),
        button_(other.button_),
        item_(other.item_),
        on_click_(other.on_click_) {
    add(button_);
    button_.setOnInteractiveChange([this]() { on_click_(idx_); });
    add(item_);
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

 private:
  friend class RadioListModel<Model>;

  int idx_;
  RadioButton button_;
  typename Model::Element item_;
  std::function<void(int idx)> on_click_;
};

template <typename Model>
class RadioListModel : public roo_windows::ListModel<RadioListItem<Model>> {
 public:
  RadioListModel() : model_(nullptr), selected_(-1) {}
  RadioListModel(Model& model) : model_(&model), selected_(-1) {}

  int elementCount() const override {
    return model_ == nullptr ? 0 : model_->elementCount();
  }

  void set(int idx, RadioListItem<Model>& dest) const override {
    // Note: set is not expected tobe called when model_ is nullptr, because we
    // report zero element count in such case.
    model_->set(idx, dest.item_);
    dest.set(idx, idx == selected_);
  }

  void setModel(Model& model) {
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
  const Model* model_;
  int selected_;
};

template <typename Model>
class RadioList : public roo_windows::Holder {
 public:
  using RadioListWidget = roo_windows::ListLayout<RadioListItem<Model>>;

  RadioList(const roo_windows::Environment& env)
      : Holder(env),
        list_model_(),
        list_(env, list_model_, RadioListItem<Model>(env, [this](int idx) {
                elementSelected(idx);
              })) {}

  RadioList(const roo_windows::Environment& env, Model& model)
      : RadioList(env) {
    setModel(model);
  }

  int selected() const {
    return list_model_.selected();
  }

  void setModel(Model& model) {
    list_model_.setModel(model);
    setContents(list_);
  }

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  void reset() {
    list_model_.setSelected(-1);
    list_.modelChanged();
    triggerInteractiveChange();
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

  RadioListModel<Model> list_model_;
  RadioListWidget list_;
};

}  // namespace roo_windows
