#include "roo_windows/material2/editor_destination/editor_destination.h"

#include "gtest/gtest.h"
#include "roo_windows/material2/typography.h"
#include "roo_windows/material3/editor_destination/editor_destination.h"
#include "roo_windows/material3/text_field/secure_text_field.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {
class EditorDestinationTest : public testing::TestWithParam<DesignSystem> {
 protected:
  EditorDestinationTest()
      : device_(320, 240, pixels_, {}),
        display_(device_),
        env_(scheduler_, theme_, DefaultKeyboardColorTheme(), GetParam()),
        app_(&env_, display_) {}
  ApplicationContext& context() { return app_.context(); }
  void refresh() { app_.refresh(); }
  const std::string& DraftValue(Widget& field) {
    return GetParam() == DesignSystem::kMaterial2
               ? static_cast<material2::TextField&>(field).content()
               : static_cast<material3::TextField&>(field).text();
  }
  roo::byte pixels_[320 * 240 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Theme theme_ = DefaultTheme();
  Environment env_;
  Application app_;
};

Widget& ClearButton(Destination& destination) {
  auto& pane = static_cast<VerticalLayout&>(destination.getContents());
  return static_cast<HorizontalLayout&>(pane.child_at(0)).child_at(2);
}

// Verifies one lazy destination follows the application default even for a
// borrowed field from the other system, with one keyboard and no overlapping
// edits.
TEST_P(EditorDestinationTest, OneAppStyledDestinationServesBothFieldTypes) {
  EXPECT_EQ(GetParam(), context().defaultDesignSystem());
  EXPECT_EQ(nullptr, context().editorDestinationIfPresent());
  material2::TextField first(context(), material2::font_body1(), "M2",
                             roo_display::kLeft,
                             material2::TextField::UNDERLINE);
  material3::TextField second(context(), "M3");
  Task& t2 = app_.addTaskFullScreen(first);
  Task& t3 = app_.addTaskFullScreen(second);
  first.setContent("old2");
  second.setText("old3");
  refresh();
  Widget* keyboard_parent = app_.keyboard().getContents().parent();
  ASSERT_TRUE(first.editInDestination());
  EditorDestination* destination = context().editorDestinationIfPresent();
  ASSERT_NE(nullptr, destination);
  refresh();
  Widget* draft = t2.textFieldEditor().editedWidget();
  ASSERT_NE(nullptr, draft);
  EXPECT_EQ("old2", DraftValue(*draft));
  if (GetParam() == DesignSystem::kMaterial3) {
    EXPECT_EQ(material3::TextFieldVariant::kFilled,
              static_cast<material3::TextField*>(draft)->variant());
  }
  EXPECT_FALSE(second.editInDestination());
  TextInputEmitter input;
  input.connect(app_);
  t2.textFieldEditor().setSelection(0, 4);
  EXPECT_TRUE(input.commitRune(U'2'));
  EXPECT_TRUE(input.performAction(TextInputAction::kDone));
  EXPECT_EQ("2", first.content());
  ASSERT_TRUE(second.editInDestination());
  refresh();
  EXPECT_EQ(draft, t3.textFieldEditor().editedWidget());
  EXPECT_EQ("old3", DraftValue(*draft));
  EXPECT_FALSE(first.editInDestination());
  t3.textFieldEditor().setSelection(0, 4);
  EXPECT_TRUE(input.commitRune(U'3'));
  EXPECT_TRUE(input.performAction(TextInputAction::kDone));
  EXPECT_EQ("3", second.text());
  EXPECT_EQ(keyboard_parent, app_.keyboard().getContents().parent());
  EXPECT_EQ(destination, context().editorDestinationIfPresent());
  t2.navigation().clear();
  t3.navigation().clear();
}

// Verifies the first visible frame contains the prefilled draft, the row uses
// the full available width, and Back/Clear remain reachable with the keyboard.
TEST_P(EditorDestinationTest, PrefillGeometryAndKeyboardActions) {
  Panel root(context());
  Task& task = app_.addTaskFullScreen(root);
  refresh();
  auto& destination = context().editorDestination();
  ASSERT_TRUE(destination.triggerEdit(task.navigation(), "Workshop controller",
                                      "Name", nullptr));
  refresh();
  Widget* field = task.textFieldEditor().editedWidget();
  ASSERT_NE(nullptr, field);
  EXPECT_EQ("Workshop controller", DraftValue(*field));
  EXPECT_EQ(0, task.textFieldEditor().draw_xoffset());
  auto& pane = static_cast<VerticalLayout&>(destination.getContents());
  auto& row = static_cast<HorizontalLayout&>(pane.child_at(0));
  Widget& back = row.child_at(0);
  Widget& clear = ClearButton(destination);
  const int margin = GetParam() == DesignSystem::kMaterial3 ? Scaled(4) : 0;
  EXPECT_EQ(320 - 2 * margin, row.width());
  EXPECT_EQ(margin, row.offsetLeft());
  EXPECT_EQ(margin, row.offsetTop());
  EXPECT_EQ(Scaled(40), back.width());
  EXPECT_EQ(Scaled(40), clear.width());
  const int gap = Scaled(4);
  EXPECT_EQ(gap, field->offsetLeft() - back.parent_bounds().xMax() - 1);
  EXPECT_EQ(gap, clear.offsetLeft() - field->parent_bounds().xMax() - 1);
  std::vector<roo::byte> initial(std::begin(pixels_), std::end(pixels_));
  task.textFieldEditor().resetMetrics();
  refresh();
  EXPECT_EQ(initial,
            std::vector<roo::byte>(std::begin(pixels_), std::end(pixels_)));
  auto hit = [&](Widget& button) {
    XDim x;
    YDim y;
    button.getAbsoluteOffset(x, y);
    std::vector<Widget*> path;
    EXPECT_TRUE(app_.root().fillTouchTargetPath(x + button.width() / 2,
                                                y + button.height() / 2, path));
    EXPECT_EQ(&button, path.back());
  };
  hit(clear);
  clear.onClicked();
  refresh();
  EXPECT_TRUE(DraftValue(*field).empty());
  EXPECT_FALSE(clear.isVisible());
  EXPECT_EQ(field, task.textFieldEditor().editedWidget());
  EXPECT_TRUE(app_.keyboard().getContents().isVisible());
  EXPECT_EQ(row.width(), 320 - 2 * margin);
  task.textFieldEditor().rune(U'x');
  refresh();
  EXPECT_TRUE(clear.isVisible());
  task.textFieldEditor().del();
  refresh();
  EXPECT_FALSE(clear.isVisible());
  hit(back);
  back.onClicked();
  refresh();
  EXPECT_FALSE(destination.busy());
  EXPECT_FALSE(app_.keyboard().getContents().isVisible());
  task.navigation().clear();
}

// Verifies M2 cancellation preserves the source and clears retained draft text;
// destroying a detached source safely removes the cached destination's borrow.
TEST_P(EditorDestinationTest, M2CancelMaskingAndSourceDestruction) {
  class Form : public Panel {
   public:
    using Panel::add;
    using Panel::Panel;
    using Panel::removeAll;
  } form(context());
  auto source = std::make_unique<material2::TextField>(
      context(), material2::font_body1(), "Password", roo_display::kLeft,
      material2::TextField::UNDERLINE);
  source->setContent(std::string(128, 's'));
  source->setStarred(true);
  form.add(*source);
  Task& task = app_.addTaskFullScreen(form);
  refresh();
  auto& destination = context().editorDestination();
  ASSERT_TRUE(source->editInDestination());
  refresh();
  Widget* draft = task.textFieldEditor().editedWidget();
  ASSERT_NE(nullptr, draft);
  roo_display::GlyphMetrics mask;
  const roo_display::Font& font =
      GetParam() == DesignSystem::kMaterial2
          ? material2::font_body1()
          : material3::text_style_body_large().font();
  font.getGlyphMetrics('*', roo_display::FontLayout::kHorizontal, &mask);
  EXPECT_EQ(mask.advance(), task.textFieldEditor().glyphs().front().advance());
  task.textFieldEditor().rune(U'x');
  task.requestBack();
  EXPECT_EQ(std::string(128, 's'), source->content());
  EXPECT_TRUE(DraftValue(*draft).empty());
  RecordProperty("retained_draft_capacity",
                 std::to_string(DraftValue(*draft).capacity()));
  ASSERT_TRUE(source->editInDestination());
  form.removeAll();
  source.reset();
  task.textFieldEditor().enter();
  EXPECT_FALSE(destination.busy());
  task.navigation().clear();
}

// Verifies navigation superseding admission drops callbacks and draft contents,
// and the same cached destination can subsequently be admitted.
TEST_P(EditorDestinationTest, SupersededAdmissionClearsDraftAndCallback) {
  class Redirect : public Destination {
   public:
    explicit Redirect(ApplicationContext& context) : pane(context) {}
    Widget& getContents() override { return pane; }
    void onPause() override {
      if (redirect) {
        redirect = false;
        getNavigationHost()->clear();
      }
    }
    Panel pane;
    bool redirect = true;
  } root(context());
  Task& task = app_.addTaskFullScreen();
  task.navigation().push(root);
  auto& destination = context().editorDestination();
  int completions = 0;
  EXPECT_FALSE(
      destination.triggerEdit(task.navigation(), "draft", "Name",
                              [&](const std::string&) { ++completions; }));
  EXPECT_FALSE(destination.busy());
  EXPECT_EQ(0, completions);
  task.navigation().push(root);
  ASSERT_TRUE(destination.triggerEdit(task.navigation(), "again", "Name",
                                      [&](const std::string& value) {
                                        EXPECT_EQ("again", value);
                                        ++completions;
                                      }));
  task.textFieldEditor().enter();
  EXPECT_EQ(1, completions);
  task.navigation().clear();
}

// Verifies source completion cannot open another cached extracted editor while
// the first destination is completing, even though it has left navigation.
TEST_P(EditorDestinationTest, CompletionRejectsOverlappingDraft) {
  class Source : public material3::TextField {
   public:
    using TextField::TextField;
    std::function<void()> done;
    void onEditFinished(bool confirmed) override {
      if (confirmed && done != nullptr) done();
    }
  } source(context(), "M3");
  Task& task = app_.addTaskFullScreen(source);
  refresh();
  auto& destination = context().editorDestination();
  source.done = [&]() {
    EXPECT_FALSE(
        destination.triggerEdit(task.navigation(), "blocked", "M2", nullptr));
  };
  ASSERT_TRUE(source.editInDestination());
  task.textFieldEditor().enter();
  EXPECT_FALSE(destination.busy());
  task.navigation().clear();
}

// Verifies either app presentation can commit to an M3 source whose change
// callback destroys it, without a later completion through the stale pointer.
TEST_P(EditorDestinationTest, ValueChangeCanDestroyMixedSource) {
  class Source : public material3::TextField {
   public:
    using TextField::TextField;
    std::function<void()> changed;
    void onTextChanged() override {
      auto callback = changed;
      if (callback != nullptr) callback();
    }
  };
  class Form : public Panel {
   public:
    using Panel::add;
    using Panel::Panel;
    using Panel::removeAll;
  } form(context());
  auto source = std::make_unique<Source>(context(), "Name");
  form.add(*source);
  Task& task = app_.addTaskFullScreen(form);
  refresh();
  source->changed = [&]() {
    form.removeAll();
    source.reset();
  };
  ASSERT_TRUE(source->editInDestination());
  task.textFieldEditor().rune(U'X');
  task.textFieldEditor().enter();
  EXPECT_EQ(nullptr, source);
  EXPECT_FALSE(context().editorDestination().busy());
  task.navigation().clear();
}

// Verifies final external completion may destroy a caller-owned destination;
// navigation and the shared editor must make no later accesses to its controls.
TEST_P(EditorDestinationTest, ExternalCallbackMayDestroyDestination) {
  Panel form(context());
  Task& task = app_.addTaskFullScreen(form);
  std::unique_ptr<EditorDestination> destination;
  if (GetParam() == DesignSystem::kMaterial2) {
    destination = std::make_unique<material2::EditorDestination>(context());
  } else {
    destination = std::make_unique<material3::EditorDestination>(context());
  }
  ASSERT_TRUE(destination->triggerEdit(
      task.navigation(), "value", "Name", [&](const std::string& value) {
        EXPECT_EQ("value", value);
        EXPECT_EQ(nullptr, destination->getNavigationHost());
        destination.reset();
      }));
  task.textFieldEditor().enter();
  EXPECT_EQ(nullptr, destination);
  EXPECT_EQ(1u, task.navigation().depth());
  task.navigation().clear();
}

// Verifies a destination's M3 field stays in place even when its minimum height
// exceeds the available viewport; repeated activation must not nest extraction.
TEST(EditorDestination, TinyViewportNeverExtractsDraftAgain) {
  roo::byte pixels[160 * 64 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(160, 64, pixels,
                                                             {});
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  EXPECT_EQ(DesignSystem::kMaterial3, env.defaultDesignSystem());
  Application app(&env, display);
  material3::TextField source(app.context(), "Name");
  Task& task = app.addTaskFullScreen(source);
  app.refresh();
  source.edit();
  ASSERT_EQ(2u, task.navigation().depth());
  app.refresh();
  auto* draft =
      static_cast<material3::TextField*>(task.textFieldEditor().editedWidget());
  ASSERT_NE(nullptr, draft);
  EXPECT_NE(&source, draft);
  EXPECT_LE(draft->height(), 32);
  draft->edit();
  EXPECT_EQ(2u, task.navigation().depth());
  task.navigation().clear();
}
// Verifies live palette and density changes reach cached controls and that
// dirty painting produces the same image as a full repaint in either style.
TEST_P(EditorDestinationTest, CachedControlsFollowLiveTheme) {
  material3::TextField source(context(), "Device name");
  source.setText(std::string(128, 'a'));
  Task& task = app_.addTaskFullScreen(source);
  refresh();
  ASSERT_TRUE(source.editInDestination());
  refresh();
  Widget* draft = task.textFieldEditor().editedWidget();
  ASSERT_NE(nullptr, draft);
  EditorDestination* destination = context().editorDestinationIfPresent();
  for (bool dark : {false, true}) {
    for (auto density :
         {material3::Density::kDefault, material3::Density::kMinus3}) {
      const Color surface = dark ? Color(0xFF202020) : Color(0xFFF0F0F0);
      const Color content = dark ? Color(0xFFF0F0F0) : Color(0xFF202020);
      theme_.material2Theme().color.surface = surface;
      theme_.material2Theme().color.onSurface = content;
      theme_.material3Theme().color.surface = surface;
      theme_.material3Theme().color.onSurface = content;
      theme_.material3Theme().density = density;
      app_.invalidateTheme(true);
      refresh();
      EXPECT_EQ(destination, context().editorDestinationIfPresent());
      EXPECT_EQ(
          surface,
          static_cast<SurfaceWidget&>(destination->getContents()).background());
      std::vector<roo::byte> partial(std::begin(pixels_), std::end(pixels_));
      app_.root().invalidateDescending();
      refresh();
      EXPECT_EQ(partial,
                std::vector<roo::byte>(std::begin(pixels_), std::end(pixels_)));
    }
  }
  task.requestBack();
  EXPECT_TRUE(DraftValue(*draft).empty());
  RecordProperty("themed_retained_draft_capacity",
                 std::to_string(DraftValue(*draft).capacity()));
  RecordProperty("context_size", std::to_string(sizeof(ApplicationContext)));
  RecordProperty("m2_destination_size",
                 std::to_string(sizeof(material2::EditorDestination)));
  RecordProperty("m3_destination_size",
                 std::to_string(sizeof(material3::EditorDestination)));
  task.navigation().clear();
}

INSTANTIATE_TEST_SUITE_P(AppStyle, EditorDestinationTest,
                         testing::Values(DesignSystem::kMaterial2,
                                         DesignSystem::kMaterial3));

}  // namespace
}  // namespace roo_windows
