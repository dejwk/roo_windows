#pragma once

namespace roo_windows {

/// Move-only smart pointer to a `Widget`, with optional ownership.
///
/// Used by container APIs that need to accept either a caller-owned reference
/// (`Widget&`) or a unique pointer that should be adopted. The contained
/// widget is `delete`d on destruction only when the ref was constructed with
/// `std::unique_ptr<T>`.
class WidgetRef {
 public:
  /// Creates a null widget ref.
  WidgetRef() : ptr_(nullptr), is_owned_(false) {}

  /// Adopts `w`; the widget will be deleted when this ref is destroyed.
  template <typename T>
  WidgetRef(std::unique_ptr<T> w) : ptr_(w.release()), is_owned_(true) {}

  /// Borrows `w`; ownership stays with the caller.
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

  /// Returns the raw pointer to the referenced widget (may be nullptr).
  Widget* get() { return ptr_; }
  /// Returns the raw pointer to the referenced widget (may be nullptr).
  const Widget* get() const { return ptr_; }

  /// Relinquishes ownership and returns the underlying pointer. The caller
  /// becomes responsible for deleting the widget if it was owned.
  Widget* release() {
    Widget* result = ptr_;
    ptr_ = nullptr;
    return result;
  }

  /// Returns true if this ref owns the widget (i.e. it will delete on
  /// destruction).
  bool is_owned() const { return is_owned_; }

 private:
  friend class Panel;

  Widget* ptr_;
  bool is_owned_;
};

}  // namespace roo_windows