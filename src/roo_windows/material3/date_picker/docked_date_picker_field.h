#pragma once

#include <memory>

#include "roo_windows/material3/date_picker/date_picker.h"
#include "roo_windows/material3/text_field/text_field.h"

namespace roo_windows::material3 {

/// Editable date field whose open calendar exclusively owns focus and input.
/// Calendar state is allocated only while open. Setters require a closed
/// calendar. Typed dates commit on edit confirmation; invalid text is retained
/// with an error. Calendar cancellation preserves the pre-open text and date.
class DockedDatePickerField : public TextField {
 public:
  /// Creates an empty outlined field with a trailing calendar affordance.
  explicit DockedDatePickerField(
      ApplicationContext& context, roo::string_view label,
      TextFieldVariant variant = TextFieldVariant::kOutlined);

  /// Silently cancels an open calendar before the field is destroyed.
  ~DockedDatePickerField() override;

  /// Sets the committed date and updates its formatted text.
  void setDate(roo_time::CivilDay day);

  /// Returns the committed civil date, independent of unconfirmed text.
  roo_time::CivilDay date() const { return date_; }

  /// Sets the explicit today marker for the next session.
  void setToday(roo_time::CivilDay day);

  /// Returns the explicit today marker.
  roo_time::CivilDay today() const { return today_; }

  /// Sets inclusive bounds; reversed intervals reject opening.
  void setBounds(DatePickerBounds bounds);

  /// Returns the configured interval.
  DatePickerBounds bounds() const { return bounds_; }

  /// Seeds the next calendar month, or invalid for automatic selection.
  void setDisplayedMonth(roo_time::CivilDay day);

  /// Returns the current or most recently displayed month.
  roo_time::CivilDay displayedMonth() const;

  /// Captures anchor geometry, opens below the field, or promotes to modal.
  PresentationStartResult openPicker();

  /// Closes the calendar without committing its draft.
  void dismissPicker(
      DatePickerDismissReason reason = DatePickerDismissReason::kProgrammatic);

  /// Returns whether a calendar session or its admission is active.
  bool isPickerOpen() const { return picker_ != nullptr; }

  /// Opens the calendar on field activation.
  void onClicked() override;

  /// Opens on fresh focus; restoration from the calendar never reopens it.
  void onFocusChanged(bool focused) override;

  /// Ends a presentation whose field is detached or hidden.
  void onPresentationChanged(const PresentationChange& change) override;

 protected:
  /// Filters otherwise in-range dates without mutating the field.
  virtual bool isDateEnabled(roo_time::CivilDay day) const { return true; }

  /// Returns shared labels that outlive each calendar session.
  virtual const DatePickerStrings& datePickerStrings() const;

  /// Returns a shared codec whose output fits 63 bytes.
  virtual const DateTextCodec& dateTextCodec() const;

  /// Receives a committed typed or calendar date. May destroy this field.
  virtual void onAccepted(roo_time::CivilDay day) {}

  /// Receives calendar cancellation after detachment. May destroy this field.
  virtual void onDismissed(DatePickerDismissReason reason) {}

  /// Validates uncommitted text with the shared date policy.
  void onTextChanged() override;
  /// Commits valid text on explicit edit confirmation while the picker is idle.
  void onEditFinished(bool confirmed) override;

 private:
  class Picker;
  bool parseText(roo_time::CivilDay& day) const;
  void completed(bool accepted, DatePickerDismissReason reason);

  roo_time::CivilDay date_;
  roo_time::CivilDay today_;
  roo_time::CivilDay month_;
  DatePickerBounds bounds_;
  std::unique_ptr<Picker> picker_;
  bool changing_text_ = false;
};

static_assert(sizeof(DockedDatePickerField) <=
                  sizeof(TextField) + 24 + 2 * sizeof(void*),
              "Closed fields must not retain calendar widgets or draft text");
}  // namespace roo_windows::material3
