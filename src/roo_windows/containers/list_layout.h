#pragma once

#include "roo_windows/config.h"
#include "roo_windows/core/dimensions.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

template <typename ElementType>
class ListModel {
 public:
  using Element = ElementType;

  ListModel() {}
  virtual ~ListModel() {}

  // How many items in the list?
  virtual int elementCount() const = 0;

  // Set the specified visual element to the ith item of the list.
  virtual void set(int idx, Element& dest) const = 0;
};

template <typename Element>
class CircularBuffer {
 public:
  CircularBuffer(const Environment& env)
      : elements_(nullptr), capacity_(0), start_(0), count_(0) {}

  ~CircularBuffer() { operator delete[](elements_.release()); }

  CircularBuffer(CircularBuffer&& other)
      : elements_(other.elements_.release()),
        capacity_(other.capacity_),
        start_(other.start_),
        count_(other.count_) {}

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

  int pos(int idx) const { return (start_ + idx) % capacity_; }

  Element& operator[](int idx) { return elements_[pos(idx)]; }
  const Element& operator[](int idx) const { return elements_[pos(idx)]; }

  bool empty() const { return count_ == 0; }

  size_t capacity() const { return capacity_; }

  size_t count() const { return count_; }

 private:
  std::unique_ptr<Element[]> elements_;
  int capacity_;
  int start_;
  int count_;
};

