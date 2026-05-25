#pragma once

#include "roo_windows/containers/holder.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/widgets/radio_button.h"

namespace roo_windows {

class RadioListModel;

/// Single row of a `RadioList`: a radio button plus a caller-supplied item
/// widget laid out horizontally.
///
/// The wrapping `RadioListModel` drives `set()` to keep the radio button's
/// on/off state in sync with the current selection; clicking the radio button
/// (or, optionally, the entire row) reports the row's index through the
/// stored on-click callback.
class RadioListItem : public HorizontalLayout {
 public:
  RadioListItem(ApplicationContext& context, std::unique_ptr<Widget> item,
                std::function<void(int idx)> on_click)
      : HorizontalLayout(context),
        button_(context),
        item_(std::move(item)),
        on_click_(on_click) {
    init();
  }

  /// Binds this row to its model index and synchronizes the radio button's
  /// on/off state to the model's current selection.
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

  /// Reports this row's index through the stored on-click callback.
  void onClicked() override { on_click_(idx_); }

  /// Returns the caller-supplied item widget hosted in this row.
  Widget& item() { return *item_; }

  const Widget& item() const { return *item_; }

 private:
  friend class RadioListModel;

  void init() {
    setGravity(kGravityMiddle);
    setPadding(Padding(PaddingSize::kTiny, PaddingSize::kNone));
    button_.setMargins(MarginSize::kNone, MarginSize::kSmall);
    add(button_);
    add(item(), {weight : 1});
    button_.setOnInteractiveChange([this]() { on_click_(idx_); });
  }

  int idx_;
  RadioButton button_;
  std::unique_ptr<Widget> item_;
  std::function<void(int idx)> on_click_;
};

/// `ListModel` adapter that adds single-selection state on top of an existing
/// item model.
///
/// The wrapped `model_` provides the row contents; this class tracks which
/// index is selected and ensures each `RadioListItem` bound by `set()` shows
/// the correct radio-button state.
class RadioListModel : public ListModel {
 public:
  RadioListModel() : model_(nullptr), selected_(-1) {}
  RadioListModel(ListModel& model) : model_(&model), selected_(-1) {}

  /// Returns the number of items from the wrapped model, or 0 when no model
  /// has been bound.
  int elementCount() const override {
    return model_ == nullptr ? 0 : model_->elementCount();
  }

  /// Populates `dest` (a `RadioListItem`) by delegating to the wrapped model
  /// and then syncing the radio button to match the selected index.
  void set(int idx, Widget& dest) const override {
    // Note: set is not expected tobe called when model_ is nullptr, because we
    // report zero element count in such case.
    model_->set(idx, ((RadioListItem&)dest).item());
    ((RadioListItem&)dest).set(idx, idx == selected_);
  }

  /// Replaces the wrapped item model and clears the current selection.
  void setModel(ListModel& model) {
    model_ = &model;
    selected_ = -1;
  }

  /// Detaches the wrapped model and clears the current selection.
  void clearModel() {
    model_ = nullptr;
    selected_ = -1;
  }

  /// Updates the selected index; returns true when the value actually
  /// changed.
  bool setSelected(int idx) {
    if (idx == selected_) return false;
    selected_ = idx;
    return true;
  }

  /// Returns the currently selected index, or -1 when nothing is selected.
  int selected() const { return selected_; }

 private:
  const ListModel* model_;
  int selected_;
};

/// High-level single-selection list view.
///
/// Wraps a `ListLayout` of `RadioListItem`s and a `RadioListModel` inside a
/// `Holder`, exposing `setModel()` / `setSelected()` / `selected()` plus an
/// interactive-change callback. The caller supplies a prototype function that
/// builds the per-row item widget; selection state is owned here.
class RadioList : public Holder {
 public:
  RadioList(ApplicationContext& context,
            std::function<std::unique_ptr<Widget>()> prototype_fn)
      : Holder(context),
        list_model_(),
        list_(context, list_model_, [&, prototype_fn]() {
          return std::make_unique<RadioListItem>(
              context, prototype_fn(), [this](int idx) { elementSelected(idx); });
        }) {}

  RadioList(ApplicationContext& context, ListModel& model,
            const std::function<std::unique_ptr<Widget>()>& prototype_fn)
      : RadioList(context, prototype_fn) {
    setModel(model);
  }

  /// Returns the currently selected index, or -1 when nothing is selected.
  int selected() const { return list_model_.selected(); }

  /// Binds an item model; rebuilds the underlying list view from scratch.
  void setModel(ListModel& model) {
    list_model_.setModel(model);
    setContents(list_);
  }

  /// Disallowed: callers must bind their own `ListModel`, not the internal
  /// adapter.
  void setModel(RadioListModel& model) = delete;

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }

  /// Clears the current selection and re-fires the interactive-change hook.
  void reset() {
    list_model_.setSelected(-1);
    modelChanged();
  }

  /// Notifies the list view that its model contents (or selection) have
  /// changed.
  void modelChanged() {
    list_.modelChanged();
    triggerInteractiveChange();
  }

  /// Programmatically updates the selected index, refreshing the previously
  /// and newly selected rows.
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
