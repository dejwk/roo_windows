#include <limits>
#include <type_traits>

#include "examples/material3/theme/density/density_settings.h"
#include "gtest/gtest.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/checkbox/checkbox.h"
#include "roo_windows/material3/date_picker/date_picker_internal.h"
#include "roo_windows/material3/dialog/basic_dialog.h"
#include "roo_windows/material3/list/dynamic_list.h"
#include "roo_windows/material3/radio_button/radio_button.h"
#include "roo_windows/material3/text_field/text_field.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows::material3 {
namespace {
constexpr Density kLevels[] = {Density::kDefault, Density::kMinus1,
                               Density::kMinus2,  Density::kMinus3,
                               Density::kMinus4,  Density::kMinus5};

class DensityTest : public testing::Test {
 protected:
  DensityTest()
      : offscreen_(240, 320, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        material_(DefaultTheme().material3Theme()),
        theme_{MakeFrameworkTheme(material_), &material_},
        env_(scheduler_, theme_),
        app_(&env_, display_) {}
  ApplicationContext& context() { return app_.context(); }
  void refresh() { app_.refresh(); }
  void change(Density density) {
    material_.density = density;
    app_.root().requestLayoutDescending();
    app_.root().invalidateDescending();
    refresh();
  }
  roo_display::Color pixelAt(int x, int y) {
    int16_t xs[] = {static_cast<int16_t>(x)};
    int16_t ys[] = {static_cast<int16_t>(y)};
    roo_display::Color result[1];
    offscreen_.raster().readColors(xs, ys, 1, result);
    return result[0];
  }
  roo::byte raster_[240 * 320 * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Material3Theme material_;
  Theme theme_;
  Environment env_;
  Application app_;
};

// Verifies the public representation, explicit default, and existing aggregate
// initialization with omitted trailing fields.
TEST_F(DensityTest, NamedLevelsAndAggregateCompatibility) {
  static_assert(sizeof(Density) == 1);
  static_assert(std::is_same<std::underlying_type_t<Density>, int8_t>::value);
  EXPECT_EQ(DefaultTheme().material3Theme().density, Density::kDefault);
  Material3Theme legacy{material_.color, material_.state};
  EXPECT_EQ(legacy.density, Density::kDefault);
  Theme copy = theme_;
  Button button(context(), "Save");
  TextField filled(context(), "Name");
  TextField outlined(context(), "Name", TextFieldVariant::kOutlined);
  StandardListItem item(StandardListItemInit::OneLine("Row"));
  ListEntry row(context());
  row.setItem(item);
  Checkbox checkbox(context());
  RadioButton radio(context());
  const Dimensions checkbox_size = checkbox.getNaturalDimensions();
  const Dimensions radio_size = radio.getNaturalDimensions();
  const int button_heights[] = {40, 36, 32, 28, 28, 28};
  const int filled_heights[] = {56, 52, 48, 48, 48, 48};
  const int outlined_heights[] = {64, 60, 56, 52, 48, 44};
  const int row_heights[] = {56, 52, 48, 44, 40, 36};
  for (int step = 0; step <= 5; ++step) {
    material_.density = kLevels[step];
    EXPECT_EQ(static_cast<int>(kLevels[step]), -step);
    EXPECT_EQ(copy.material3Theme().density, kLevels[step]);
    EXPECT_EQ(checkbox.getNaturalDimensions().width(), checkbox_size.width());
    EXPECT_EQ(checkbox.getNaturalDimensions().height(), checkbox_size.height());
    EXPECT_EQ(radio.getNaturalDimensions().width(), radio_size.width());
    EXPECT_EQ(radio.getNaturalDimensions().height(), radio_size.height());
    EXPECT_EQ(button.getNaturalDimensions().height(),
              Scaled(button_heights[step]));
    EXPECT_EQ(filled.getSuggestedMinimumDimensions().height(),
              Scaled(filled_heights[step]));
    EXPECT_EQ(outlined.getSuggestedMinimumDimensions().height(),
              Scaled(outlined_heights[step]));
    EXPECT_EQ(row.measure(WidthSpec::Exactly(240), HeightSpec::Unspecified(0))
                  .height(),
              Scaled(row_heights[step]));
  }
}

// Verifies invalid casts assert in debug consumers and produce default geometry
// in release consumers, including measured list rows.
TEST_F(DensityTest, InvalidEnums) {
  Button button(context(), "Save");
  TextField field(context(), "Name");
  StandardListItem item(StandardListItemInit::OneLine("Row"));
  ListEntry row(context());
  row.setItem(item);
  for (int value : {-128, -6, 1, 127}) {
    material_.density = static_cast<Density>(value);
#ifndef NDEBUG
    EXPECT_DEATH(button.getNaturalDimensions(), "invalid Material 3 density");
    EXPECT_DEATH(field.getSuggestedMinimumDimensions(),
                 "invalid Material 3 density");
    EXPECT_DEATH(row.getSuggestedMinimumDimensions(),
                 "invalid Material 3 density");
#else
    EXPECT_EQ(button.getNaturalDimensions().height(), Scaled(40));
    EXPECT_EQ(field.getSuggestedMinimumDimensions().height(), Scaled(56));
    EXPECT_EQ(row.getSuggestedMinimumDimensions().height(), Scaled(56));
    EXPECT_EQ(row.measure(WidthSpec::Exactly(240), HeightSpec::Unspecified(0))
                  .height(),
              Scaled(56));
#endif
  }
}

// Verifies external integers are validated before casting and invalid settings
// preserve the current shared choice.
TEST_F(DensityTest, ExternalSettingsValidation) {
  material_.density = Density::kMinus2;
  for (int value : {std::numeric_limits<int>::min(), -6, 1, 256,
                    std::numeric_limits<int>::max()}) {
    EXPECT_FALSE(density_example::ApplySetting(value, material_, app_.root()));
    EXPECT_EQ(material_.density, Density::kMinus2);
  }
  for (int value = -5; value <= 0; ++value) {
    EXPECT_TRUE(density_example::ApplySetting(value, material_, app_.root()));
    EXPECT_EQ(static_cast<int>(material_.density), value);
  }
}

// Verifies real layout, exact hits, and vacated pixels against a full redraw.
TEST_F(DensityTest, RuntimeBoundsAndVacatedPixels) {
  Button button(context(), "Save");
  VerticalLayout form(context());
  form.add(button);
  Task& task = app_.addTaskFullScreen(form);
  refresh();
  for (Density level :
       {Density::kMinus2, Density::kMinus5, Density::kDefault}) {
    change(level);
    int height = Scaled(level == Density::kDefault  ? 40
                        : level == Density::kMinus2 ? 32
                                                    : 28);
    EXPECT_EQ(button.height(), height);
    std::vector<Widget*> path;
    EXPECT_TRUE(
        button.fillTouchTargetPath(button.width() / 2, height - 1, path));
    path.clear();
    EXPECT_FALSE(button.fillTouchTargetPath(button.width() / 2, height, path));
    if (level != Density::kDefault) {
      XDim dx = 0;
      YDim dy = 0;
      button.getAbsoluteOffset(dx, dy);
      EXPECT_EQ(pixelAt(dx + button.width() / 2, dy + Scaled(39)),
                test_support::QuantizeToArgb4444(form.background()));
    }
    std::vector<roo_display::Color> incremental;
    for (int y = 0; y < Scaled(42); ++y)
      incremental.push_back(pixelAt(button.width() / 2, y));
    app_.root().invalidateDescending();
    refresh();
    for (int y = 0; y < Scaled(42); ++y)
      EXPECT_EQ(incremental[y], pixelAt(button.width() / 2, y));
  }
  task.navigation().clear();
}

// Verifies both public root invalidation overloads register display damage
// while a retained field's allocated bounds stay fixed.
TEST_F(DensityTest, FixedBoundsFieldRepaints) {
  TextField field(context(), "Name");
  field.setText("value");
  Task& task = app_.addTask(field, roo_display::Box(0, 0, 239, 119));
  refresh();
  const Rect bounds = field.bounds();
  std::vector<roo_display::Color> original;
  for (int y = 0; y < 120; ++y)
    for (int x = 0; x < 240; ++x) original.push_back(pixelAt(x, y));
  change(Density::kMinus5);
  EXPECT_EQ(field.bounds(), bounds);
  std::vector<roo_display::Color> compact;
  for (int y = 0; y < 120; ++y)
    for (int x = 0; x < 240; ++x) compact.push_back(pixelAt(x, y));
  EXPECT_NE(compact, original);
  material_.density = Density::kDefault;
  app_.root().invalidateDescending(Rect(0, 0, 239, 119));
  refresh();
  for (int y = 0; y < 120; ++y)
    for (int x = 0; x < 240; ++x)
      EXPECT_EQ(pixelAt(x, y), original[y * 240 + x]);
  task.navigation().clear();
}

// Verifies pinned field density controls both measurement and painting while
// ordinary fields still inherit live theme changes.
TEST_F(DensityTest, TextFieldDensityOverride) {
  class CompactField : public TextField {
   public:
    using TextField::TextField;
    DensityOverride densityOverride() const override {
      return DensityOverride::Explicit(Density::kMinus5);
    }
  };
  TextField inherited(context(), "Name");
  EXPECT_TRUE(inherited.densityOverride().isInherited());
  for (TextFieldVariant variant :
       {TextFieldVariant::kFilled, TextFieldVariant::kOutlined}) {
    CompactField field(context(), "Name", variant);
    field.setText("value");
    Task& task = app_.addTask(field, roo_display::Box(0, 0, 239, 119));
    change(Density::kDefault);
    EXPECT_EQ(field.getSuggestedMinimumDimensions().height(),
              Scaled(variant == TextFieldVariant::kFilled ? 48 : 44));
    EXPECT_EQ(inherited.getSuggestedMinimumDimensions().height(), Scaled(56));
    std::vector<roo_display::Color> original;
    for (int y = 0; y < 120; ++y)
      for (int x = 0; x < 240; ++x) original.push_back(pixelAt(x, y));
    change(Density::kMinus5);
    EXPECT_EQ(field.getSuggestedMinimumDimensions().height(),
              Scaled(variant == TextFieldVariant::kFilled ? 48 : 44));
    EXPECT_EQ(inherited.getSuggestedMinimumDimensions().height(), Scaled(48));
    for (int y = 0; y < 120; ++y)
      for (int x = 0; x < 240; ++x)
        EXPECT_EQ(pixelAt(x, y), original[y * 240 + x]);
    task.navigation().clear();
  }
}

// Verifies active hardware editing retains text, selection, and focus while
// the source field follows the shared live theme.
TEST_F(DensityTest, ActiveInPlaceEditor) {
  TextField field(context(), "Name");
  VerticalLayout form(context());
  form.add(field);
  Task& task = app_.addTaskFullScreen(form);
  refresh();
  ASSERT_TRUE(field.requestFocus());
  field.setText("draft");
  ASSERT_TRUE(
      field.onKeyEvent(KeyEvent(KeyPhase::kDown, KeyCode::kEnter, 0, 0)));
  ASSERT_TRUE(field.isEdited());
  task.textFieldEditor().setSelection(1, 4);
  for (Density level :
       {Density::kMinus2, Density::kMinus5, Density::kDefault}) {
    change(level);
    EXPECT_TRUE(field.isEdited());
    EXPECT_TRUE(field.isFocused());
    EXPECT_EQ(task.textFieldEditor().editedWidget(), &field);
    EXPECT_EQ(field.text(), "draft");
    EXPECT_EQ(field.height(), Scaled(level == Density::kDefault ? 56 : 48));
  }
  task.textFieldEditor().rune(U'X');
  EXPECT_EQ(field.text(), "dXt");
  task.textFieldEditor().enter();
  task.navigation().clear();
}

// Verifies open dialog bodies and action buttons inherit live theme density.
TEST_F(DensityTest, OpenDialogBody) {
  VerticalLayout home(context());
  Task& task = app_.addTaskFullScreen(home);
  TextField field(context(), "Name");
  Button button(context(), "Apply");
  VerticalLayout body(context());
  body.add(field);
  body.add(button);
  DialogActionSpec actions[] = {{1, "OK", DialogActionRole::kAcknowledge}};
  BasicDialog dialog(context(), body, actions, 1);
  ASSERT_EQ(dialog.show(task), DialogShowResult::kShown);
  refresh();
  auto* strip = static_cast<internal::DialogActionStrip*>(
      static_cast<Widget&>(dialog).focusChildAt(4));
  ASSERT_NE(strip, nullptr);
  Widget& action = strip->actionButton(0);
  for (Density level :
       {Density::kMinus2, Density::kMinus5, Density::kDefault}) {
    change(level);
    EXPECT_TRUE(dialog.isShowing());
    EXPECT_EQ(action.height(), Scaled(level == Density::kDefault  ? 40
                                      : level == Density::kMinus2 ? 32
                                                                  : 28));
    EXPECT_EQ(field.height(), Scaled(level == Density::kDefault ? 56 : 48));
    EXPECT_EQ(button.height(), Scaled(level == Density::kDefault  ? 40
                                      : level == Density::kMinus2 ? 32
                                                                  : 28));
  }
  dialog.dismiss();
  task.navigation().clear();
}

/// Exposes fixed placement to trigger the existing full-screen extraction.
class ExtractionForm : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
};

// Verifies density preserves the Material 2 extraction destination and the
// returning Material 3 source sees the live setting without losing its draft.
TEST_F(DensityTest, ExtractedEditorAndReturningSource) {
  TextField field(context(), "Name");
  ExtractionForm form(context());
  form.add(field, Rect(0, 0, 239, 199));
  Task& task = app_.addTaskFullScreen(form);
  refresh();
  field.setText("old");
  field.edit();
  ASSERT_EQ(task.navigation().depth(), 2u);
  refresh();
  Widget* editor = task.textFieldEditor().editedWidget();
  ASSERT_NE(editor, nullptr);
  ASSERT_NE(editor, &field);
  Rect original = editor->bounds();
  for (Density level :
       {Density::kMinus2, Density::kMinus5, Density::kDefault}) {
    change(level);
    EXPECT_EQ(task.navigation().depth(), 2u);
    EXPECT_EQ(task.textFieldEditor().editedWidget(), editor);
    EXPECT_EQ(editor->bounds(), original);
  }
  change(Density::kMinus5);
  task.textFieldEditor().setSelection(0, 3);
  task.textFieldEditor().rune(U'X');
  task.textFieldEditor().enter();
  refresh();
  EXPECT_EQ(field.text(), "X");
  EXPECT_EQ(field.getSuggestedMinimumDimensions().height(), Scaled(48));
  EXPECT_EQ(task.navigation().depth(), 1u);
  task.navigation().clear();
}

// Verifies reused date-picker numeric input follows density while its chrome
// retains presentation geometry and the session retains its draft.
TEST_F(DensityTest, OpenDatePickerInput) {
  VerticalLayout home(context());
  Task& task = app_.addTaskFullScreen(home);
  ModalDatePicker picker(context());
  const roo_time::CivilDay date = roo_time::CivilDay::FromYmd(2024, 2, 29);
  picker.setValue(date);
  picker.setEntryMode(DatePickerEntryMode::kInput);
  ASSERT_EQ(picker.open(task), PresentationStartResult::kStarted);
  refresh();
  auto* panel =
      static_cast<internal::DatePickerPanel*>(task.focus().scopeRoot());
  TextField* input = panel->input();
  ASSERT_NE(input, nullptr);
  const std::string text = input->text();
  const Rect chrome = panel->bounds();
  for (Density level :
       {Density::kMinus2, Density::kMinus5, Density::kDefault}) {
    change(level);
    EXPECT_TRUE(picker.isOpen());
    EXPECT_EQ(panel->input(), input);
    EXPECT_EQ(input->getSuggestedMinimumDimensions().height(),
              Scaled(level == Density::kDefault  ? 64
                     : level == Density::kMinus2 ? 56
                                                 : 44));
    EXPECT_EQ(input->text(), text);
    EXPECT_EQ(panel->bounds(), chrome);
    EXPECT_EQ(panel->session().draft, date);
  }
  picker.dismiss();
  task.navigation().clear();
}

using ProductionRow = ListRow<InvokableListItemBase>;
class DensityModel : public DynamicListModel<ProductionRow> {
 public:
  int elementCount() const override { return 20; }
  mutable int releases = 0;
  void prepare(ProductionRow& row) const override {
    row.item().setHeadline("Row");
  }
  void bind(int index, ProductionRow& row) const override {
    row.item().setHeadline("Row");
    row.item().setOnInvoked([]() {});
  }
  void unbind(ProductionRow&) const override { ++releases; }
  DynamicListSectionState sectionState() const override {
    return {true, DynamicListFocusTarget::kRowSurface};
  }
};

class DensitySection : public DynamicList<ProductionRow> {
 public:
  using DynamicList<ProductionRow>::DynamicList;
  using DynamicListBase::rowStride;
  using ListLayout::poolCapacity;
  ProductionRow* row(int index) {
    return static_cast<ProductionRow*>(materializedRow(index));
  }
};

class WrappingList : public List {
 public:
  using List::List;
  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::WrapContentHeight()};
  }
};

