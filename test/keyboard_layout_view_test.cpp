#include "roo_windows/keyboard_layout/keyboard_layout_view.h"

#include <algorithm>
#include <cstring>
#include <vector>

#include "gtest/gtest.h"
#include "roo_windows/keyboard_layout/accent_demo.h"
#include "roo_windows/keyboard_layout/en_us.h"
#include "roo_windows/keyboard_layout/en_us_binary.h"
#include "roo_windows/keyboard_layout/pl_pl.h"

namespace roo_windows {
namespace {
using View = KeyboardLayoutView;

// Verifies every original US key, uppercase value, label, width, and page
// target.
TEST(KeyboardLayoutViewTest, CapturesLegacyUSExactly) {
  const KeyboardSpec* old = kbEngUS();
  View view = kbEngUSLayout();
  ASSERT_EQ(old->page_count, view.pageCount());
  for (int p = 0; p < old->page_count; ++p) {
    View::Page page;
    ASSERT_TRUE(view.readPage(p, page));
    EXPECT_EQ(old->pages[p].row_width, page.width);
    EXPECT_EQ(old->pages[p].row_count, page.row_count);
    for (int r = 0; r < page.row_count; ++r) {
      const KeyboardRowSpec& row = old->pages[p].rows[r];
      View::Row decoded;
      ASSERT_TRUE(view.readRow(p, r, decoded));
      ASSERT_EQ(row.key_count, decoded.key_count);
      int start = row.start_offset;
      for (int k = 0; k < row.key_count; ++k) {
        View::Key key;
        ASSERT_TRUE(view.readKey(p, r, k, key));
        EXPECT_EQ(start, key.start);
        EXPECT_EQ(row.keys[k].width, key.width);
        EXPECT_EQ(static_cast<int>(row.keys[k].function),
                  static_cast<int>(key.function));
        if (key.function == View::Function::kText) {
          EXPECT_EQ(row.keys[k].data, key.character.lower);
          EXPECT_EQ(row.keys_caps[k].data, key.character.upper);
        } else if (key.function == View::Function::kSwitchPage) {
          uint32_t data = row.keys[k].data;
          EXPECT_EQ(data & 255, key.target_page);
          char label[255];
          size_t length;
          ASSERT_TRUE(view.copyLabel(p, r, k, label, sizeof(label), length));
          EXPECT_EQ(data >> 16, length);
          EXPECT_EQ(0,
                    std::memcmp(label,
                                row.pageswitch_key_labels + ((data >> 8) & 255),
                                length));
        }
        start += key.width;
      }
    }
  }
}

// Verifies hit/range searches against an independent scan, including gaps.
TEST(KeyboardLayoutViewTest, SearchesMatchIntervalOracle) {
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
TEST(KeyboardLayoutViewTest, CheckedReadsAndAlternatives) {
  View view = accentDemoLayout();
  View::Key key;
  ASSERT_TRUE(view.readKey(0, 0, 1, key));
  EXPECT_EQ(3, key.alternative_count);
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
TEST(KeyboardLayoutViewTest, RejectsMalformedBlobs) {
  const std::vector<uint8_t> valid = {'R', 'W', 'K', 'B', 1, 1, 0,   27, 2,
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
  bad[4] = 2;
  EXPECT_EQ(View::Error::kUnsupportedVersion,
            View::Open(bad.data(), bad.size(), view));
  EXPECT_TRUE(view.empty());
}

}  // namespace
}  // namespace roo_windows
