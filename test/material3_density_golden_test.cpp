#include <algorithm>
#include <string>
#include <vector>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "material3_density_acceptance_theme.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/filled/navigation.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/application.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/list/list.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/text_field/text_field.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows::material3 {
namespace {

// Renders real controls at natural heights through the normal application path
// into RGB565, retaining an equal canvas for side-by-side density comparisons.
roo_display::Offscreen<roo_display::Rgb888> RenderColumn(Density density,
                                                         bool dark) {
  const int width = Scaled(224);
  const int height = Scaled(528);
  std::vector<roo::byte> raster(width * height * 2);
  roo_display::OffscreenDevice<roo_display::Rgb565> device(
      width, height, raster.data(), roo_display::Rgb565());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Material3Theme material = test_support::MakeAcceptanceTheme(dark);
  material.density = density;
  Theme theme{MakeFrameworkTheme(material), &material};
  Environment environment(scheduler, theme);
  Application app(&environment, display);
  std::string heading = "Density " + std::to_string(static_cast<int>(density));
  TextLabel title(app.context(), heading, text_style_title_medium());
  Button save(app.context(), "Save");
  save.setIcon(&SCALED_ROO_ICON(filled, navigation_check));
  Button cancel(app.context(), "Cancel", ButtonVariant::kOutlined);
  cancel.setHover(true);
  HorizontalLayout buttons(app.context());
  buttons.add(save);
  buttons.add(cancel);
  TextField filled(app.context(), "Device name");
  filled.setText("Kitchen");
  filled.setSupportingText("Choose a room");
  TextField outlined(app.context(), "Location", TextFieldVariant::kOutlined);
  outlined.setText("Hall");
  outlined.setErrorText("Check location");
  outlined.setLeadingIcon(&SCALED_ROO_ICON(filled, navigation_check));
  ListRow<CheckboxListItem> baseline(app.context(), "Alerts");
  baseline.item().setChecked(true);
  ListEntryVisualContext baseline_context;
  baseline_context.variant = ListVariant::kBaseline;
  baseline.setVisualContext(baseline_context);
  ListRow<RadioListItem> expressive(app.context(), "Automatic", "Online");
  expressive.item().setSelected(true);
  ListEntryVisualContext expressive_context;
  expressive_context.style = ListStyle::kSegmented;
  expressive_context.selected = true;
  expressive.setVisualContext(expressive_context);
  StandardMenuItem baseline_item(StandardMenuItemInit{
      "Inspect", {}, nullptr, StandardMenuItemFlags::kSelectable});
  baseline_item.setShortcut("Ctrl+I");
  MenuEntry baseline_menu(app.context());
  baseline_menu.setMenuItem(baseline_item);
  ListEntryVisualContext baseline_menu_context;
  baseline_menu_context.variant = ListVariant::kBaseline;
  baseline_menu_context.density = DensityOverride::Explicit(density);
  baseline_menu.setVisualContext(baseline_menu_context);
  StandardMenuItem expressive_item(StandardMenuItemInit{
      "Settings",
      {},
      nullptr,
      StandardMenuItemFlags::kSelectable | StandardMenuItemFlags::kSelected});
  expressive_item.setBadgeValue(3);
  expressive_item.setTrailingIcon(
      &SCALED_ROO_ICON(filled, navigation_chevron_right));
  MenuEntry expressive_menu(app.context());
  expressive_menu.setMenuItem(expressive_item);
  ListEntryVisualContext menu_context;
  menu_context.style = ListStyle::kSegmented;
  menu_context.selected = true;
  menu_context.density = DensityOverride::Explicit(density);
  expressive_menu.setVisualContext(menu_context);
  VerticalLayout column(app.context());
  column.setPadding(Padding(Scaled(8)));
  column.add(title);
  column.add(buttons);
  column.add(filled);
  column.add(outlined);
  column.add(baseline);
  column.add(expressive);
  column.add(baseline_menu);
  column.add(expressive_menu);
  Task& task = app.addTaskFullScreen(column);
  app.refresh();
  // These bounds also guard against a gallery silently cropping a component.
  EXPECT_GE(expressive_menu.offsetTop(), 0);
  EXPECT_LE(expressive_menu.offsetTop() + expressive_menu.height(), height);
  EXPECT_GT(baseline.height(), baseline.item().checkbox().height());
  EXPECT_GT(expressive.height(), expressive.item().radioButton().height());
  roo_display::Offscreen<roo_display::Rgb888> capture =
      ::roo_windows::test::CaptureRgb(device.raster(), 0, 0, width, height);
  task.navigation().clear();
  return capture;
}

// Captures only foreground pixels of an icon-only text button. Comparing this
// sequence across heights detects glyph clipping while allowing translation.
std::vector<Color> RenderIconPixels(Density density, ButtonSize size) {
  const int width = Scaled(192);
  const int height = Scaled(160);
  std::vector<roo::byte> raster(width * height * 2);
  roo_display::OffscreenDevice<roo_display::Rgb565> device(
      width, height, raster.data(), roo_display::Rgb565());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Material3Theme material = DefaultTheme().material3Theme();
  material.density = density;
  Theme theme{MakeFrameworkTheme(material), &material};
  Environment environment(scheduler, theme);
  Application app(&environment, display);
  Button button(app.context(), {}, ButtonVariant::kText);
  button.setSize(size);
  button.setIcon(&SCALED_ROO_ICON(filled, navigation_check));
  VerticalLayout column(app.context());
  column.add(button);
  Task& task = app.addTaskFullScreen(column);
  app.refresh();
  std::vector<int16_t> xs(width);
  std::vector<int16_t> ys(width);
  std::vector<Color> colors(width);
  for (int x = 0; x < width; ++x) xs[x] = x;
  device.raster().readColors(xs.data(), ys.data(), width, colors.data());
  Color background = colors.back();
  std::vector<Color> pixels;
  for (int y = 0; y < height; ++y) {
    std::fill(ys.begin(), ys.end(), y);
    device.raster().readColors(xs.data(), ys.data(), width, colors.data());
    for (Color color : colors) {
      if (color != background) pixels.push_back(color);
    }
  }
  task.navigation().clear();
  return pixels;
}

// Verifies compacting all five sizes preserves every painted icon pixel even
// when the row becomes shorter than its transparent anchor canvas.
TEST(Material3DensityGolden, CompactButtonsPreserveCompleteIconArtwork) {
  for (int index = 0; index < 5; ++index) {
    ButtonSize size = static_cast<ButtonSize>(index);
    std::vector<Color> expected = RenderIconPixels(Density::kDefault, size);
    ASSERT_FALSE(expected.empty());
    for (int level = -1; level >= -5; --level) {
      SCOPED_TRACE(::testing::Message()
                   << "size=" << index << " level=" << level);
      EXPECT_EQ(RenderIconPixels(static_cast<Density>(level), size), expected);
    }
  }
}

// Verifies light/dark RGB565 raster output at default, moderate, and maximum
// compaction: icons, interaction layers, field labels/outlines, lists and
// menus.
TEST(Material3DensityGolden, LightAndDarkRgb565Comparisons) {
  for (bool dark : {false, true}) {
    const int width = Scaled(224);
    const int height = Scaled(528);
    roo_display::Offscreen<roo_display::Rgb888> gallery(
        width * 3, height, roo_display::color::Black, roo_display::Rgb888());
    const Density levels[] = {Density::kDefault, Density::kMinus2,
                              Density::kMinus5};
    for (int column = 0; column < 3; ++column) {
      roo_display::Offscreen<roo_display::Rgb888> capture =
          RenderColumn(levels[column], dark);
      roo_display::DrawingContext context(gallery, column * width, 0);
      context.draw(capture);
    }
    std::string name = std::string(dark ? "dark_" : "light_") +
                       std::to_string(ROO_WINDOWS_ZOOM);
    EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
        gallery, "test/goldens/material3_density/" + name + ".ppm",
        "material3_density_" + name));
  }
}

}  // namespace
}  // namespace roo_windows::material3
