#include <memory>

#include "gtest/gtest.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {

using roo_display::Color;
using test_support::QuantizeToArgb4444;
using test_support::RooWindowsRenderTestSized;

class KeyboardPresentationPinTest : public RooWindowsRenderTestSized<160, 128> {
};

// Verifies a text-key preview is an active-only popup pin that escapes the
// keyboard task without creating a focus or presentation owner.
TEST_F(KeyboardPresentationPinTest,
       PreviewEscapesKeyboardTaskAndPreservesPopupFocus) {
  static_assert(sizeof(Keyboard) == sizeof(std::unique_ptr<Widget>) +
                                        sizeof(TextInputEmitter) +
                                        sizeof(Task*),
                "Keyboard must retain no persistent preview storage");

  Keyboard& keyboard = app_.keyboard();
  keyboard.show();
  ASSERT_TRUE(refresh());

  Panel& keyboard_contents = static_cast<Panel&>(keyboard.getContents());
  Panel& letter_page = static_cast<Panel&>(keyboard_contents.child_at(0));
  Widget& key = letter_page.child_at(0);
  EXPECT_EQ(ParentClipMode::kClipped, keyboard_contents.getParentClipMode());
  EXPECT_EQ(ParentClipMode::kClipped, letter_page.getParentClipMode());
  Task* const owner = keyboard.getContents().getTask();
  ASSERT_NE(nullptr, owner);
  EXPECT_EQ(owner, letter_page.getTask());
  ASSERT_TRUE(key.requestFocus());
  EXPECT_EQ(&key, owner->focus().focused());

  XDim page_dx;
  YDim page_dy;
  letter_page.getAbsoluteOffset(page_dx, page_dy);
  const Rect& key_bounds = key.parent_bounds();
  const XDim preview_center_x =
      (key_bounds.xMin() + key_bounds.xMax()) / 2 + page_dx;
  const YDim preview_center_y = key_bounds.yMin() + page_dy - 4 - 24;
  XDim keyboard_dx;
  YDim keyboard_dy;
  keyboard_contents.getAbsoluteOffset(keyboard_dx, keyboard_dy);
  ASSERT_LT(preview_center_y, keyboard_dy);
  ASSERT_GE(preview_center_x, 0);
  ASSERT_GE(preview_center_y, 0);

  key.onShowPress(0, 0);
  EXPECT_TRUE(letter_page.hasPresentationPin());
  EXPECT_EQ(owner, keyboard.getContents().getTask());
  EXPECT_EQ(&key, owner->focus().focused());
  ASSERT_TRUE(refresh());

  EXPECT_EQ(QuantizeToArgb4444(static_cast<Button&>(key).background()),
            pixelAt(preview_center_x, preview_center_y));

  key.onCancel();
  EXPECT_FALSE(letter_page.hasPresentationPin());
  EXPECT_EQ(&key, owner->focus().focused());
}

}  // namespace
}  // namespace roo_windows
