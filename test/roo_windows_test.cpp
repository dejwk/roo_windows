#include <Arduino.h>

#include <new>
#include <type_traits>

#include "roo_display/shape/basic.h"
#include "roo_windows/widgets/text_field.h"
#include "roo_windows_render_test_support.h"

using namespace roo_display;
using namespace roo_windows;
using namespace roo_windows::test_support;

namespace roo_windows {

class TiledRectWidget : public BasicSurfaceWidget {
 public:
  TiledRectWidget(ApplicationContext& context, Rect tile_bounds,
                  bool draw_border)
      : BasicSurfaceWidget(context),
        tile_bounds_(tile_bounds),
        draw_border_(draw_border) {}

  Color background() const override { return color::Black; }

  void paint(PaintContext& ctx) const override {
    ctx.clear();
    auto rect = FilledRect(0, 0, 5, 5, color::White);
    ctx.drawTiled(rect, tile_bounds_, kNoAlign, draw_border_);
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(12, 12);
  }

 private:
  Rect tile_bounds_;
  bool draw_border_;
};

class ExposedPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;
};

class PreferredFocusPanel : public ExposedPanel {
 public:
  using ExposedPanel::ExposedPanel;

  Widget* preferredFocusChild() override { return preferred_; }

  Widget* preferred_ = nullptr;
};

static_assert(!std::is_copy_constructible<FocusScope>::value,
              "An active focus scope must retain a stable address");
static_assert(!std::is_move_constructible<FocusScope>::value,
              "An active focus scope must retain a stable address");

class OpaqueExposedPanel : public ExposedPanel {
 public:
  using ExposedPanel::ExposedPanel;

  Color background() const override { return color::Blue; }
};

class FocusableTestWidget : public BasicWidget {
 public:
  explicit FocusableTestWidget(ApplicationContext& context)
      : BasicWidget(context), focus_change_count_(0), last_focused_(false) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }

  bool isFocusable() const override { return true; }

  void onFocusChanged(bool focused) override {
    ++focus_change_count_;
    last_focused_ = focused;
  }

  int focus_change_count_;
  bool last_focused_;
};

class TextFieldDestination : public Destination {
 public:
  explicit TextFieldDestination(ApplicationContext& context)
      : field(context, font_body1(), "", kLeft | kMiddle, TextField::NONE) {}

  Widget& getContents() override { return field; }

  TextField field;
};

class SloppyTouchSpyWidget : public BasicWidget {
 public:
  SloppyTouchSpyWidget(ApplicationContext& context, Dimensions dims,
                       int16_t right_slop)
      : BasicWidget(context), dims_(dims), right_slop_(right_slop) {}

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

  Rect getSloppyTouchParentBounds() const override {
    return Rect(parent_bounds().xMin(), parent_bounds().yMin(),
                parent_bounds().xMax() + right_slop_, parent_bounds().yMax());
  }

 private:
  Dimensions dims_;
  int16_t right_slop_;
};

class DispatcherTestWidget : public BasicWidget {
 public:
  explicit DispatcherTestWidget(ApplicationContext& context)
      : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }

  void triggerChange() { triggerInteractiveChange(); }
};

class SelfExcludingColorWidget : public BasicWidget {
 public:
  SelfExcludingColorWidget(ApplicationContext& context, Color color,
                           Dimensions dims)
      : BasicWidget(context),
        color_(color),
        dims_(dims),
        paint_delay_ms_(0),
        paint_count_(0) {}

  void paint(PaintContext& ctx) const override {
    ++paint_count_;
    ctx.fillRect(bounds(), color_);
    ctx.addExclusion(bounds());
    if (paint_delay_ms_ > 0) delay(paint_delay_ms_);
  }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

  void setPaintDelay(unsigned long delay_ms) { paint_delay_ms_ = delay_ms; }

  void setColor(Color color) {
    color_ = color;
    invalidateInterior();
  }

  int paintCount() const { return paint_count_; }

 protected:
  Rect getDirectPaintExclusionBounds() const override {
    return Rect(0, 0, -1, -1);
  }

 private:
  Color color_;
  Dimensions dims_;
  unsigned long paint_delay_ms_;
  mutable int paint_count_;
};

class RedirtyingColorWidget : public BasicWidget {
 public:
  explicit RedirtyingColorWidget(ApplicationContext& context)
      : BasicWidget(context), redirty_(false), paint_count_(0) {}

  void paint(PaintContext& ctx) const override {
    ++paint_count_;
    ctx.fillRect(bounds(), color::Red);
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(20, 20);
  }

  void setRedirty(bool redirty) { redirty_ = redirty; }

  int paintCount() const { return paint_count_; }

 protected:
  void paintWidgetContents(PaintContext& ctx) override {
    Widget::paintWidgetContents(ctx);
    if (redirty_) setDirty();
  }

 private:
  bool redirty_;
  mutable int paint_count_;
};

