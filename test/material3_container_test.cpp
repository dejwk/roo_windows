#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/container.h"

namespace roo_windows::material3 {
namespace {

class EmptyContainer : public Material3Container {
 public:
  using Material3Container::Material3Container;

 protected:
  int getChildrenCount() const override { return 0; }
  const Widget& getChild(int) const override { return *this; }
  Widget& getChild(int) override { return *this; }
};

class RoleContainer : public EmptyContainer {
 public:
  using EmptyContainer::EmptyContainer;
  ColorToken containerRole() const override { return role; }

  ColorToken role = ColorToken::kPrimaryContainer;
};

class CustomParent : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  Color background() const override { return Color(127, 12, 34, 56); }
  ColorToken containerRole() const override { return ColorToken::kSurface; }
  bool fullyCoversBoundsWithOpaqueColors() const override { return false; }
};

static_assert(sizeof(EmptyContainer) == sizeof(Container),
              "Material3Container must not add per-instance state.");

// Verifies kNone preserves the generic container fallback without a parent.
TEST(Material3ContainerTest, DetachedContainerUsesFrameworkSurface) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context(env.scheduler(), env.theme(),
                             env.keyboardColorTheme());
  EmptyContainer container(context);
  EXPECT_EQ(ColorToken::kNone, container.containerRole());
  EXPECT_EQ(env.theme().framework.color.surface, container.background());
}

// Verifies inheritance uses the actual parent fill, including alpha, even when
// that fill differs from the parent's semantic color role.
TEST(Material3ContainerTest, NoneInheritsActualParentBackgroundAndRole) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context(env.scheduler(), env.theme(),
                             env.keyboardColorTheme());
  EmptyContainer child(context);
  CustomParent parent(context);
  parent.add(child);
  EXPECT_EQ(parent.background(), child.background());
  EXPECT_NE(env.theme().material3Theme().color.surface, child.background());
  EXPECT_EQ(ColorToken::kSurface, child.effectiveContainerRole());
}

// Verifies an owned role follows both theme and role changes without caching,
// and switching back to kNone restores the parent's custom fill.
TEST(Material3ContainerTest, OwnedRoleUsesCurrentThemeAndOverridesParentFill) {
  roo_scheduler::SchedulingService scheduler;
  Material3Theme material = DefaultTheme().material3Theme();
  Theme theme = DefaultTheme();
  theme.material3_theme = &material;
  Environment env(scheduler, theme);
  ApplicationContext context(env.scheduler(), env.theme(),
                             env.keyboardColorTheme());
  RoleContainer child(context);
  CustomParent parent(context);
  parent.add(child);
  EXPECT_EQ(material.color.primaryContainer, child.background());
  material.color.primaryContainer = Color(12, 34, 56);
  EXPECT_EQ(material.color.primaryContainer, child.background());
  child.role = ColorToken::kSurfaceContainerHigh;
  EXPECT_EQ(material.color.surfaceContainerHigh, child.background());
  EXPECT_EQ(child.role, child.effectiveContainerRole());
  child.role = ColorToken::kNone;
  EXPECT_EQ(parent.background(), child.background());
  EXPECT_EQ(parent.containerRole(), child.effectiveContainerRole());
}

}  // namespace
}  // namespace roo_windows::material3
