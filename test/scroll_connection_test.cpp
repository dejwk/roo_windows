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