// Verifies that a minimal Application can be constructed against an offscreen
// display and Environment without crashing — basic API surface compiles and
// links.
TEST(Windows, BasicCompilation) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
}

// Verifies that Application exposes an ApplicationContext borrowing the
// scheduler and themes from the bootstrap Environment.
TEST(Windows, ApplicationContextExposesEnvironmentServices) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);

  EXPECT_EQ(&scheduler, &app.context().scheduler());
  EXPECT_EQ(&env.theme(), &app.context().theme());
  EXPECT_EQ(&env.keyboardColorTheme(), &app.context().keyboardColorTheme());

  DispatcherTestWidget widget(app.context());
  EXPECT_EQ(&app.context().theme(), &widget.theme());
}

// Verifies that focus is application-owned, updates widget state, and moves
// cleanly between eligible attached widgets.
TEST(Windows, FocusManagerTransfersFocusBetweenAttachedWidgets) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context(scheduler, env.theme(), env.keyboardColorTheme());
  ExposedPanel panel(context);
  FocusableTestWidget first(context);
  FocusableTestWidget second(context);
  panel.add(WidgetRef(first));
  panel.add(WidgetRef(second));
  first.layout(Rect(0, 0, 9, 9));
  second.layout(Rect(10, 0, 19, 9));

  EXPECT_TRUE(first.requestFocus());
  EXPECT_EQ(&first, context.focus().focused());
  EXPECT_TRUE(first.isFocused());
  EXPECT_TRUE(first.last_focused_);

  EXPECT_TRUE(second.requestFocus());
  EXPECT_EQ(&second, context.focus().focused());
  EXPECT_FALSE(first.isFocused());
  EXPECT_TRUE(second.isFocused());
  EXPECT_FALSE(first.last_focused_);
}

// Verifies that reverse traversal honors Shift+Tab semantics and wraps to the
// final eligible descendant when the current target is first in tree order.
TEST(Windows, FocusManagerMovesFocusBackwardsAndWraps) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context(scheduler, env.theme(), env.keyboardColorTheme());
  ExposedPanel panel(context);
  FocusableTestWidget first(context);
  FocusableTestWidget second(context);
  FocusableTestWidget third(context);
  panel.add(WidgetRef(first));
  panel.add(WidgetRef(second));
  panel.add(WidgetRef(third));
  first.layout(Rect(0, 0, 9, 9));
  second.layout(Rect(10, 0, 19, 9));
  third.layout(Rect(20, 0, 29, 9));

  ASSERT_TRUE(second.requestFocus());
  EXPECT_TRUE(context.focus().moveFocus(panel, true));
  EXPECT_EQ(&first, context.focus().focused());
  EXPECT_TRUE(context.focus().moveFocus(panel, true));
  EXPECT_EQ(&third, context.focus().focused());
}

// Verifies that directional traversal prefers candidates overlapping the
// orthogonal axis, then uses the nearest forward edge deterministically.
TEST(Windows, FocusManagerMovesFocusByGeometry) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context(scheduler, env.theme(), env.keyboardColorTheme());
  ExposedPanel panel(context);
  FocusableTestWidget source(context);
  FocusableTestWidget diagonal(context);
  FocusableTestWidget aligned(context);
  panel.add(WidgetRef(source));
  panel.add(WidgetRef(diagonal));
  panel.add(WidgetRef(aligned));
  source.layout(Rect(10, 10, 19, 19));
  diagonal.layout(Rect(20, 0, 29, 9));
  aligned.layout(Rect(40, 10, 49, 19));

  ASSERT_TRUE(source.requestFocus());
  EXPECT_TRUE(
      context.focus().moveFocusDirection(panel, FocusDirection::kRight));
  EXPECT_EQ(&aligned, context.focus().focused());
}

// Verifies that a focused editable field consumes hardware editing keys while
// keeping the software keyboard optional, and that read-only fields remain
// focusable without accepting text.
TEST(Windows, TextFieldEditsFromHardwareKeys) {
  roo::byte raster[320 * 240 * 2] = {};
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Application app(&env, display);
  NavigationHost& navigation = app.addTaskFullScreen().navigation();
  TextFieldDestination destination(app.context());
  navigation.push(destination);
  app.refresh();
  TextField& field = destination.field;

  ASSERT_TRUE(field.requestFocus());
  EXPECT_TRUE(field.onKeyEvent({KeyPhase::kDown, KeyCode::kCharacter, 0, 'A'}));
  EXPECT_TRUE(field.onKeyEvent({KeyPhase::kDown, KeyCode::kCharacter, 0, 'B'}));
  EXPECT_EQ("AB", field.content());
  EXPECT_TRUE(field.onKeyEvent({KeyPhase::kDown, KeyCode::kLeft, 0, 0}));
  EXPECT_TRUE(field.onKeyEvent({KeyPhase::kDown, KeyCode::kCharacter, 0, 'X'}));
  EXPECT_EQ("AXB", field.content());
  EXPECT_TRUE(field.onKeyEvent({KeyPhase::kDown, KeyCode::kDelete, 0, 0}));
  EXPECT_EQ("AX", field.content());

  field.setEditable(false);
  EXPECT_FALSE(
      field.onKeyEvent({KeyPhase::kDown, KeyCode::kCharacter, 0, 'Z'}));
  EXPECT_EQ("AX", field.content());
  navigation.clear();
}

