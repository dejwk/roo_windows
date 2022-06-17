#pragma once

#include "roo_glog/logging.h"
#include "roo_windows/core/dimensions.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

template <typename Element>
class ListModel {
 public:
  ListModel() {}
  virtual ~ListModel() {}

  // How many items in the list?
  virtual int elementCount() = 0;

  // Set the specified visual element to the ith item of the list.
  virtual void set(int idx, Element& dest) = 0;
};

template <typename Element>
class CircularBuffer {
 public:
  CircularBuffer(const Environment& env)
      : elements_(nullptr), capacity_(0), start_(0), count_(0) {}

  void ensure_capacity(int capacity, const Element& prototype) {
    CHECK_EQ(count_, 0);
    if (capacity <= capacity_) return;
    elements_.reset(
        static_cast<Element*>(::operator new[](capacity * sizeof(Element))));
    capacity_ = capacity;
    start_ = 0;
    count_ = 0;
    for (int i = 0; i < capacity; ++i) {
      new (&elements_[i]) Element(prototype);
    }
  }

  Element& push_back() {
    CHECK_LT(count_, capacity_);
    int offset = (start_ + count_) % capacity_;
    ++count_;
    return elements_[offset];
  }

  Element& pop_back() {
    CHECK_GT(count_, 0);
    --count_;
    int offset = (start_ + count_) % capacity_;
    return elements_[offset];
  }

  Element& push_front() {
    CHECK_LT(count_, capacity_);
    --start_;
    if (start_ < 0) start_ += capacity_;
    ++count_;
    return elements_[start_];
  }

  Element& pop_front() {
    CHECK_GT(count_, 0);
    int offset = start_;
    ++start_;
    if (start_ >= capacity_) start_ -= capacity_;
    --count_;
    return elements_[offset];
  }

  Element& operator[](int idx) { return elements_[(start_ + idx) % capacity_]; }

  bool empty() const { return count_ == 0; }

  size_t capacity() const { return capacity_; }

 private:
  std::unique_ptr<Element[]> elements_;
  int capacity_;
  int start_;
  int count_;
};

Box unclippedRegion(const Widget* w) {
  Box box = w->bounds();
  int16_t dx = 0;
  int16_t dy = 0;
  while (w->parent() != nullptr) {
    dx -= w->xOffset();
    dy -= w->yOffset();
    if (w->getParentClipMode() == Widget::CLIPPED) {
      box.clip(w->parent()->bounds().translate(dx, dy));
    }
    w = w->parent();
  }
  return box;
}

template <typename Element>
class ListLayout : public Panel {
 public:
  // Creates a list layout that uses the specified prototype element.
  ListLayout(const Environment& env, ListModel<Element>& model,
             Element prototype)
      : Panel(env),
        padding_(),
        model_(model),
        elements_(env),
        prototype_(std::move(prototype)),
        first_(0),
        last_(-1),
        in_paint_children_(false) {
    element_count_ = model.elementCount();
  }

  void setPadding(Padding padding) {
    if (padding_ == padding) return;
    padding_ = padding;
    requestLayout();
  }

  Padding getDefaultPadding() const override { return padding_; }

  // Notifies this view that the contents of the list has changed. Causes the
  // list to be updated and rendered. If the element count has changed, the list
  // is laid out, with new elements appearning or gone elements disappearing.
  void modelChanged() { modelRangeChanged(first_, last_ + 1); }

  // Notifies this view that the range of the list has changed. Causes the range
  // to be updated and rendered. If the element count has changed, the list
  // is laid out, with new elements appearning or gone elements disappearing.
  //
  // If the element count has changed but the common subset of the elements has
  // not changed (e.g. if elements has been only added or only removed from the
  // end), specify an empty range (e.g., 0, 0).
  void modelRangeChanged(int begin, int end) {
    int prev_count = element_count_;
    element_count_ = model_.elementCount();
    if (prev_count != element_count_) {
      requestLayout();
    }
    if (first_ > last_) return;
    if (end <= first_ || begin > last_) return;
    int i = 0;
    int pos = first_;
    if (begin > first_) {
      i = begin - first_;
      pos += i;
    }
    while (pos <= last_ && pos < end && pos < element_count_) {
      model_.set(pos, elements_[i]);
      elements_[i].setVisible(true);
      ++i;
      ++pos;
    }
    markDirty();
  }

  // Notifies this view that the specified list item has changed. Causes the
  // item to be updated and rendered. If the element count has changed, the list
  // is laid out, with new elements appearning or gone elements disappearing.
  void modelItemChanged(int idx) { modelRangeChanged(idx, idx + 1); }

 protected:
  void propagateDirty(const Widget* child, const Box& box) override {
    if (in_paint_children_) return;
    Panel::propagateDirty(child, box);
  }

  void childHidden(const Widget* child) override {
    if (in_paint_children_) return;
    Panel::childHidden(child);
  }

  void childShown(const Widget* child) override {
    if (in_paint_children_) return;
    Panel::childShown(child);
  }

  void onRequestLayout() override {
    if (in_paint_children_) return;
    Widget::onRequestLayout();
  }

