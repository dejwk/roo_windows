// *************** EMULATOR SETUP BEGIN

#ifdef ROO_TESTING

#include "roo_testing/devices/display/ili9341/ili9341spi.h"
#include "roo_testing/devices/touch/xpt2046/xpt2046spi.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/transducers/ui/viewport/flex_viewport.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"
#include "roo_windows/fake/fltk_key_source.h"

using roo_testing_transducers::FlexViewport;
using roo_testing_transducers::FltkViewport;

struct Emulator {
  FltkViewport viewport;
  FlexViewport flex_viewport;
  FakeIli9341Spi display;
  FakeXpt2046Spi touch;

  Emulator()
      : viewport(),
        flex_viewport(viewport, 1, FlexViewport::kRotationRight),
        display(flex_viewport),
        touch(flex_viewport, FakeXpt2046Spi::Calibration(269, 249, 3829, 3684,
                                                         true, false, false)) {
    FakeEsp32().attachSpiDevice(display, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(7, display.cs());
    FakeEsp32().gpio.attachOutput(2, display.dc());
    FakeEsp32().gpio.attachOutput(3, display.rst());
    FakeEsp32().attachSpiDevice(touch, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(1, touch.cs());
  }
} emulator;

roo_windows::fake::FltkKeySource emulator_keys;

#endif

// *************** DISPLAY SETUP BEGIN

#include <memory>
#include <string>

#include "Arduino.h"
#include "roo_display.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"

static constexpr int kCsPin = 7;
static constexpr int kDcPin = 2;
static constexpr int kRstPin = 3;
static constexpr int kSpiSckPin = 4;
static constexpr int kSpiMisoPin = 5;
static constexpr int kSpiMosiPin = 6;
static constexpr int kTouchCsPin = 1;

Ili9341spi<kCsPin, kDcPin, kRstPin> screen(Orientation().rotateLeft());
TouchXpt2046<kTouchCsPin> touch;
Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

void initDisplay() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.enableTurbo();
  display.init();
}

// *************** EXAMPLE STARTS HERE

#include "roo_icons/outlined/24/action.h"
#include "roo_icons/outlined/24/navigation.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"
#include "roo_windows/material3/navigation_bar/navigation_bar.h"
#include "roo_windows/material3/navigation_rail/navigation_rail.h"
#include "roo_windows/widgets/text_block.h"
#include "roo_windows/widgets/text_label.h"

namespace {

using roo_windows::material3::GridLayout;
using roo_windows::material3::GridSpan;
using roo_windows::material3::LayoutBreakpoint;
using roo_windows::material3::PaneLayout;
using roo_windows::material3::PaneRole;

GridSpan DashboardSpan(uint8_t compact, uint8_t medium, uint8_t expanded) {
  GridSpan span;
  span.compact = compact;
  span.medium = medium;
  span.expanded = expanded;
  span.large = expanded;
  span.extra_large = expanded;
  return span;
}

class ListDetailPage : public PaneLayout {
 public:
  explicit ListDetailPage(ApplicationContext& context)
      : PaneLayout(context), details_(nullptr) {
    auto list = std::make_unique<FlexLayout>(context, FlexDirection::kColumn);
    list->setPadding(Padding(Scaled(12), Scaled(12)));
    list->setGap(Scaled(8));
    auto overview = std::make_unique<material3::Button>(
        context, "System overview", material3::ButtonVariant::kFilledTonal);
    overview->setOnInteractiveChange([this]() { selectDetail(0); });
    auto heater = std::make_unique<material3::Button>(
        context, "Heater controls", material3::ButtonVariant::kOutlined);
    heater->setOnInteractiveChange([this]() { selectDetail(1); });
    list->add(WidgetRef(std::move(overview)));
    list->add(WidgetRef(std::move(heater)));

    auto details = std::make_unique<StackedLayout>(context);
    details_ = details.get();
    details_->add(makeDetail(context, "System overview",
                             "Pool temperature, heat demand, and current "
                             "pump state stay in the main pane."));
    details_->add(
        makeDetail(context, "Heater controls",
                   "Choose a list item on compact screens, then "
                   "return here without changing application routing."));
    setLeadingPane(WidgetRef(std::move(list)));
    setMainPane(WidgetRef(std::move(details)));
    selectDetail(0);
  }

 private:
  std::unique_ptr<ScrollablePanel> makeDetail(ApplicationContext& context,
                                              roo::string_view title,
                                              roo::string_view description) {
    auto detail = std::make_unique<FlexLayout>(context, FlexDirection::kColumn);
    detail->setPadding(Padding(Scaled(16), Scaled(20)));
    detail->setGap(Scaled(12));
    detail->add(WidgetRef(std::make_unique<TextBlock>(
        context, std::string(title), material2::text_style_h6(),
        roo_display::kTop | roo_display::kLeft)));
    detail->add(WidgetRef(std::make_unique<TextBlock>(
        context, std::string(description), material2::text_style_body1(),
        roo_display::kTop | roo_display::kLeft)));
    auto back = std::make_unique<material3::Button>(
        context, "Back to list", material3::ButtonVariant::kText);
    back->setOnInteractiveChange(
        [this]() { setActivePane(PaneRole::kLeading); });
    detail->add(WidgetRef(std::move(back)));
    return std::make_unique<ScrollablePanel>(context,
                                             WidgetRef(std::move(detail)));
  }