// Verifies that hiding, disabling, and detaching a focused subtree clear
// focus before its parent link or visibility state becomes invalid.
TEST(Windows, FocusManagerClearsInvalidFocusedDescendants) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context(scheduler, env.theme(), env.keyboardColorTheme());
  ExposedPanel panel(context);
  FocusableTestWidget child(context);
  panel.add(WidgetRef(child));
  child.layout(Rect(0, 0, 9, 9));

  ASSERT_TRUE(child.requestFocus());
  child.setVisibility(Visibility::kInvisible);
  EXPECT_EQ(nullptr, context.focus().focused());
  EXPECT_FALSE(child.isFocused());

  child.setVisibility(Visibility::kVisible);
  ASSERT_TRUE(child.requestFocus());
  child.setEnabled(false);
  EXPECT_EQ(nullptr, context.focus().focused());

  child.setEnabled(true);
  ASSERT_TRUE(child.requestFocus());
  panel.removeLast();
  EXPECT_EQ(nullptr, context.focus().focused());
  EXPECT_FALSE(child.isFocused());
}

// Verifies scope entry prefers an explicitly selected descendant, contains
// focus requests, remembers presenter focus, and restores base focus on exit.
TEST(Windows, FocusManagerEntersContainsAndRestoresPresenterScope) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context(scheduler, env.theme(), env.keyboardColorTheme());
  PreferredFocusPanel base(context);
  FocusableTestWidget base_first(context);
  FocusableTestWidget base_second(context);
  base.add(WidgetRef(base_first));
  base.add(WidgetRef(base_second));
  base_first.layout(Rect(0, 0, 9, 9));
  base_second.layout(Rect(10, 0, 19, 9));

  PreferredFocusPanel presenter(context);
  FocusableTestWidget presenter_first(context);
  FocusableTestWidget presenter_second(context);
  presenter.add(WidgetRef(presenter_first));
  presenter.add(WidgetRef(presenter_second));
  presenter_first.layout(Rect(0, 0, 9, 9));
  presenter_second.layout(Rect(10, 0, 19, 9));
  presenter.preferred_ = &presenter_second;

  FocusManager manager(&base);
  ASSERT_TRUE(manager.requestFocus(base_second));
  FocusScope scope;
  ASSERT_TRUE(manager.canAdmitScope(scope, base, nullptr));
  manager.enterScope(scope, presenter, base);

  EXPECT_EQ(&presenter, manager.scopeRoot());
  EXPECT_EQ(&presenter_second, manager.focused());
  EXPECT_FALSE(manager.requestFocus(base_first));
  ASSERT_TRUE(manager.requestFocus(presenter_first));

  manager.exitScope(scope, base);
  EXPECT_EQ(&base, manager.scopeRoot());
  EXPECT_EQ(&base_second, manager.focused());
  EXPECT_EQ(&presenter_first, scope.last_focused);

  manager.enterScope(scope, presenter, base);
  EXPECT_EQ(&presenter_first, manager.focused());
  manager.exitScope(scope, base);
}

// Verifies an empty scope remains active without focus and an invalid
// remembered address falls back to the root's fresh preferred target.
TEST(Windows, FocusManagerAcceptsEmptyScopeAndValidatesRememberedAddress) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context(scheduler, env.theme(), env.keyboardColorTheme());
  ExposedPanel base(context);
  FocusableTestWidget base_target(context);
  base.add(WidgetRef(base_target));
  base_target.layout(Rect(0, 0, 9, 9));
  FocusManager manager(&base);
  ASSERT_TRUE(manager.requestFocus(base_target));

  ExposedPanel empty_presenter(context);
  FocusScope empty_scope;
  manager.enterScope(empty_scope, empty_presenter, base);
  EXPECT_EQ(&empty_presenter, manager.scopeRoot());
  EXPECT_EQ(nullptr, manager.focused());
  EXPECT_FALSE(manager.requestFocus(base_target));
  manager.exitScope(empty_scope, base);
  EXPECT_EQ(&base_target, manager.focused());

  PreferredFocusPanel presenter(context);
  FocusableTestWidget fallback(context);
  presenter.add(WidgetRef(fallback));
  fallback.layout(Rect(0, 0, 9, 9));
  presenter.preferred_ = &fallback;
  auto removed = std::make_unique<FocusableTestWidget>(context);
  Widget* removed_address = removed.get();
  presenter.add(WidgetRef(std::move(removed)));
  presenter.removeLast();
  empty_scope.last_focused = removed_address;

  manager.enterScope(empty_scope, presenter, base);
  EXPECT_EQ(&fallback, manager.focused());
  EXPECT_EQ(nullptr, empty_scope.last_focused);
  manager.exitScope(empty_scope, base);
}

