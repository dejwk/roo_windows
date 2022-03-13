#pragma once

#include "roo_windows/core/dimensions.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

template <typename Element>
class ListModel {
 public:
  ListModel() {}
  virtual ~ListModel() {}

  virtual int elementCount() = 0;

  virtual int elementHeight() = 0;

  virtual PreferredSize::Dimension preferredWidth() {
    return PreferredSize::MatchParent();
  }

  virtual int maxVisibleElementCount() = 0;

  virtual void set(int idx, Element& dest) = 0;
};

// Point * p = static_cast<Point*>(::operator new[](k*sizeof(Point)));
// // placement new construction
// for (std::size_t i{0}; i<k; ++i)
// {
//   new((void *)(p+i)) Point{5};
// }

template <typename Element>
class CircularBuffer {
 public:
  template <typename... Args>
  CircularBuffer(const Environment& env, int capacity, Args&&... args)
      : elements_(static_cast<Element*>(
            ::operator new[](capacity * sizeof(Element)))),
        capacity_(capacity),
        start_(0),
        count_(0) {
    for (int i = 0; i < capacity; ++i) {
      new (&elements_[i]) Element(env, std::forward<Args>(args)...);
    }
  }

  Element& push_back() {
    int offset = (start_ + count_) % capacity_;
    ++count_;
    return elements_[offset];
  }

  Element& pop_back() {
    --count_;
    int offset = (start_ + count_) % capacity_;
    return elements_[offset];
  }

  Element& push_front() {
    --start_;
    if (start_ < 0) start_ += capacity_;
    ++count_;
    return elements_[start_];
  }

  Element& pop_front() {
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
  template <typename... Args>
  ListLayout(const Environment& env, ListModel<Element>& model, Args&&... args)
      : Panel(env),
        model_(model),
        elements_(env, model.maxVisibleElementCount(),
                  std::forward<Args>(args)...),
        element_height_(model.elementHeight()),
        first_(0),
        last_(-1) {
    for (int i = 0; i < elements_.capacity(); ++i) {
      add(elements_[i]);
    }
  }

  void paintChildren(const Surface& s, Clipper& clipper) override {
    int element_count = model_.elementCount();
    int new_first = ((s.clip_box().yMin() - s.dy() + 1) / element_height_) - 1;
    if (new_first < 0) new_first = 0;
    int new_last = ((s.clip_box().yMax() - s.dy()) / element_height_);
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

    // Now that we have set up all the children, we are ready to paint them.
    Panel::paintChildren(s, clipper);
  }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(
        model_.preferredWidth(),
        PreferredSize::Exact(model_.elementCount() * element_height_));
  }

  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override {
    return Dimensions(
        width.resolveSize(this->width()),
        height.resolveSize(element_height_ * model_.elementCount()));
  }

  void onLayout(bool changed, const roo_display::Box& box) override {
    Panel::onLayout(changed, box);
    // Invalidate all the children so that they get repositioned during next
    // paintChildren().
    while (last_-- >= first_) {
      elements_.pop_back().setVisible(false);
    }
    first_ = 0;
    last_ = -1;
  }

 private:
  // Configures the given element to represent the item at the given pos.
  void show(int pos, Element& e) {
    model_.set(pos, e);
    e.setVisible(true);
    Dimensions d = e.measure(MeasureSpec::AtMost(width()),
                             MeasureSpec::Exactly(element_height_));
    int voffset = pos * element_height_;
    // TODO: support gravity, and margins. And maybe dividers?
    e.layout(Box(0, voffset, d.width() - 1, voffset + element_height_ - 1));
  }

  ListModel<Element>& model_;
  CircularBuffer<Element> elements_;
  int element_height_;
  int first_;
  int last_;
};

}  // namespace roo_windows