inline Rect unclippedRegion(const Widget* w) {
  Rect rect = w->bounds();
  XDim dx = 0;
  YDim dy = 0;
  while (w->parent() != nullptr) {
    dx -= w->offsetLeft();
    dy -= w->offsetTop();
    if (w->getParentClipMode() == Widget::CLIPPED) {
      rect = Rect::Intersect(rect, w->parent()->bounds().translate(dx, dy));
    }
    w = w->parent();
  }
  return rect;
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
        in_paint_children_(false),
        paint_offset_(0) {
    element_count_ = model.elementCount();
  }

  void setPadding(Padding padding) {
    if (padding_ == padding) return;
    padding_ = padding;
    requestLayout();
  }

  Padding getPadding() const override { return padding_; }

  // Notifies this view that the contents of the list has changed. Causes the
  // list to be updated and rendered. If the element count has changed, the list
  // is laid out, with new elements appearning or gone elements disappearing.
  void modelChanged() { modelRangeChanged(first_, last_ + 1); }

  // Returns the index of the first model item in the visible range.
  int first() const { return first_; }

  // Returns the index of the last model item in the visible range.
  int last() const { return last_; }

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
      setFromModel(pos, elements_[i]);
      ++i;
      ++pos;
    }
    setDirty();
  }

  // Notifies this view that the specified list item has changed. Causes the
  // item to be updated and rendered. If the element count has changed, the list
  // is laid out, with new elements appearning or gone elements disappearing.
  void modelItemChanged(int idx) { modelRangeChanged(idx, idx + 1); }

  bool respectsChildrenBoundaries() const override { return true; }

 protected:
  void propagateDirty(const Widget* child, const Rect& rect) override {
    if (in_paint_children_) return;
    Panel::propagateDirty(child, rect);
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

  const Widget& getChild(int idx) const override {
    return elements_[((elements_.capacity() - idx - 1) + paint_offset_) %
                     elements_.capacity()];
  }

  Widget& getChild(int idx) override {
    return elements_[((elements_.capacity() - idx - 1) + paint_offset_) %
                     elements_.capacity()];
  }

  void paintChildren(const Canvas& canvas, Clipper& clipper) override {
    paint_offset_ = (paint_offset_ + 1) % elements_.capacity();
    CHECK(!in_paint_children_);
    in_paint_children_ = true;
    // NOTE: we are not just using clipbox, because it could result in removing
    // some children that just happen to not need rendering at the moment -
    // but we want to keep them so that they can keep receiving events.
    Rect rect = unclippedRegion(this);
    int new_first = (rect.yMin() + 1) / element_height();
    if (new_first < 0) new_first = 0;
    int new_last = rect.yMax() / element_height();
    if (new_last < new_first) new_last = new_first - 1;
    if (new_last - new_first >= element_count_) {
      new_last = new_first + element_count_ - 1;
    }

    // First, see if we need to shrink the list from any side.
    while (first_ < new_first && first_ <= last_) {
      elements_.pop_front().setVisibility(GONE);
      ++first_;
    }
    while (new_last < last_ && first_ <= last_) {
      elements_.pop_back().setVisibility(GONE);
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
    Panel::paintChildren(canvas, clipper);
  }

  PreferredSize getPreferredSize() const override {
    PreferredSize element = prototype_.getPreferredSize();
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         element.height().isExact()
                             ? PreferredSize::ExactHeight(
                                   calculateHeight(element.height().value()))
                             : element.height());
  }

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    int16_t h_padding = padding_.left() + padding_.right();
    int16_t v_padding = padding_.top() + padding_.bottom();
    // Measure the element under new constraints, and see how many max instances
    // will fit on the screen.
    Margins margins = prototype_.getMargins();
    int16_t h_margin = margins.left() + margins.right();
    int16_t v_margin = margins.top() + margins.bottom();
    PreferredSize preferred = prototype_.getPreferredSize();
    Dimensions d = prototype_.measure(
        width.getChildWidthSpec(h_padding + h_margin, preferred.width()),
        preferred.height().isExact()
            ? HeightSpec::Exactly(preferred.height().value())
            : HeightSpec::Unspecified(40));
    YDim h = d.height();
    if (h == 0) {
      h = prototype_.getNaturalDimensions().height();
    }
    if (h == 0) h = Scaled(15);
    h += v_margin;
    return Dimensions(width.resolveSize(d.width() + h_margin + h_padding),
                      height.resolveSize(calculateHeight(h)));
  }

  void onLayout(bool changed, const Rect& rect) override {
    // NOTE: because we do deferred layout in paint, we don't actually call
    // the superclass' onLayout from here.
    if (rect.height() <= 0) {
      return;
    }
    prototype_.setVisibility(VISIBLE);
    int16_t h =
        (element_count_ == 0) ? rect.height() : rect.height() / element_count_;
    prototype_.layout(Rect(0, 0, rect.width() - 1, h - 1));
    prototype_.setVisibility(GONE);
    bool moved = (rect.yMin() != parent_bounds().yMin() ||
                  rect.yMax() != parent_bounds().yMax());
    int capacity = (getMainWindow()->height() - 2) / element_height() + 2;
    if (capacity != elements_.capacity() || moved) {
      // Invalidate all the children so that they get repositioned during next
      // paintChildren().
      while (last_-- >= first_) {
        elements_.pop_back().setVisibility(GONE);
      }
      removeAll();
      if (element_count_ == 0) return;
      while (getChildrenCount() > 1) {
        removeLast();
      }
      elements_.ensure_capacity(capacity, prototype_);
      for (int i = 0; i < elements_.capacity(); i++) {
        add(elements_[i]);
      }
      first_ = 0;
      last_ = -1;
    }
  }

 private:
  void setFromModel(int pos, Element& e) {
    model_.set(pos, e);
    if (e.isLayoutRequested()) {
      layoutElement(pos, e);
    }
  }

  // Configures the given element to represent the item at the given pos.
  void show(int pos, Element& e) {
    Margins m = prototype_.getMargins();
    if (element_height() <= m.top() + m.bottom()) {
      // Won't fit anyway.
      return;
    }
    setFromModel(pos, e);
    e.setVisibility(VISIBLE);
  }

  void layoutElement(int pos, Element& e) {
    Margins m = prototype_.getMargins();
    int16_t h_padding = padding_.left() + padding_.right();
    int16_t v_padding = padding_.top() + padding_.bottom();
    Dimensions d = e.measure(
        WidthSpec::Exactly(width() - m.left() - m.right() - h_padding),
        HeightSpec::Exactly(element_height() - m.top() - m.bottom()));
    int hoffset = m.left() + padding_.left();
    int voffset = pos * element_height() + padding_.top() + m.top();
    // TODO: support gravity, and margins. And maybe dividers?
    e.layout(Rect(hoffset, voffset, hoffset + d.width() - 1,
                  voffset + d.height() - 1));
  }

  YDim calculateHeight(int16_t desired_element_height) const {
    int16_t v_padding = padding_.top() - padding_.bottom();
    YDim tops = Rect::MaximumRect().yMax() - v_padding;
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
  int paint_offset_;
};

template <typename Element>
ListLayout<Element> MakeListLayout(const Environment& env,
                                   ListModel<Element>& model,
                                   Element prototype) {
  ListLayout<Element> layout(env, model, std::move(prototype));
  return std::move(layout);
}

}  // namespace roo_windows
