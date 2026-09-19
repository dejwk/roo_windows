#include "gtest/gtest.h"
#include "roo_windows/config.h"
#include "roo_windows/material3/date_picker/date_picker_types.h"

namespace roo_windows::material3 {
namespace {
using roo_time::CivilDay;

// Verifies inclusive, open-ended and reversed bounds without invalid ordering.
TEST(DatePickerTypes, Bounds) {
  CivilDay day = CivilDay::FromYmd(2024, 2, 29);
  EXPECT_TRUE(DatePickerBounds().contains(day));
  EXPECT_FALSE(DatePickerBounds().contains(CivilDay()));
  EXPECT_TRUE((DatePickerBounds{day, day}.contains(day)));
  EXPECT_FALSE((DatePickerBounds{day, day}.contains(day.addDays(1))));
  EXPECT_FALSE((DatePickerBounds{day, day.addDays(-1)}.isValid()));
}

// Verifies leap-month alignment, weekday rotation and civil-range edges.
TEST(DatePickerTypes, CalendarGeometry) {
  CivilDay march = CivilDay::FromYmd(2024, 3, 1);
  EXPECT_EQ(CivilDay::FromYmd(2024, 2, 25),
            internal::GridDay(march, roo_time::kSunday, 0));
  EXPECT_EQ(CivilDay::FromYmd(2024, 2, 26),
            internal::GridDay(march, roo_time::kMonday, 0));
  EXPECT_EQ(CivilDay::FromYmd(2024, 2, 1), internal::ShiftMonth(march, -1));
  EXPECT_FALSE(internal::ShiftMonth(CivilDay::FromYmd(1, 1, 1), -1).isValid());
  EXPECT_FALSE(
      internal::ShiftMonth(CivilDay::FromYmd(9999, 12, 1), 1).isValid());
  EXPECT_FALSE(internal::GridDay(march, roo_time::kSunday, 42).isValid());
}

// Verifies shared codec round-trip, strict parsing and bounded output.
TEST(DatePickerTypes, NumericCodec) {
  const DateTextCodec& codec = DefaultDateTextCodec();
  CivilDay day = CivilDay::FromYmd(2024, 2, 29);
  char text[11];
  EXPECT_EQ(roo_time::TextStatus::kOk,
            codec.format(day, text, sizeof(text)).status);
#if ROO_WINDOWS_LANG == ROO_LANG_pl
  EXPECT_STREQ("29.02.2024", text);
  EXPECT_EQ(roo_time::kMonday, DefaultDatePickerStrings().first_weekday);
#else
  EXPECT_STREQ("02/29/2024", text);
  EXPECT_EQ(roo_time::kSunday, DefaultDatePickerStrings().first_weekday);
#endif
  CivilDay parsed;
  EXPECT_EQ(roo_time::TextStatus::kOk, codec.parse(text, parsed).status);
  EXPECT_EQ(day, parsed);
  EXPECT_NE(roo_time::TextStatus::kOk, codec.parse("invalid", parsed).status);
  EXPECT_EQ(day, parsed);
  char tiny[2];
  EXPECT_EQ(roo_time::TextStatus::kBufferTooSmall,
            codec.format(day, tiny, sizeof(tiny)).status);
  EXPECT_EQ('\0', tiny[1]);
}
}  // namespace
}  // namespace roo_windows::material3
