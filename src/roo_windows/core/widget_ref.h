#pragma once

namespace roo_windows {

// A type of a smart pointer to a widget, with optional ownership.
class WidgetRef {
 public:
  // Creates a 'null' widget ref.
  WidgetRef() : ptr_(nullptr), is_owned_(false) {}

  // Creates a WidgetRef that owns the widget.
  template <typename T>
  WidgetRef(std::unique_ptr<T> w) : ptr_(w.release()), is_owned_(true) {}

  // Creates a WidgetRef that does not own the widget.
  WidgetRef(Widget& w) : ptr_(&w), is_owned_(false) {}

  WidgetRef(const WidgetRef& w) = delete;

  WidgetRef(WidgetRef&& w) {
    ptr_ = w.ptr_;
    is_owned_ = w.is_owned_;
    w.ptr_ = nullptr;
  }

  ~WidgetRef() {
    if (is_owned_) delete ptr_;
  }

  Widget* operator->() { return ptr_; }
  Widget& operator*() { return *ptr_; }

  const Widget* operator->() const { return ptr_; }
  const Widget& operator*() const { return *ptr_; }

  Widget* get() { return ptr_; }
  const Widget* get() const { return ptr_; }

  Widget* release() {
    Widget* result = ptr_;
    ptr_ = nullptr;
    return result;
  }

  bool is_owned() const { return is_owned_; }

 private:
  friend class Panel;

  Widget* ptr_;
  bool is_owned_;
};

}  // namespace roo_windows