// Verifies a root preference is advisory: normal focus eligibility may reject
// it while the presenter scope still activates successfully without focus.
TEST(Windows, FocusManagerAllowsIneligiblePreferredTarget) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context(scheduler, env.theme(), env.keyboardColorTheme());
  ExposedPanel base(context);
  PreferredFocusPanel presenter(context);
  FocusableTestWidget disabled(context);
  presenter.add(WidgetRef(disabled));
  disabled.layout(Rect(0, 0, 9, 9));
  disabled.setEnabled(false);
  presenter.preferred_ = &disabled;

  FocusManager manager(&base);
  FocusScope scope;
  manager.enterScope(scope, presenter, base);

  EXPECT_EQ(&presenter, manager.scopeRoot());
  EXPECT_EQ(nullptr, manager.focused());
  manager.exitScope(scope, base);
}

// Verifies restoration scans the live base tree before consulting a saved
// address that was removed while presenter focus covered the task.
TEST(Windows, FocusManagerFallsBackWhenSavedBaseTargetWasRemoved) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context(scheduler, env.theme(), env.keyboardColorTheme());
  PreferredFocusPanel base(context);
  FocusableTestWidget fallback(context);
  base.add(WidgetRef(fallback));
  fallback.layout(Rect(0, 0, 9, 9));
  auto removed = std::make_unique<FocusableTestWidget>(context);
  FocusableTestWidget* removed_ptr = removed.get();
  base.add(WidgetRef(std::move(removed)));
  removed_ptr->layout(Rect(10, 0, 19, 9));
  base.preferred_ = &fallback;

  ExposedPanel presenter(context);
  FocusableTestWidget presenter_target(context);
  presenter.add(WidgetRef(presenter_target));
  presenter_target.layout(Rect(0, 0, 9, 9));

  FocusManager manager(&base);
  ASSERT_TRUE(manager.requestFocus(*removed_ptr));
  FocusScope scope;
  manager.enterScope(scope, presenter, base);
  base.removeLast();
  manager.exitScope(scope, base);

  EXPECT_EQ(&fallback, manager.focused());
}

// Verifies preflight permits only an inactive incoming same-owner replacement
// scope and rejects an unrelated nested presenter scope.
TEST(Windows, FocusManagerPreflightsSinglePresenterScopeReplacement) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context(scheduler, env.theme(), env.keyboardColorTheme());
  ExposedPanel base(context);
  ExposedPanel outgoing_root(context);
  FocusManager manager(&base);
  FocusScope outgoing;
  FocusScope incoming;

  manager.enterScope(outgoing, outgoing_root, base);
  EXPECT_FALSE(manager.canAdmitScope(incoming, base, nullptr));
  EXPECT_TRUE(manager.canAdmitScope(incoming, base, &outgoing));
  EXPECT_FALSE(manager.canAdmitScope(outgoing, base, &outgoing));

  manager.exitScope(outgoing, base);
  EXPECT_TRUE(manager.canAdmitScope(incoming, base, nullptr));
}

// Verifies that the phase-1 widget-event dispatcher stores, replaces,
// clears, and dispatches one interactive-change handler per widget.
TEST(Windows, WidgetEventDispatcherStoresAndDispatchesHandlers) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  DispatcherTestWidget widget(app.context());
  WidgetEventDispatcher& dispatcher = app.context().widgetEvents();
  int call_count = 0;

  EXPECT_FALSE(dispatcher.hasInteractiveChangeHandler(widget));
  dispatcher.dispatchInteractiveChange(widget);
  EXPECT_EQ(0, call_count);

  dispatcher.setInteractiveChangeHandler(widget, [&]() { ++call_count; });
  EXPECT_TRUE(dispatcher.hasInteractiveChangeHandler(widget));
  dispatcher.dispatchInteractiveChange(widget);
  EXPECT_EQ(1, call_count);

  dispatcher.setInteractiveChangeHandler(widget, [&]() { call_count += 10; });
  dispatcher.dispatchInteractiveChange(widget);
  EXPECT_EQ(11, call_count);

  dispatcher.clearInteractiveChangeHandler(widget);
  EXPECT_FALSE(dispatcher.hasInteractiveChangeHandler(widget));
  dispatcher.dispatchInteractiveChange(widget);
  EXPECT_EQ(11, call_count);

  dispatcher.setInteractiveChangeHandler(widget, [&]() { ++call_count; });
  EXPECT_TRUE(dispatcher.hasInteractiveChangeHandler(widget));
  dispatcher.clearHandlers(widget);
  EXPECT_FALSE(dispatcher.hasInteractiveChangeHandler(widget));
}