  void selectDetail(size_t index) {
    for (size_t i = 0; i < details_->children().size(); ++i) {
      details_->children()[i]->setVisibility(i == index ? Visibility::kVisible
                                                        : Visibility::kGone);
    }
    setActivePane(PaneRole::kMain);
  }

  StackedLayout* details_;
};

class FeedPage : public GridLayout {
 public:
  explicit FeedPage(ApplicationContext& context) : GridLayout(context) {
    addCard(context, "Solar gain", DashboardSpan(2, 4, 6),
            material3::ButtonVariant::kFilled);
    addCard(context, "Pool temperature", DashboardSpan(2, 4, 6),
            material3::ButtonVariant::kFilledTonal);
    addCard(context, "Pump history", DashboardSpan(4, 8, 12),
            material3::ButtonVariant::kOutlined);
  }

 private:
  void addCard(ApplicationContext& context, roo::string_view label,
               GridSpan span, material3::ButtonVariant variant) {
    Params params;
    params.span = span;
    add(WidgetRef(std::make_unique<material3::Button>(context, label, variant)),
        params);
  }
};

class LayoutCatalog;

class CatalogRail : public material3::NavigationRail {
 public:
  CatalogRail(ApplicationContext& context, LayoutCatalog& catalog)
      : material3::NavigationRail(context), catalog_(catalog) {}

 protected:
  void onSelectedIndexChanged(int old_index, int new_index) override;

 private:
  LayoutCatalog& catalog_;
};

class CatalogBar : public material3::NavigationBar {
 public:
  CatalogBar(ApplicationContext& context, LayoutCatalog& catalog)
      : material3::NavigationBar(context), catalog_(catalog) {}

 protected:
  void onSelectedIndexChanged(int old_index, int new_index) override;

 private:
  LayoutCatalog& catalog_;
};

class LayoutCatalog : public material3::LayoutScaffold {
 public:
  explicit LayoutCatalog(ApplicationContext& context)
      : LayoutScaffold(context),
        pages_(nullptr),
        rail_(nullptr),
        bar_(nullptr) {
    auto top_bar = std::make_unique<material3::AppBar>(context);
    top_bar->setTitle("Adaptive pool layout");
    setTopBar(WidgetRef(std::move(top_bar)));

    auto pages = std::make_unique<StackedLayout>(context);
    pages_ = pages.get();
    auto details = std::make_unique<ListDetailPage>(context);
    auto feed = std::make_unique<FeedPage>(context);
    pages_->add(WidgetRef(std::move(details)));
    pages_->add(WidgetRef(std::move(feed)));
    setBody(WidgetRef(std::move(pages)));

    auto rail = std::make_unique<CatalogRail>(context, *this);
    rail_ = rail.get();
    rail_->setLayout(material3::NavigationRailLayout::kExpanded);
    rail_->add(WidgetRef(std::make_unique<material3::NavigationRailDestination>(
        context, "Details", &ic_outlined_24_navigation_home_work())));
    rail_->add(WidgetRef(std::make_unique<material3::NavigationRailDestination>(
        context, "Feed", &ic_outlined_24_action_bookmark())));
    rail_->setSelectedIndex(0);
    setLeadingRail(WidgetRef(std::move(rail)), {LayoutBreakpoint::kExpanded,
                                                LayoutBreakpoint::kExtraLarge});

    auto bar = std::make_unique<CatalogBar>(context, *this);
    bar_ = bar.get();
    bar_->add(WidgetRef(std::make_unique<material3::NavigationBarDestination>(
        context, "Details", &ic_outlined_24_navigation_home_work())));
    bar_->add(WidgetRef(std::make_unique<material3::NavigationBarDestination>(
        context, "Feed", &ic_outlined_24_action_bookmark())));
    bar_->setSelectedIndex(0);
    setBottomBar(WidgetRef(std::move(bar)),
                 {LayoutBreakpoint::kCompact, LayoutBreakpoint::kMedium});
    selectPage(0);
  }

  void selectPage(int index) {
    for (size_t i = 0; i < pages_->children().size(); ++i) {
      pages_->children()[i]->setVisibility(i == static_cast<size_t>(index)
                                               ? Visibility::kVisible
                                               : Visibility::kGone);
    }
    if (rail_ != nullptr && rail_->selectedIndex() != index) {
      rail_->setSelectedIndex(index);
    }
    if (bar_ != nullptr && bar_->selectedIndex() != index) {
      bar_->setSelectedIndex(index);
    }
  }

 private:
  StackedLayout* pages_;
  CatalogRail* rail_;
  CatalogBar* bar_;
};

void CatalogRail::onSelectedIndexChanged(int old_index, int new_index) {
  (void)old_index;
  catalog_.selectPage(new_index);
}

void CatalogBar::onSelectedIndexChanged(int old_index, int new_index) {
  (void)old_index;
  catalog_.selectPage(new_index);
}

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
LayoutCatalog layout_catalog(app.context());
SingletonActivity activity(app, layout_catalog);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
