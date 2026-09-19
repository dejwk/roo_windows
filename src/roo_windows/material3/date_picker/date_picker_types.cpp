#include "roo_windows/material3/date_picker/date_picker_types.h"

#include "roo_windows/config.h"

namespace roo_windows::material3 {
using roo_time::CivilDay;

bool DatePickerBounds::isValid() const {
  return !first.isValid() || !last.isValid() || first <= last;
}

bool DatePickerBounds::contains(CivilDay day) const {
  return day.isValid() && isValid() && (!first.isValid() || day >= first) &&
         (!last.isValid() || day <= last);
}

namespace {
#if ROO_WINDOWS_LANG == ROO_LANG_pl
const DatePickerStrings kStrings{
    roo_time::kMonday,
    "Wybierz datę",
    "Wpisz datę",
    "OK",
    "Anuluj",
    "Nieprawidłowa data",
    {"Styczeń", "Luty", "Marzec", "Kwiecień", "Maj", "Czerwiec", "Lipiec",
     "Sierpień", "Wrzesień", "Październik", "Listopad", "Grudzień"},
    {"N", "P", "W", "Ś", "C", "P", "S"}};
constexpr char kFormat[] = "%d.%m.%Y";
constexpr char kPlaceholder[] = "DD.MM.RRRR";
#else
const DatePickerStrings kStrings{
    roo_time::kSunday,
    "Select date",
    "Enter date",
    "OK",
    "Cancel",
    "Invalid date",
    {"January", "February", "March", "April", "May", "June", "July", "August",
     "September", "October", "November", "December"},
    {"S", "M", "T", "W", "T", "F", "S"}};
constexpr char kFormat[] = "%m/%d/%Y";
constexpr char kPlaceholder[] = "MM/DD/YYYY";
#endif

/// Shared build-language codec backed by allocation-free civil-date APIs.
class NumericCodec final : public DateTextCodec {
 public:
  /// Parses a complete numeric date without changing output on failure.
  roo_time::ParseResult parse(roo::string_view text,
                              CivilDay& out) const override {
    return roo_time::ParseCivilDay(text, kFormat, &out);
  }

  /// Formats the date into caller-owned bounded storage.
  roo_time::FormatResult format(CivilDay day, char* out,
                                size_t capacity) const override {
    return roo_time::FormatCivilDay(day, kFormat, out, capacity);
  }

  /// Returns the static build-language numeric format hint.
  roo::string_view placeholder() const override { return kPlaceholder; }
};

const NumericCodec kCodec;
}  // namespace

const DatePickerStrings& DefaultDatePickerStrings() { return kStrings; }

const DateTextCodec& DefaultDateTextCodec() { return kCodec; }

namespace internal {
CivilDay MonthStart(CivilDay day) {
  return day.isValid() ? CivilDay::FromYmd(day.year(), day.month(), 1)
                       : CivilDay::Invalid();
}

CivilDay ShiftMonth(CivilDay day, int delta) {
  if (!day.isValid()) return CivilDay::Invalid();
  int64_t index = (day.year() - 1) * 12 + day.month() - 1;
  index += delta;
  if (index < 0 || index >= 9999 * 12) return CivilDay::Invalid();
  return CivilDay::FromYmd(index / 12 + 1, index % 12 + 1, 1);
}

CivilDay GridDay(CivilDay month, roo_time::DayOfWeek first_weekday, int cell) {
  month = MonthStart(month);
  if (!month.isValid() || cell < 0 || cell >= 42) return CivilDay::Invalid();
  int offset = (7 + month.dayOfWeek() - first_weekday) % 7;
  return month.addDays(cell - offset);
}
}  // namespace internal
}  // namespace roo_windows::material3