TEST(Windows, WidgetStoresInteractiveChangeHandlersInDispatcher) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  DispatcherTestWidget widget(app.context());
  int call_count = 0;

  EXPECT_FALSE(widget.isClickable());
  EXPECT_FALSE(widget.hasInteractiveChangeHandler());

  widget.setOnInteractiveChange([&]() { ++call_count; });

  EXPECT_TRUE(widget.isClickable());
  EXPECT_TRUE(widget.hasInteractiveChangeHandler());
  widget.triggerChange();
  EXPECT_EQ(1, call_count);

  widget.setOnInteractiveChange(nullptr);
  EXPECT_FALSE(widget.isClickable());
  EXPECT_FALSE(widget.hasInteractiveChangeHandler());
  widget.triggerChange();
  EXPECT_EQ(1, call_count);
}

TEST(Windows, WidgetMoveTransfersInteractiveChangeHandler) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  DispatcherTestWidget widget(app.context());
  int call_count = 0;

  widget.setOnInteractiveChange([&]() { ++call_count; });

  DispatcherTestWidget moved(std::move(widget));
  EXPECT_FALSE(widget.hasInteractiveChangeHandler());
  EXPECT_FALSE(widget.isClickable());
  EXPECT_TRUE(moved.hasInteractiveChangeHandler());
  EXPECT_TRUE(moved.isClickable());

  moved.triggerChange();
  EXPECT_EQ(1, call_count);
}

TEST(Windows, WidgetDestructorClearsDispatcherHandlers) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  WidgetEventDispatcher& dispatcher = app.context().widgetEvents();
  alignas(DispatcherTestWidget) char storage[sizeof(DispatcherTestWidget)];

  auto* widget = new (storage) DispatcherTestWidget(app.context());
  widget->setOnInteractiveChange([]() {});
  EXPECT_TRUE(dispatcher.hasInteractiveChangeHandler(*widget));
  widget->~DispatcherTestWidget();

  auto* replacement = new (storage) DispatcherTestWidget(app.context());
  EXPECT_FALSE(dispatcher.hasInteractiveChangeHandler(*replacement));
  replacement->~DispatcherTestWidget();
}

// Verifies that a standalone context invalidates its widget lifetime handle,
// allowing a caller-owned widget with a registered handler to be destroyed
// after the context's runtime services are gone.
TEST(Windows, WidgetMayBeDestroyedAfterStandaloneContext) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  DispatcherTestWidget* widget = nullptr;
  {
    ApplicationContext context(scheduler, env.theme(),
                               env.keyboardColorTheme());
    widget = new DispatcherTestWidget(context);
    widget->setOnInteractiveChange([]() {});
  }

  delete widget;
}

// Verifies that the later-added child is painted on top of the earlier child
// where they overlap, while the earlier child remains visible outside the
// overlap.
TEST_F(RooWindowsRenderTest, LaterAddedChildPaintsOnTop) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(20, 20));
  auto front = std::make_unique<ColorBoxWidget>(context(), color::Blue,
                                                Dimensions(20, 20));

  app_.add(std::move(back), Box(4, 4, 30, 30));
  app_.add(std::move(front), Box(16, 16, 40, 40));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(8, 8));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(20, 20));
}

// Verifies that hiding a top child re-exposes the underlying pixels of the
// child beneath, and that re-showing it restores its pixels: invalidation
// covers the previously-occluded region on both transitions.
TEST_F(RooWindowsRenderTest, HideAndShowRestoresUnderlyingContent) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(20, 20));
  auto front = std::make_unique<ColorBoxWidget>(context(), color::Blue,
                                                Dimensions(20, 20));
  ColorBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(4, 4, 30, 30));
  app_.add(std::move(front), Box(16, 16, 40, 40));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(20, 20));

  front_ptr->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(20, 20));

  front_ptr->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(20, 20));
}

// Verifies that an elevated surface widget reports decoration bounds (drop
// shadow) extending beyond its layout bounds on all four sides, while a
// plain widget has zero decoration overflow and zero transient overflow.
TEST_F(RooWindowsRenderTest, SurfaceWidgetShadowBoundsExtendPastParentBounds) {
  auto surface = std::make_unique<ElevatedColorBoxWidget>(
      context(), color::Blue, Dimensions(20, 20), 12);
  auto plain = std::make_unique<TouchSpyWidget>(context(), Dimensions(20, 20));

  ElevatedColorBoxWidget* surface_ptr = surface.get();
  TouchSpyWidget* plain_ptr = plain.get();

  app_.add(std::move(surface), Box(16, 16, 40, 40));
  app_.add(std::move(plain), Box(2, 2, 12, 12));

  Rect surface_bounds = surface_ptr->parent_bounds();
  Rect shadow_bounds = surface_ptr->getParentDecorationBounds();
  EXPECT_TRUE(surface_ptr->hasDecorationOverflow());
  EXPECT_LT(shadow_bounds.xMin(), surface_bounds.xMin());
  EXPECT_LT(shadow_bounds.yMin(), surface_bounds.yMin());
  EXPECT_GT(shadow_bounds.xMax(), surface_bounds.xMax());
  EXPECT_GT(shadow_bounds.yMax(), surface_bounds.yMax());

  EXPECT_FALSE(plain_ptr->hasDecorationOverflow());
  EXPECT_EQ(plain_ptr->parent_bounds(), plain_ptr->getParentDecorationBounds());
  EXPECT_FALSE(surface_ptr->hasTransientPaintOverflow());
  EXPECT_FALSE(plain_ptr->hasTransientPaintOverflow());
  EXPECT_EQ(surface_ptr->parent_bounds(),
            surface_ptr->getParentTransientPaintBounds());
  EXPECT_EQ(plain_ptr->parent_bounds(),
            plain_ptr->getParentTransientPaintBounds());
}

