#pragma once

#include "roo_time/civil_day.h"
#include "roo_time/format.h"

namespace roo_windows::material3 {

/// Inclusive selectable interval; invalid endpoints are unbounded.
struct DatePickerBounds {
  roo_time::CivilDay first;
  roo_time::CivilDay last;

  /// Reports whether the interval is ordered.
  bool isValid() const;

  /// Rejects invalid dates and reversed intervals.
  bool contains(roo_time::CivilDay day) const;
};

/// Initial presentation of a modal picker.
enum class DatePickerEntryMode : uint8_t { kCalendar, kInput };

/// Terminal reason for a dismissed picker; acceptance has its own hook.
enum class DatePickerDismissReason : uint8_t {
  kCancel,
  kOutsideTap,
  kBack,
  kProgrammatic,
  kOwnerUnavailable,
};

/// Shared localized labels. Custom tables must outlive the picker session.
struct DatePickerStrings {
  roo_time::DayOfWeek first_weekday;
  roo::string_view headline_select_date;
  roo::string_view headline_input_date;
  roo::string_view ok_label;
  roo::string_view cancel_label;
  roo::string_view invalid_date;
  roo::string_view month_names[12];
  roo::string_view weekday_narrow[7];
};

/// Allocation-free date-only conversion; custom instances must outlive use.
class DateTextCodec {
 public:
  virtual ~DateTextCodec() = default;

  /// Parses complete input, preserving out on failure.
  virtual roo_time::ParseResult parse(roo::string_view text,
                                      roo_time::CivilDay& out) const = 0;

  /// Formats a date into a bounded buffer, including a terminating NUL.
  virtual roo_time::FormatResult format(roo_time::CivilDay day, char* out,
                                        size_t capacity) const = 0;

  /// Returns a static hint for numeric input.
  virtual roo::string_view placeholder() const = 0;
};

/// Returns the build-language labels (English or Polish).
const DatePickerStrings& DefaultDatePickerStrings();

/// Returns the build-language strict numeric codec.
const DateTextCodec& DefaultDateTextCodec();

namespace internal {

/// Returns the first day of a valid date's month, or invalid.
roo_time::CivilDay MonthStart(roo_time::CivilDay day);

/// Moves a month start without overflowing the supported civil range.
roo_time::CivilDay ShiftMonth(roo_time::CivilDay day, int delta);

/// Returns the date at a 7-by-6 calendar cell, including adjacent months.
roo_time::CivilDay GridDay(roo_time::CivilDay month,
                           roo_time::DayOfWeek first_weekday, int cell);

}  // namespace internal
}  // namespace roo_windows::material3