// Verifies retained section extent/positions are remeasured without model
// reset, pool growth preserves live focus, selection stays logical, and
// scrolling is preserved then clamped after compaction. Offscreen rows bind
// current geometry.
TEST_F(DensityTest, MixedSectionsRelayoutRecyclingFocusAndScrollClamping) {
  DensityModel first_model;
  DensityModel second_model;
  ProductionRow header(context());
  ProductionRow footer(context());
  header.item().setHeadline("Header");
  footer.item().setHeadline("Footer");
  DensitySection first(context(), first_model, [&]() {
    return std::make_unique<ProductionRow>(context());
  });
  DensitySection second(context(), second_model, [&]() {
    return std::make_unique<ProductionRow>(context());
  });
  WrappingList list(context());
  list.setVariant(ListVariant::kBaseline);
  ListSelectionPolicy selection;
  selection.mode = SelectionMode::kSingle;
  list.setSelectionPolicy(selection);
  list.add(header);
  list.add(first);
  list.add(second);
  list.add(footer);
  SimpleScrollablePanel scroll(context(), list);
  Task& task = app_.addTaskFullScreen(scroll);
  refresh();
  ASSERT_NE(first.row(1), nullptr);
  ProductionRow* focused = first.row(1);
  ASSERT_TRUE(focused->requestFocus());
  ASSERT_TRUE(list.select(first, 1));
  const size_t old_capacity = first.poolCapacity();
  for (int8_t next : {int8_t(-2), int8_t(-5), int8_t(0)}) {
    material_.density = static_cast<Density>(next);
    scroll.requestLayoutDescending();
    scroll.invalidateDescending();
    refresh();
    int height = Scaled(next == 0 ? 56 : next == -2 ? 48 : 36);
    EXPECT_EQ(first.rowStride(), height);
    EXPECT_EQ(first.height(), 20 * height);
    EXPECT_EQ(second.offsetTop(), 21 * height);
    EXPECT_EQ(footer.offsetTop(), 41 * height);
    EXPECT_EQ(list.height(), 42 * height);
    EXPECT_EQ(list.selection().section, &first);
    EXPECT_EQ(list.selection().index, 1);
    EXPECT_EQ(first.row(1), focused);
    EXPECT_TRUE(focused->isFocused());
    EXPECT_TRUE(first.row(1)->visualContext().selected);
    if (next != 0) {
      EXPECT_EQ(first_model.releases, 0);
    }
    if (next == -5) {
      EXPECT_GT(first.poolCapacity(), old_capacity);
    }
  }
  scroll.scrollTo(0, -500);
  refresh();
  material_.density = Density::kMinus5;
  scroll.requestLayoutDescending();
  refresh();
  EXPECT_EQ(list.offsetTop(), -500);
  for (int index = first.first(); index <= first.last(); ++index) {
    ASSERT_NE(first.row(index), nullptr);
    EXPECT_EQ(first.row(index)->height(), Scaled(36));
    EXPECT_EQ(first.row(index)->offsetTop(), index * Scaled(36));
  }
  material_.density = Density::kDefault;
  scroll.requestLayoutDescending();
  refresh();
  scroll.scrollTo(0, -100000);
  refresh();
  material_.density = Density::kMinus5;
  scroll.requestLayoutDescending();
  refresh();
  EXPECT_EQ(list.offsetTop(), 320 - 42 * Scaled(36));
  EXPECT_EQ(footer.offsetTop(), 41 * Scaled(36));
  ASSERT_NE(second.row(19), nullptr);
  EXPECT_EQ(second.row(19)->height(), Scaled(36));
  EXPECT_EQ(list.selection().section, &first);
  EXPECT_EQ(list.selection().index, 1);
  task.navigation().clear();
}

}  // namespace
}  // namespace roo_windows::material3
