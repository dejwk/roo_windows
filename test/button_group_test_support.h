#pragma once
#include "roo_windows/material3/button/internal/button_group_base.h"
namespace roo_windows::material3::test_support {
// Exercises internal membership before the public family is available.
class InternalGroup : public internal::ButtonGroupBase {
 public:
  using ButtonGroupBase::ButtonGroupBase;
  ~InternalGroup() override { clearEntries(); }
  bool add(internal::ButtonGroupEntry& entry) {
    return addEntry(&entry, WidgetRef(entry));
  }
  template <class T>
  bool adopt(std::unique_ptr<T> entry) {
    internal::ButtonGroupEntry* raw = entry.get();
    return addEntry(raw, WidgetRef(std::move(entry)));
  }
  void clear() { clearEntries(); }
  size_t capacity() const { return entries_.capacity(); }

 protected:
  bool connected() const override { return true; }
  void activateEntry(internal::ButtonGroupEntry&) override {}
};
}  // namespace roo_windows::material3::test_support
