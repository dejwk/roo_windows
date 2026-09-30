#include "roo_windows/core/scroll_connection.h"

#include "gtest/gtest.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {
using namespace internal;
using namespace test_support;
using ScrollConnectionTest = RooWindowsRenderTest;
class ProbeWidget : public Widget {
 public:
  using Widget::Widget;
  ProbeWidget(ProbeWidget&&) = default;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(0, 0);
  }
};

class DragPanel : public SimpleScrollablePanel {
 public:
  using SimpleScrollablePanel::onDrag;
  using SimpleScrollablePanel::onDragStart;
  using SimpleScrollablePanel::SimpleScrollablePanel;
};

class DistanceConsumer : public ScrollConnection {
 public:
  using ScrollConnection::ScrollConnection;
  YDim amount = 0;
  YDim onPreScroll(YDim available) override {
    YDim next = std::max<YDim>(0, std::min<YDim>(48, amount - available));
    YDim consumed = amount - next;
    amount = next;
    return consumed;
  }
  bool canScroll() const override { return true; }
};

// Verifies the connection consumes a prefix, callbacks report content only,
// and reversing expands the connection before moving content.
TEST_F(ScrollConnectionTest, SignedConsumptionAndCallbackCoexistence) {
  ProbeWidget owner(context());
  DragPanel panel(context());
  ColorBoxWidget content(context(), roo_display::color::White,
                         Dimensions(100, 600));
  panel.setContents(content);
  panel.measure(WidthSpec::Exactly(100), HeightSpec::Exactly(200));
  panel.layout(Rect(0, 0, 99, 199));
  auto connection = std::make_shared<DistanceConsumer>(owner, panel);
  context().scrollConnections().install(connection);
  int calls = 0;
  panel.setOnScrollPositionChanged(
      [&](ScrollPosition, ScrollPosition) { ++calls; });
  panel.onDragStart(0, 0);
  panel.onDrag(0, 0, 0, -60);
  EXPECT_EQ(48, connection->amount);
  EXPECT_EQ(-12, panel.getScrollPosition().y);
  EXPECT_EQ(1, calls);
  panel.onDrag(0, 0, 0, 10);
  EXPECT_EQ(38, connection->amount);
  EXPECT_EQ(-12, panel.getScrollPosition().y);
  EXPECT_EQ(1, calls);
}

// Verifies a connection remains scrollable even when all body content fits.
TEST_F(ScrollConnectionTest, ShortContentStillConsumesBarTravel) {
  ProbeWidget owner(context());
  DragPanel panel(context());
  ColorBoxWidget content(context(), roo_display::color::White,
                         Dimensions(100, 20));
  panel.setContents(content);
  panel.measure(WidthSpec::Exactly(100), HeightSpec::Exactly(200));
  panel.layout(Rect(0, 0, 99, 199));
  auto connection = std::make_shared<DistanceConsumer>(owner, panel);
  context().scrollConnections().install(connection);
  panel.onDragStart(0, 0);
  panel.onDrag(0, 0, 0, -20);
  EXPECT_EQ(20, connection->amount);
  EXPECT_EQ(0, panel.getScrollPosition().y);
}

// Verifies both endpoint indexes, busy protection, and atomic failed rebinding.
TEST_F(ScrollConnectionTest, RegistrationAndDispatchLifetime) {
  ProbeWidget owner(context());
  ProbeWidget other(context());
  SimpleScrollablePanel panel(context());
  SimpleScrollablePanel horizontal(
      context(), SimpleScrollablePanel::Direction::kHorizontal);
  auto link = std::make_shared<ScrollConnection>(owner, panel);
  EXPECT_EQ(nullptr, context().scrollConnectionsIfPresent());
  auto& registry = context().scrollConnections();
  EXPECT_EQ(ScrollConnectionStatus::kSuccess, registry.install(link));
  EXPECT_EQ(link, registry.find(owner));
  EXPECT_EQ(link, registry.find(panel));
  EXPECT_EQ(ScrollConnectionStatus::kAlreadyConnected,
            registry.install(std::make_shared<ScrollConnection>(other, panel)));
  EXPECT_EQ(
      ScrollConnectionStatus::kUnsupportedAxis,
      registry.install(std::make_shared<ScrollConnection>(owner, horizontal)));
  {
    ScrollConnection::Dispatch dispatch(link);
    EXPECT_EQ(ScrollConnectionStatus::kBusy, registry.remove(owner));
    EXPECT_EQ(ScrollConnectionStatus::kBusy, registry.install(link));
  }
  EXPECT_EQ(link, registry.find(panel));
  EXPECT_EQ(ScrollConnectionStatus::kSuccess, registry.remove(owner));
  EXPECT_EQ(nullptr, registry.find(panel));
}

// Verifies either endpoint's destruction removes both borrowed identities.
TEST_F(ScrollConnectionTest, EndpointDestruction) {
  ProbeWidget owner(context());
  {
    SimpleScrollablePanel panel(context());
    context().scrollConnections().install(
        std::make_shared<ScrollConnection>(owner, panel));
  }
  EXPECT_EQ(nullptr, ScrollConnectionRegistry::Find(owner));
  SimpleScrollablePanel panel(context());
  {
    ProbeWidget temporary(context());
    context().scrollConnections().install(
        std::make_shared<ScrollConnection>(temporary, panel));
  }
  EXPECT_EQ(nullptr, ScrollConnectionRegistry::Find(panel));
}

// Verifies moving an endpoint disconnects rather than transferring derived
// identity.
TEST_F(ScrollConnectionTest, MoveDisconnects) {
  ProbeWidget owner(context());
  SimpleScrollablePanel panel(context());
  context().scrollConnections().install(
      std::make_shared<ScrollConnection>(owner, panel));
  ProbeWidget moved(std::move(owner));
  EXPECT_EQ(nullptr, ScrollConnectionRegistry::Find(panel));
  EXPECT_EQ(nullptr, ScrollConnectionRegistry::Find(moved));
}

// Verifies context teardown releases records before borrowed endpoints die.
TEST_F(ScrollConnectionTest, ContextLifetimeAndForeignEndpoint) {
  ProbeWidget owner(context());
  std::unique_ptr<ProbeWidget> foreign_owner;
  std::unique_ptr<SimpleScrollablePanel> foreign_panel;
  std::weak_ptr<ScrollConnection> lifetime;
  {
    ApplicationContext foreign(scheduler_, context().theme(),
                               context().keyboardColorTheme());
    foreign_owner = std::make_unique<ProbeWidget>(foreign);
    foreign_panel = std::make_unique<SimpleScrollablePanel>(foreign);
    auto invalid = std::make_shared<ScrollConnection>(owner, *foreign_panel);
    EXPECT_EQ(ScrollConnectionStatus::kDifferentContext,
              context().scrollConnections().install(invalid));
    auto valid =
        std::make_shared<ScrollConnection>(*foreign_owner, *foreign_panel);
    lifetime = valid;
    EXPECT_EQ(ScrollConnectionStatus::kSuccess,
              foreign.scrollConnections().install(valid));
  }
  EXPECT_TRUE(lifetime.expired());
  foreign_panel.reset();
  foreign_owner.reset();
}
}  // namespace
}  // namespace roo_windows
