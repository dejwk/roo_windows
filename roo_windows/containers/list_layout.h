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

template <typename Element>
class ListLayout : public Panel {
 public:
  // Creates a list layout that uses the specified prototype element.
  ListLayout(const Environment& env, ListModel<Element>& model,
             Element prototype)
      : Panel(env),
        model_(model),
        elements_(env),
        prototype_(std::move(prototype)),
        first_(0),
        last_(-1),
        in_paint_children_(false) {}

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
    int element_count = model_.elementCount();
    int new_first = ((s.clip_box().yMin() - s.dy() + 1) / element_height());
    if (new_first < 0) new_first = 0;
    int new_last = ((s.clip_box().yMax() - s.dy()) / element_height());
    if (new_last < new_first) new_last = new_first - 1;
    if (new_last - new_first >= element_count) {
      new_last = new_first + element_count - 1;
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
            ? PreferredSize::Exact(
                  calculatePossibleElementHeight(element.height().value()) *
                  model_.elementCount())
            : element.height());
  }

  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override {
    // Measure the element under new constraints, and see how many max instances
    // will fit on the screen.
    Dimensions d = prototype_.measure(width, MeasureSpec::Unspecified(40));
    int32_t h = d.height();
    if (h == 0) h = prototype_.getNaturalDimensions().height();
    if (h == 0) h = 15;
    Padding p = prototype_.getDefaultPadding();
    h += (p.top() + p.bottom());
    return Dimensions(width.resolveSize(this->width()),
                      height.resolveSize(calculatePossibleElementHeight(h)));
  }

  void onLayout(bool changed, const roo_display::Box& box) override {
    // Invalidate all the children so that they get repositioned during next
    // paintChildren().
    while (last_-- >= first_) {
      elements_.pop_back().setVisible(false);
    }
    removeAll();
    add(prototype_);
    prototype_.setVisible(true);
    prototype_.layout(
        Box(0, 0, box.width() - 1, box.height() / model_.elementCount() - 1));
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
    model_.set(pos, e);
    e.setVisible(true);
    Dimensions d = e.measure(MeasureSpec::Unspecified(width()),
                             MeasureSpec::Exactly(element_height()));
    int voffset = pos * element_height();
    // TODO: support gravity, and margins. And maybe dividers?
    Padding p = prototype_.getDefaultPadding();
    e.layout(Box(0, voffset, d.width() + p.left() + p.right() - 1,
                 voffset + element_height() - 1));
  }

  int16_t calculatePossibleElementHeight(int16_t desired_height) const {
    int count = model_.elementCount();
    int tops = Box::MaximumBox().yMax();
    if (count >= tops) return 1;
    if (count * desired_height > tops) {
      return tops / count;
    }
    return desired_height;
  }

  int element_height() const { return prototype_.height(); }

  ListModel<Element>& model_;
  CircularBuffer<Element> elements_;
  Element prototype_;
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