  void paintChildren(const Surface& s, Clipper& clipper) override {
    CHECK(!in_paint_children_);
    in_paint_children_ = true;
    // NOTE: we are not just using clipbox, because it could result in removing
    // some children that just happen to not need rendering at the moment -
    // but we want to keep them so that they can keep receiving events.
    Box box = unclippedRegion(this);
    int new_first = (box.yMin() + 1) / element_height();
    if (new_first < 0) new_first = 0;
    int new_last = box.yMax() / element_height();
    if (new_last < new_first) new_last = new_first - 1;
    if (new_last - new_first >= element_count_) {
      new_last = new_first + element_count_ - 1;
    }

    // First, see if we need to shrink the list from any side.
    while (first_ < new_first && first_ <= last_) {
      elements_.pop_front().setVisible(false);
      ++first_;
    }
    while (new_last < last_ && first_ <= last_) {
      elements_.pop_back().setVisible(false);
      --last_;
    }
    if (first_ > last_) {
      // All hidden; nothing to reuse.
      first_ = new_first;
      last_ = first_ - 1;
    }
    // Now, grow if needed.
    while (first_ > new_first) {
      show(--first_, elements_.push_front());
    }
    while (last_ < new_last) {
      show(++last_, elements_.push_back());
    }

    // markClean();
    in_paint_children_ = false;

    // Now that we have set up all the children, we are ready to paint them.
    Panel::paintChildren(s, clipper);
  }

  PreferredSize getPreferredSize() const override {
    PreferredSize element = prototype_.getPreferredSize();
    return PreferredSize(
        PreferredSize::MatchParent(),
        element.height().isExact()
            ? PreferredSize::Exact(calculateHeight(element.height().value()))
            : element.height());
  }

  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override {
    int16_t h_padding = padding_.left() + padding_.right();
    int16_t v_padding = padding_.top() + padding_.bottom();
    // Measure the element under new constraints, and see how many max instances
    // will fit on the screen.
    Margins margins = prototype_.getDefaultMargins();
    int16_t h_margin = margins.left() + margins.right();
    int16_t v_margin = margins.top() + margins.bottom();
    PreferredSize preferred = prototype_.getPreferredSize();

    Dimensions d = prototype_.measure(
        width.getChildMeasureSpec(h_padding + h_margin, preferred.width()),
        MeasureSpec::Unspecified(40));

    int32_t h = d.height();
    if (h == 0) {
      h = prototype_.getNaturalDimensions().height();
    }
    if (h == 0) h = 15;
    h += v_margin;
    return Dimensions(width.resolveSize(d.width() + h_margin + h_padding),
                      height.resolveSize(calculateHeight(h)));
  }

  void onLayout(bool changed, const roo_display::Box& box) override {
    // Invalidate all the children so that they get repositioned during next
    // paintChildren().
    while (last_-- >= first_) {
      elements_.pop_back().setVisible(false);
    }
    removeAll();
    if (element_count_ == 0) return;
    add(prototype_);
    prototype_.setVisible(true);
    prototype_.layout(
        Box(0, 0, box.width() - 1, box.height() / element_count_ - 1));
    prototype_.setVisible(false);
    Panel::onLayout(changed, box);
    int capacity = (getMainWindow()->height() - 2) / element_height() + 2;
    elements_.ensure_capacity(capacity, prototype_);
    for (int i = 0; i < elements_.capacity(); i++) {
      add(elements_[i]);
    }
    first_ = 0;
    last_ = -1;
  }

 private:
  // Configures the given element to represent the item at the given pos.
  void show(int pos, Element& e) {
    Margins m = prototype_.getDefaultMargins();
    if (element_height() <= m.top() + m.bottom()) {
      // Won't fit anyway.
      return;
    }
    model_.set(pos, e);
    e.setVisible(true);
    int16_t h_padding = padding_.left() + padding_.right();
    int16_t v_padding = padding_.top() + padding_.bottom();
    Dimensions d = e.measure(
        MeasureSpec::Exactly(width() - m.left() - m.right() - h_padding),
        MeasureSpec::Exactly(element_height() - m.top() - m.bottom()));
    int hoffset = m.left() + padding_.left();
    int voffset = pos * element_height() + padding_.top() + m.top();
    // TODO: support gravity, and margins. And maybe dividers?
    e.layout(Box(hoffset, voffset, hoffset + d.width() - 1,
                 voffset + d.height() - 1));
  }

  int16_t calculateHeight(int16_t desired_element_height) const {
    int16_t v_padding = padding_.top() - padding_.bottom();
    int tops = Box::MaximumBox().yMax() - v_padding;
    if (element_count_ >= tops) return 1;
    if (element_count_ * desired_element_height > tops) {
      return (tops / element_count_) * element_count_ + v_padding;
    }
    return desired_element_height * element_count_ + v_padding;
  }

  int element_height() const { return prototype_.height(); }

  Padding padding_;
  ListModel<Element>& model_;
  CircularBuffer<Element> elements_;
  Element prototype_;
  int element_count_;
  int element_height_;
  int first_;
  int last_;
  bool in_paint_children_;
};

template <typename Element>
ListLayout<Element> MakeListLayout(const Environment& env,
                                   ListModel<Element>& model,
                                   Element prototype) {
  ListLayout<Element> layout(env, model, std::move(prototype));
  return std::move(layout);
}

}  // namespace roo_windows
