#include "roo_windows/keyboard/layout/keyboard_layout.h"

#include <algorithm>
#include <cstring>
#include <vector>

#include "gtest/gtest.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/keyboard/layout/accent_demo.h"
#include "roo_windows/keyboard/layout/en_us.h"
#include "roo_windows/keyboard/layout/pl_pl.h"

namespace roo_windows {
namespace {
using View = KeyboardLayout;

// Verifies the keyboard's default font can display the Polish letter choices.
TEST(KeyboardLayoutTest, DefaultFontCoversPolishLetters) {
  for (char32_t rune : U"ąćęłńóśźżĄĆĘŁŃÓŚŹŻ") {
    if (rune == 0) continue;
    roo_display::GlyphMetrics metrics;
    EXPECT_TRUE(font_body1().getGlyphMetrics(
        rune, roo_display::FontLayout::kHorizontal, &metrics))
        << static_cast<uint32_t>(rune);
  }
}

// Verifies hit/range searches against an independent scan, including gaps.
TEST(KeyboardLayoutTest, SearchesMatchIntervalOracle) {
  for (View view : {accentDemoLayout(), kbEngUSLayout(), kbPolPLLayout()}) {
    for (int p = 0; p < view.pageCount(); ++p) {
      View::Page page;
      ASSERT_TRUE(view.readPage(p, page));
      for (int r = 0; r < page.row_count; ++r) {
        View::Row row;
        ASSERT_TRUE(view.readRow(p, r, row));
        for (int x = -1; x <= page.width; ++x) {
          int expected = -1;
          for (int k = 0; k < row.key_count; ++k) {
            View::Key key;
            ASSERT_TRUE(view.readKey(p, r, k, key));
            if (x >= key.start && x < key.start + key.width) expected = k;
          }
          EXPECT_EQ(expected, view.findKey(p, r, x));
          for (int end = x; end <= page.width + 1; ++end) {
            std::vector<int> expected_range;
            if (x < end) {
              for (int k = 0; k < row.key_count; ++k) {
                View::Key key;
                view.readKey(p, r, k, key);
                if (key.start < end && key.start + key.width > x)
                  expected_range.push_back(k);
              }
            }
            View::KeyRange range = view.findKeyRange(p, r, x, end);
            std::vector<int> actual;
            for (int k = range.first; k < range.past_last; ++k)
              actual.push_back(k);
            EXPECT_EQ(expected_range, actual);
          }
        }
      }
    }
  }
}

// Verifies invalid indices reset outputs and labels cannot partially overwrite.
TEST(KeyboardLayoutTest, CheckedReadsAndAlternatives) {
  View view = accentDemoLayout();
  View::Key key;
  ASSERT_TRUE(view.readKey(0, 0, 1, key));
  EXPECT_EQ(3, key.alternative_count);
  EXPECT_EQ(1, key.alternative_rows);
  EXPECT_EQ(0, key.default_alternative);
  View::Character ch;
  ASSERT_TRUE(view.readAlternative(0, 0, 1, 0, ch));
  EXPECT_EQ(U'é', ch.lower);
  EXPECT_EQ(U'É', ch.upper);
  EXPECT_FALSE(view.readAlternative(0, 0, 1, 3, ch));
  EXPECT_EQ(0u, ch.lower);
  EXPECT_FALSE(view.readKey(-1, 0, 0, key));
  EXPECT_EQ(0, key.width);
  char buffer[] = "untouched";
  size_t length = 9;
  EXPECT_FALSE(kbEngUSLayout().copyLabel(0, 3, 0, buffer, 1, length));
  EXPECT_EQ(0u, length);
  EXPECT_STREQ("untouched", buffer);
  View empty;
  EXPECT_EQ(0, empty.pageCount());
  EXPECT_EQ(-1, empty.findKey(0, 0, 0));
}

// Verifies hostile/truncated binary input is rejected before record access.
TEST(KeyboardLayoutTest, RejectsMalformedBlobs) {
  const std::vector<uint8_t> valid = {'R', 'W', 'K', 'B', 2, 1, 0,   27, 2,
                                      1,   0,   12,  1,   0, 0, 16,  0,  2,
                                      0,   0,   0,   'a', 0, 0, 'A', 0,  0};
  View view;
  ASSERT_EQ(View::Error::kOk, View::Open(valid.data(), valid.size(), view));
  for (size_t n = 0; n < valid.size(); ++n) {
    EXPECT_EQ(View::Error::kInvalidData, View::Open(valid.data(), n, view));
    EXPECT_TRUE(view.empty());
  }
  for (size_t offset :
       {size_t(5), size_t(8), size_t(9), size_t(12), size_t(17)}) {
    std::vector<uint8_t> bad = valid;
    bad[offset] = 0;
    EXPECT_EQ(View::Error::kInvalidData,
              View::Open(bad.data(), bad.size(), view));
  }
  for (size_t offset : {size_t(14), size_t(18), size_t(19), size_t(25)}) {
    std::vector<uint8_t> bad = valid;
    bad[offset] = 255;
    EXPECT_EQ(View::Error::kInvalidData,
              View::Open(bad.data(), bad.size(), view));
  }
  std::vector<uint8_t> bad = valid;
  bad[4] = 1;
  EXPECT_EQ(View::Error::kUnsupportedVersion,
            View::Open(bad.data(), bad.size(), view));
  EXPECT_TRUE(view.empty());
}

// Verifies authored row counts and bottom-row defaults are validated before
// use.
TEST(KeyboardLayoutTest, ValidatesAlternativeGeometry) {
  std::vector<uint8_t> data = {
      'R', 'W', 'K',  'B', 2, 1,    0, 48, 2,    1,   0, 12,
      1,   0,   0,    16,  0, 2,    0, 0,  0,    'e', 0, 0,
      'E', 0,   27,   3,   2, 2,    0, 0,  0xe9, 0,   0, 0xc9,
      0,   0,   0xe8, 0,   0, 0xc8, 0, 1,  0x19, 0,   1, 0x18};
  View view;
  ASSERT_EQ(View::Error::kOk, View::Open(data.data(), data.size(), view));
  View::Key key;
  ASSERT_TRUE(view.readKey(0, 0, 0, key));
  EXPECT_EQ(2, key.alternative_rows);
  EXPECT_EQ(2, key.default_alternative);
  for (int rows : {0, 4}) {
    auto bad = data;
    bad[28] = rows;
    EXPECT_EQ(View::Error::kInvalidData,
              View::Open(bad.data(), bad.size(), view));
  }
  for (int index : {0, 1, 3}) {
    auto bad = data;
    bad[29] = index;
    EXPECT_EQ(View::Error::kInvalidData,
              View::Open(bad.data(), bad.size(), view));
  }
}

// Verifies all 255 row-local indices remain usable without a sentinel
// collision.
TEST(KeyboardLayoutTest, MaximumWidthRowHasNoReservedKeyIndex) {
  std::vector<uint8_t> data(16 + 255 * 11, 0);
  const uint8_t header[] = {'R', 'W', 'K', 'B', 2,   1, 0, 0,
                            255, 1,   0,   12,  255, 0, 0, 16};
  std::copy(std::begin(header), std::end(header), data.begin());
  data[6] = data.size() >> 8;
  data[7] = data.size() & 255;
  for (int k = 0; k < 255; ++k) {
    data[16 + k * 11] = k;
    data[17 + k * 11] = 1;
    data[21 + k * 11] = 'a';
    data[24 + k * 11] = 'A';
  }
  View view;
  ASSERT_EQ(View::Error::kOk, View::Open(data.data(), data.size(), view));
  for (int k = 0; k < 255; ++k) EXPECT_EQ(k, view.findKey(0, 0, k));
  View::KeyRange range = view.findKeyRange(0, 0, 127, 255);
  EXPECT_EQ(127, range.first);
  EXPECT_EQ(255, range.past_last);
  EXPECT_EQ(-1, view.findKey(0, 0, 255));
}

// Verifies flash label validation rejects overlong UTF-8 and invalid payload
// offsets.
TEST(KeyboardLayoutTest, RejectsInvalidLabelAndMenuPayloads) {
  std::vector<uint8_t> data = {'R', 'W', 'K', 'B', 2,  1,  0, 30, 2,    1,
                               0,   12,  1,   0,   0,  16, 0, 2,  5,    0,
                               0,   0,   0,   0,   27, 0,  0, 2,  0xC3, 0xA9};
  View view;
  ASSERT_EQ(View::Error::kOk, View::Open(data.data(), data.size(), view));
  char label[2];
  size_t length;
  ASSERT_TRUE(view.copyLabel(0, 0, 0, label, sizeof(label), length));
  EXPECT_EQ(std::string(u8"é"), std::string(label, length));
  data[28] = 0xC0;
  EXPECT_EQ(View::Error::kInvalidData,
            View::Open(data.data(), data.size(), view));
  data[18] = 0;
  data[24] = 'a';
  data[26] = 27;
  EXPECT_EQ(View::Error::kInvalidData,
            View::Open(data.data(), data.size(), view));
}

}  // namespace
}  // namespace roo_windows
