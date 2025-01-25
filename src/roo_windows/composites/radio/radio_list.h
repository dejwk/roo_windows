#pragma once

#include "roo_windows/containers/holder.h"
#include "roo_windows/containers/horizontal_layout.h"
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
  using Element = typename Model::Element;

  RadioListItem(const roo_windows::Environment& env,
                std::function<void(int idx)> on_click)
      : HorizontalLayout(env), button_(env), item_(), on_click_(on_click) {
    new (item_) Element(env);
    init();
  }

  RadioListItem(const roo_windows::Environment& env,
                const Element& prototype_item,
                std::function<void(int idx)> on_click)
      : HorizontalLayout(env), button_(env), item_(), on_click_(on_click) {
    new (item_) Element(prototype_item);
    init();
  }

  RadioListItem(const RadioListItem& other)
      : HorizontalLayout(other),
        button_(other.button_),
        item_(),
        on_click_(other.on_click_) {
    new (item_) Element(*((Element*)other.item_));
    init();
  }

  ~RadioListItem() { ((Element*)item_)->~Element(); }

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

  Element& item() { return *(Element*)item_; }

  const Element& item() const { return *(Element*)item_; }

 private:
  friend class RadioListModel<Model>;

  void init() {
    item().setMargins(roo_windows::MARGIN_NONE);
    item().setPadding(roo_windows::PADDING_NONE);
    setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                    roo_windows::kVerticalGravityMiddle));
    setPadding(Padding(PADDING_TINY, PADDING_NONE));
    button_.setMargins(roo_windows::MARGIN_NONE, roo_windows::MARGIN_SMALL);
    add(button_);
    add(item(), HorizontalLayout::Params().setWeight(1));
    button_.setOnInteractiveChange([this]() { on_click_(idx_); });
  }

  int idx_;
  RadioButton button_;
  unsigned char item_[sizeof(Element)];
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
    model_->set(idx, dest.item());
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

  RadioList(const roo_windows::Environment& env,
            const typename Model::Element& prototype)
      : Holder(env),
        list_model_(),
        list_(env, list_model_,
              RadioListItem<Model>(
                  env, prototype, [this](int idx) { elementSelected(idx); })) {}

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

  RadioList(const roo_windows::Environment& env, Model& model,
            typename Model::Element prototype)
      : RadioList(env, std::move(prototype)) {
    setModel(model);
  }

  int selected() const { return list_model_.selected(); }

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

  RadioListModel<Model> list_model_;
  RadioListWidget list_;
};

}  // namespace roo_windows
