#pragma once

#include <memory>

#include "roo_windows/core/transient_presentation.h"
#include "roo_windows/material3/date_picker/date_picker_types.h"

namespace roo_windows {
class ApplicationContext;
class Task;
class Rect;
namespace material3 {
namespace internal {
class DatePickerSession;
}
class DockedDatePickerField;

/// Reusable single-date presenter. Open widgets exist only during a session.
/// Configuration setters require an idle picker. Confirm commits the draft;
/// every other finish preserves the committed value. Override hooks must keep
/// the presenter alive during admission; terminal hooks may destroy or reopen
/// it.
class ModalDatePicker {
 public:
  /// Creates an idle picker with no selected date or implicit wall clock.
  explicit ModalDatePicker(ApplicationContext& context);

  /// Cancels without application callbacks before presentation resources die.
  virtual ~ModalDatePicker();

  /// Sets the committed date, or invalid for no selection.
  void setValue(roo_time::CivilDay day);

  /// Returns the committed date, unchanged while a draft is being selected.
  roo_time::CivilDay value() const { return value_; }

  /// Sets the explicit today marker, or invalid to hide it.
  void setToday(roo_time::CivilDay day);

  /// Returns the explicit today marker.
  roo_time::CivilDay today() const { return today_; }

  /// Sets inclusive bounds. Reversed bounds reject admission.
  void setBounds(DatePickerBounds bounds);

  /// Returns the configured interval.
  DatePickerBounds bounds() const { return bounds_; }

  /// Seeds the displayed month; invalid uses value, today, bounds, then 1970.
  void setDisplayedMonth(roo_time::CivilDay day);

  /// Returns the current or most recently displayed month.
  roo_time::CivilDay displayedMonth() const;

  /// Selects calendar or numeric input on the next open.
  void setEntryMode(DatePickerEntryMode mode);

  /// Returns the next session's initial body mode.
  DatePickerEntryMode entryMode() const { return entry_mode_; }

  /// Opens through the owner's shared transient host; never replaces a dialog.
  PresentationStartResult open(Task& owner);

  /// Dismisses without committing. No-op while idle.
  void dismiss(
      DatePickerDismissReason reason = DatePickerDismissReason::kProgrammatic);

  /// Returns whether presentation or admission owns session resources.
  bool isOpen() const { return session_ != nullptr; }

 protected:
  /// Filters otherwise in-range dates. Must not mutate the picker or allocate.
  virtual bool isDateEnabled(roo_time::CivilDay day) const { return true; }

  /// Returns shared labels whose storage outlives the open session.
  virtual const DatePickerStrings& datePickerStrings() const;

  /// Returns a shared allocation-free codec; formatted dates must fit 63 bytes.
  virtual const DateTextCodec& dateTextCodec() const;

  /// Receives a committed date after detachment, instead of onDismissed().
  virtual void onAccepted(roo_time::CivilDay day) {}

  /// Receives cancellation after detachment, instead of onAccepted().
  virtual void onDismissed(DatePickerDismissReason reason) {}

 private:
  friend class internal::DatePickerSession;
  friend class DockedDatePickerField;

  PresentationStartResult openAt(Task& owner, const Rect* anchor);
  void completed(PresentationFinishReason reason);

  ApplicationContext& context_;
  roo_time::CivilDay value_;
  roo_time::CivilDay today_;
  roo_time::CivilDay month_;
  DatePickerBounds bounds_;
  DatePickerEntryMode entry_mode_ = DatePickerEntryMode::kCalendar;
  std::unique_ptr<internal::DatePickerSession> session_;
};

static_assert(sizeof(ModalDatePicker) <= 24 + 3 * sizeof(void*),
              "Closed pickers retain only configuration and a session pointer");
}  // namespace material3
}  // namespace roo_windows
