#pragma once

#include <functional>
#include <string>

#include "roo_windows/core/destination.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/core/navigation_host.h"

namespace roo_windows {
namespace internal {
class TextEditTarget;
}

/// Shares admission, keyboard confirmation, and detached draft completion.
/// Concrete destinations own their controls; both systems share source routing.
class EditorDestination : public Destination {
 public:
  /// Returns whether this destination is attached or delivering source
  /// completion.
  bool busy() const {
    return editing_ || completing_ || getNavigationHost() != nullptr;
  }

  /// Cancels the draft and leaves navigation history.
  BackResult onBackRequested(BackSource source) override;

  /// Resumes in-place input using the single application keyboard.
  void onResume() override;

  /// Clears the draft and delivers completion after navigation removal.
  void onRemoved() override;

  /// Edits a copy of @p initial using @p hint and optional masking.
  /// Returns false without completion if this or the app-owned destination is
  /// busy, the host is unavailable, or navigation callbacks supersede
  /// admission. The host must belong to the same context. Call on the UI
  /// thread. Confirmation calls @p enter_fn after removal with a temporarily
  /// borrowed string. Cancellation discards the draft. The callback may
  /// navigate or destroy this detached destination; captured objects must
  /// survive delivery.
  bool triggerEdit(NavigationHost& navigation, const std::string& initial,
                   const std::string& hint,
                   std::function<void(const std::string&)> enter_fn,
                   bool obscure = false);

  /// Starts a draft for an attached target with @p hint as its label.
  /// Field adapters expose this through editInDestination(). Returns false
  /// while busy or if navigation refuses admission. The application-owned
  /// destination observes source destruction through forgetSource().
  bool triggerEditField(internal::TextEditTarget& source,
                        const std::string& hint);

  /// Releases a source before its buffer and callback hooks are destroyed.
  void forgetSource(internal::TextEditTarget& source);

 protected:
  EditorDestination()
      : editing_(false), confirmed_(false), completing_(false) {}

  /// Restricts editor controls to the viewport above the hosted keyboard.
  static HeightSpec AvailableHeight(const Widget& pane, HeightSpec height);

  /// Returns whether a new draft can enter @p navigation.
  bool canStart(NavigationHost& navigation);

  /// Removes the destination and commits its draft.
  void confirm();

  virtual void prepareDraft(const std::string& initial, const std::string& hint,
                            bool obscure) = 0;
  virtual const std::string& draft() const = 0;
  virtual void clearDraft() = 0;
  virtual void editInPlace() = 0;

 private:
  void finishSource(bool confirmed, const std::string& value);
  internal::TextEditTarget* source_ = nullptr;
  bool editing_ : 1;
  bool confirmed_ : 1;
  bool completing_ : 1;
  std::function<void(const std::string&)> enter_fn_;
};

}  // namespace roo_windows