// Verifies that custom ink insets shape every derived rectangle: content
// bounds, parent-content bounds, visual bounds, parent-visual bounds, max
// bounds, max-parent bounds, and the exclusion rect.
TEST_F(RooWindowsRenderTest, InkInsetsDriveContentAndVisualBounds) {
  InkBoundsWidget widget(context(), Dimensions(20, 10), Insets(-2, 1, 3, -4));
  widget.layout(Rect(12, 8, 31, 17));

  EXPECT_EQ(Rect(-2, 1, 16, 13), widget.getContentBounds());
  EXPECT_EQ(Rect(10, 9, 28, 21), widget.getParentContentBounds());
  EXPECT_EQ(Rect(-2, 0, 19, 13), widget.getVisualBounds());
  EXPECT_EQ(Rect(10, 8, 31, 21), widget.getParentVisualBounds());
  EXPECT_EQ(widget.getVisualBounds(), widget.maxBounds());
  EXPECT_EQ(widget.getParentVisualBounds(), widget.maxParentBounds());
  EXPECT_EQ(widget.getContentBounds(), widget.exclusionBounds());
}

// Verifies that for an unclipped child the parent's max-bounds expand to
// include the child's ink overflow outside the layout rect (so the parent
// reserves repaint area for ink that bleeds past the child's layout bounds).
TEST_F(RooWindowsRenderTest, UnclippedChildMaxBoundsIncludeInkOverflow) {
  auto child = std::make_unique<InkBoundsWidget>(context(), Dimensions(10, 8),
                                                 Insets(-3, 0, 0, 0));
  InkBoundsWidget* child_ptr = child.get();
  child_ptr->setParentClipMode(ParentClipMode::kUnclipped);

  app_.add(std::move(child), Box(1, 6, 10, 13));

  EXPECT_EQ(Rect(-2, 6, 10, 13), child_ptr->maxParentBounds());
  EXPECT_EQ(Rect(-2, 0, 63, 47), app_.root().maxBounds());
}

// Verifies that hiding an elevated surface correctly erases its drop-shadow
// pixels outside the layout rect, and that re-showing it restores the
// shadow to exactly the same color (regression test for shadow-overflow
// invalidation).
TEST_F(RooWindowsRenderTest, HideAndShowRestoresShadowOverflowRegion) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto front = std::make_unique<ElevatedColorBoxWidget>(context(), color::Blue,
                                                        Dimensions(20, 20), 12);
  ElevatedColorBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(16, 12, 35, 31));

  ASSERT_TRUE(refresh());
  Color shadow_pixel = pixelAt(14, 22);
  EXPECT_NE(QuantizeToArgb4444(color::Red), shadow_pixel);

  front_ptr->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(14, 22));

  front_ptr->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(shadow_pixel, pixelAt(14, 22));
}

// Verifies that switching a surface widget from rectangular to rounded
// invalidates the now-exposed corners so the underlying widget's pixels
// re-appear there.
TEST_F(RooWindowsRenderTest, RoundedSurfaceInvalidationRestoresExposedCorners) {
  auto back = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                               Dimensions(48, 40));
  auto front = std::make_unique<MutableShapeColorBoxWidget>(
      context(), color::Blue, Dimensions(20, 20));
  MutableShapeColorBoxWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 47, 39));
  app_.add(std::move(front), Box(16, 12, 35, 31));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(16, 12));

  front_ptr->setRounded(true);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(16, 12));
}

// Verifies that callback-free path construction picks the topmost visible
// child at the touch point; if it is hidden, the next-lower child is selected.
TEST_F(RooWindowsRenderTest, GesturePathPrefersTopmostVisibleChild) {
  auto back = std::make_unique<TouchSpyWidget>(context(), Dimensions(20, 20));
  auto front = std::make_unique<TouchSpyWidget>(context(), Dimensions(20, 20));
  TouchSpyWidget* back_ptr = back.get();
  TouchSpyWidget* front_ptr = front.get();

  app_.add(std::move(back), Box(0, 0, 24, 24));
  app_.add(std::move(front), Box(10, 10, 34, 34));

  std::vector<Widget*> path;
  ASSERT_TRUE(app_.root().fillTouchTargetPath(15, 15, path));
  ASSERT_EQ(front_ptr, path.back());

  front_ptr->setVisibility(Visibility::kInvisible);
  path.clear();
  ASSERT_TRUE(app_.root().fillTouchTargetPath(15, 15, path));
  ASSERT_EQ(back_ptr, path.back());
}

// Verifies that sloppy-hit path construction recurses through containers and
// reaches the intended leaf widget.
TEST_F(RooWindowsRenderTest, SloppyGesturePathRecursesThroughContainers) {
  auto outer = std::make_unique<ExposedPanel>(context());
  ExposedPanel* outer_ptr = outer.get();
  auto inner = std::make_unique<ExposedPanel>(context());
  ExposedPanel* inner_ptr = inner.get();
  auto leaf =
      std::make_unique<SloppyTouchSpyWidget>(context(), Dimensions(4, 4), 8);
  SloppyTouchSpyWidget* leaf_ptr = leaf.get();

  inner_ptr->add(std::move(leaf), Rect(0, 0, 3, 3));
  outer_ptr->add(std::move(inner), Rect(2, 2, 5, 5));
  app_.add(std::move(outer), Box(4, 4, 23, 23));

  std::vector<Widget*> path;
  ASSERT_TRUE(outer_ptr->fillSloppyTouchTargetPath(10, 3, path));
  ASSERT_EQ(leaf_ptr, path.back());
}

// Verifies that geometric path construction reaches the deepest enabled child
// without invoking its role callbacks, and skips it after it becomes disabled.
TEST_F(RooWindowsRenderTest, GesturePathBuildsWithoutGestureCallbacks) {
  auto panel = std::make_unique<ExposedPanel>(context());
  ExposedPanel* panel_ptr = panel.get();
  auto child = std::make_unique<GestureRoleSpyWidget>(context());
  GestureRoleSpyWidget* child_ptr = child.get();
  panel_ptr->add(std::move(child), Rect(2, 2, 21, 21));
  app_.add(std::move(panel), Box(4, 4, 35, 35));

  std::vector<Widget*> path;
  ASSERT_TRUE(app_.root().fillTouchTargetPath(10, 10, path));
  ASSERT_FALSE(path.empty());
  EXPECT_EQ(child_ptr, path.back());
  EXPECT_EQ(0, child_ptr->down_count());

  child_ptr->setEnabled(false);
  path.clear();
  ASSERT_TRUE(app_.root().fillTouchTargetPath(10, 10, path));
  EXPECT_EQ(panel_ptr, path.back());
  EXPECT_EQ(0, child_ptr->down_count());
}

// Verifies that a refresh() call which exceeds its time deadline mid-paint
// returns false and leaves the partial state untouched; a subsequent
// refresh() without a deadline completes the paint.
TEST_F(RooWindowsRenderTest, RefreshCanResumeAfterDeadlineExceeded) {
  auto box = std::make_unique<ColorBoxWidget>(context(), color::Green,
                                              Dimensions(20, 20));
  app_.add(std::move(box), Box(8, 8, 36, 36));

  EXPECT_FALSE(refresh(roo_time::Uptime::Start()));
  EXPECT_NE(QuantizeToArgb4444(color::Green), pixelAt(16, 16));

  EXPECT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Green), pixelAt(16, 16));
}

// A child that completed before a later sibling exceeded the deadline must
// keep its exclusion on retry. Some widgets, including navigation
// destinations, create exclusions only while paint() runs; losing that state
// lets the parent's surface erase a completed child.
TEST_F(RooWindowsRenderTest,
       DeadlineRetryPreservesCompletedChildrenAndReopensInvalidations) {
  auto panel = std::make_unique<OpaqueExposedPanel>(context());
  OpaqueExposedPanel* panel_ptr = panel.get();
  auto delayed = std::make_unique<SelfExcludingColorWidget>(
      context(), color::Green, Dimensions(20, 20));
  SelfExcludingColorWidget* delayed_ptr = delayed.get();
  auto target = std::make_unique<SelfExcludingColorWidget>(
      context(), color::Red, Dimensions(20, 20));
  SelfExcludingColorWidget* target_ptr = target.get();
  panel_ptr->add(std::move(delayed), Rect(0, 0, 19, 19));
  panel_ptr->add(std::move(target), Rect(20, 0, 39, 19));
  app_.add(std::move(panel), Box(8, 8, 47, 27));
  ASSERT_TRUE(refresh());
  ASSERT_EQ(QuantizeToArgb4444(color::Red), pixelAt(32, 16));

  target_ptr->setPaintDelay(80);
  delayed_ptr->setPaintDelay(80);
  panel_ptr->invalidateInterior();
  EXPECT_FALSE(refresh(roo_time::Uptime::Now() + roo_time::Millis(50)));
  EXPECT_EQ(2, target_ptr->paintCount());
  EXPECT_EQ(1, delayed_ptr->paintCount());

  EXPECT_FALSE(refresh(roo_time::Uptime::Now() + roo_time::Millis(50)));
  EXPECT_EQ(2, target_ptr->paintCount());
  EXPECT_EQ(2, delayed_ptr->paintCount());

  target_ptr->setPaintDelay(0);
  delayed_ptr->setPaintDelay(0);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(2, target_ptr->paintCount());
  EXPECT_EQ(2, delayed_ptr->paintCount());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(32, 16));

  // A real state change between attempts reopens just that target in the saved
  // snapshot. The unrelated delayed sibling must remain completed.
  delayed_ptr->setPaintDelay(80);
  panel_ptr->invalidateInterior();
  EXPECT_FALSE(refresh(roo_time::Uptime::Now() + roo_time::Millis(50)));
  EXPECT_EQ(3, target_ptr->paintCount());
  EXPECT_EQ(3, delayed_ptr->paintCount());

  target_ptr->setColor(color::Yellow);
  delayed_ptr->setPaintDelay(0);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(4, target_ptr->paintCount());
  EXPECT_EQ(3, delayed_ptr->paintCount());
  EXPECT_EQ(QuantizeToArgb4444(color::Yellow), pixelAt(32, 16));
}

TEST_F(RooWindowsRenderTest,
       CompletedDirtyWidgetPublishesTerminalStateBeforeTimeout) {
  auto panel = std::make_unique<OpaqueExposedPanel>(context());
  OpaqueExposedPanel* panel_ptr = panel.get();
  auto delayed = std::make_unique<SelfExcludingColorWidget>(
      context(), color::Green, Dimensions(20, 20));
  SelfExcludingColorWidget* delayed_ptr = delayed.get();
  auto animated = std::make_unique<RedirtyingColorWidget>(context());
  RedirtyingColorWidget* animated_ptr = animated.get();
  panel_ptr->add(std::move(delayed), Rect(0, 0, 19, 19));
  panel_ptr->add(std::move(animated), Rect(20, 0, 39, 19));
  app_.add(std::move(panel), Box(8, 8, 47, 27));
  ASSERT_TRUE(refresh());

  animated_ptr->setRedirty(true);
  delayed_ptr->setPaintDelay(80);
  panel_ptr->invalidateInterior();
  EXPECT_FALSE(refresh(roo_time::Uptime::Now() + roo_time::Millis(50)));
  EXPECT_EQ(2, animated_ptr->paintCount());

  delayed_ptr->setPaintDelay(0);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(3, animated_ptr->paintCount());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(32, 16));
}

// Verifies that draw-tiled clips paint output to the tile bounds even when
// the tile content rectangle extends past those bounds: pixels inside the
// tile bounds are drawn, pixels outside are not.
TEST_F(RooWindowsRenderTest, DrawTiledClipsOversizedContentToTileBounds) {
  auto tile =
      std::make_unique<TiledRectWidget>(context(), Rect(4, 4, 6, 6), true);

  app_.add(std::move(tile), Box(8, 8, 19, 19));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::White), pixelAt(12, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::White), pixelAt(13, 13));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(11, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(14, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(12, 14));
}

// Verifies the same clipping invariant as DrawTiledClipsOversizedContent...
// when the tile is rendered without a border path: oversized content is
// still clipped to the tile bounds.
TEST_F(RooWindowsRenderTest,
       DrawTiledWithoutBorderClipsOversizedContentToTileBounds) {
  auto tile =
      std::make_unique<TiledRectWidget>(context(), Rect(4, 4, 6, 6), false);

  app_.add(std::move(tile), Box(8, 8, 19, 19));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::White), pixelAt(12, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::White), pixelAt(13, 13));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(11, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(14, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(12, 14));
}

// Verifies that draw-tiled with empty interior bounds and no border draws
// nothing: the tile area is left as parent background instead of being
// filled by a degenerate-rect paint.
TEST_F(RooWindowsRenderTest, DrawTiledIgnoresEmptyBoundsWithoutBorder) {
  auto tile =
      std::make_unique<TiledRectWidget>(context(), Rect(4, 4, 3, 6), false);

  app_.add(std::move(tile), Box(8, 8, 19, 19));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(12, 12));
  EXPECT_EQ(QuantizeToArgb4444(color::Black), pixelAt(8, 8));
}

}  // namespace roo_windows

// Verifies signed 24-bit Y reconstruction without shifting negative signed
// values, including both packing boundaries and the empty-rectangle sentinel.
TEST(RectTest, PackedNegativeCoordinatesAreDefined) {
  for (int32_t y : {-8388608, -65537, -65536, -1, 0, 65535, 65536, 8388607}) {
    roo_windows::Rect rect(-2, y, 2, y);
    EXPECT_EQ(y, rect.yMin());
    EXPECT_EQ(y, rect.yMax());
    EXPECT_EQ(1, rect.height());
    EXPECT_TRUE(rect.contains(0, y));
  }
  roo_windows::Rect empty(0, 0, -1, -1);
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(-1, empty.yMax());
  roo_windows::Rect moved =
      roo_windows::Rect(0, -65537, 2, -65535).translate(0, 65536);
  EXPECT_EQ(-1, moved.yMin());
  EXPECT_EQ(1, moved.yMax());
}
