#include "roo_windows/material3/menu/menu_surface.h"

#include <algorithm>

#include "roo_logging.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/core/theme.h"

namespace roo_windows::material3 {

MenuGroup::MenuGroup(ApplicationContext& context) : Container(context) {}

MenuGroup::~MenuGroup() { clear(); }

void MenuGroup::add(MenuEntry& entry) {
  CHECK(parent() == nullptr);
  CHECK(entry.parent() == nullptr);
  entries_.push_back(&entry);
  attachChild(WidgetRef(entry));
}

void MenuGroup::add(std::unique_ptr<MenuEntry> entry) {
  CHECK(parent() == nullptr);
  CHECK(entry != nullptr);
  CHECK(entry->parent() == nullptr);
  MenuEntry* raw = entry.get();
  entries_.push_back(raw);
  attachChild(WidgetRef(std::move(entry)));
}

void MenuGroup::clear() {
  CHECK(parent() == nullptr || entries_.empty());
  while (!entries_.empty()) {
    MenuEntry* entry = entries_.back();
    entries_.pop_back();
    detachChild(entry);
  }
}

void MenuGroup::paint(PaintContext& ctx) const { (void)ctx; }

Color MenuGroup::background() const { return roo_display::color::Transparent; }

bool MenuGroup::fullyCoversBoundsWithOpaqueColors() const { return false; }

Widget* MenuGroup::preferredFocusChild() {
  for (MenuEntry* entry : entries_) {
    if (!entry->isGone() && entry->isClickable()) return entry;
  }
  return nullptr;
}

int MenuGroup::getChildrenCount() const { return entries_.size(); }

const Widget& MenuGroup::getChild(int idx) const {
  CHECK(idx >= 0 && static_cast<size_t>(idx) < entries_.size());
  return *entries_[idx];
}

Widget& MenuGroup::getChild(int idx) {
  return const_cast<Widget&>(
      static_cast<const MenuGroup&>(*this).getChild(idx));
}

Dimensions MenuGroup::onMeasure(WidthSpec width, HeightSpec height) {
  int16_t measured_width = 0;
  int32_t measured_height = 0;
  for (MenuEntry* entry : entries_) {
    Dimensions size = entry->measure(width, HeightSpec::Unspecified(0));
    measured_width = std::max(measured_width, size.width());
    measured_height += size.height();
  }
  return Dimensions(width.resolveSize(measured_width),
                    height.resolveSize(measured_height));
}

void MenuGroup::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  int32_t y = 0;
  for (MenuEntry* entry : entries_) {
    Dimensions size = entry->measure(WidthSpec::Exactly(rect.width()),
                                     HeightSpec::Unspecified(0));
    entry->layout(Rect(0, y, rect.width() - 1, y + size.height() - 1));
    y += size.height();
  }
}

namespace internal {

MenuGroupStack::MenuGroupStack(ApplicationContext& context)
    : Container(context) {}

MenuGroupStack::~MenuGroupStack() { clearGroups(); }

void MenuGroupStack::addGroup(MenuGroup& group) {
  CHECK(group.parent() == nullptr);
  groups_.push_back(&group);
  attachChild(WidgetRef(group));
}

void MenuGroupStack::addGroup(std::unique_ptr<MenuGroup> group) {
  CHECK(group != nullptr);
  CHECK(group->parent() == nullptr);
  MenuGroup* raw = group.get();
  groups_.push_back(raw);
  attachChild(WidgetRef(std::move(group)));
}

void MenuGroupStack::clearGroups() {
  while (!groups_.empty()) {
    MenuGroup* group = groups_.back();
    groups_.pop_back();
    detachChild(group);
  }
}

int MenuGroupStack::groupCount() const { return groups_.size(); }

MenuGroup& MenuGroupStack::groupAt(int idx) {
  return const_cast<MenuGroup&>(
      static_cast<const MenuGroupStack&>(*this).groupAt(idx));
}

const MenuGroup& MenuGroupStack::groupAt(int idx) const {
  CHECK(idx >= 0 && static_cast<size_t>(idx) < groups_.size());
  return *groups_[idx];
}

void MenuGroupStack::setSeparatorMode(MenuSeparatorMode mode,
                                      ListVariant variant) {
  if (separator_mode_ == mode && variant_ == variant) return;
  setResolvedSeparatorMode(mode, variant);
  requestLayout();
  invalidateInterior();
}

void MenuGroupStack::setResolvedSeparatorMode(MenuSeparatorMode mode,
                                              ListVariant variant) {
  separator_mode_ = mode;
  variant_ = variant;
}

MenuSeparatorMode MenuGroupStack::separatorMode() const {
  return separator_mode_;
}

Color MenuGroupStack::background() const {
  return roo_display::color::Transparent;
}

bool MenuGroupStack::fullyCoversBoundsWithOpaqueColors() const { return false; }

int MenuGroupStack::getChildrenCount() const { return groups_.size(); }

const Widget& MenuGroupStack::getChild(int idx) const {
  CHECK(idx >= 0 && static_cast<size_t>(idx) < groups_.size());
  return *groups_[idx];
}

Widget& MenuGroupStack::getChild(int idx) {
  return const_cast<Widget&>(
      static_cast<const MenuGroupStack&>(*this).getChild(idx));
}

Dimensions MenuGroupStack::onMeasure(WidthSpec width, HeightSpec height) {
  const MenuTokens& tokens = variant_ == ListVariant::kBaseline
                                 ? kBaselineMenuTokens
                                 : kExpressiveStandardMenuTokens;
  MenuSeparatorMode effective = separator_mode_;
  int16_t separator = 0;
  if (effective == MenuSeparatorMode::kDivider) separator = 1;
  if (effective == MenuSeparatorMode::kGap &&
      variant_ == ListVariant::kExpressive) {
    separator = Scaled(tokens.group_gap_dp);
  }
  int16_t measured_width = 0;
  int32_t measured_height = 0;
  for (size_t i = 0; i < groups_.size(); ++i) {
    Dimensions size = groups_[i]->measure(width, HeightSpec::Unspecified(0));
    measured_width = std::max(measured_width, size.width());
    measured_height += size.height();
    if (i + 1 < groups_.size()) measured_height += separator;
  }
  return Dimensions(width.resolveSize(measured_width),
                    height.resolveSize(measured_height));
}

void MenuGroupStack::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  const MenuTokens& tokens = variant_ == ListVariant::kBaseline
                                 ? kBaselineMenuTokens
                                 : kExpressiveStandardMenuTokens;
  int16_t separator = 0;
  if (separator_mode_ == MenuSeparatorMode::kDivider) separator = 1;
  if (separator_mode_ == MenuSeparatorMode::kGap &&
      variant_ == ListVariant::kExpressive) {
    separator = Scaled(tokens.group_gap_dp);
  }
  int32_t y = 0;
  for (MenuGroup* group : groups_) {
    Dimensions size = group->measure(WidthSpec::Exactly(rect.width()),
                                     HeightSpec::Unspecified(0));
    group->layout(Rect(0, y, rect.width() - 1, y + size.height() - 1));
    y += size.height() + separator;
  }
}

void MenuGroupStack::paint(PaintContext& ctx) const {
  if (separator_mode_ != MenuSeparatorMode::kDivider || groups_.size() < 2) {
    return;
  }
  const MenuTokens& tokens = variant_ == ListVariant::kBaseline
                                 ? kBaselineMenuTokens
                                 : kExpressiveStandardMenuTokens;
  Color color = theme().material3Theme().color.resolve(tokens.divider);
  int16_t inset = Scaled(tokens.divider_inset_dp);
  for (size_t i = 0; i + 1 < groups_.size(); ++i) {
    int32_t y = groups_[i]->bounds().yMax() + 1;
    ctx.drawHLine(inset, y, width() - inset - 1, color);
  }
}

MenuPanel::MenuPanel(ApplicationContext& context)
    : Container(context),
      groups_(context),
      viewport_(context, WidgetRef(groups_)),
      policy_() {
  attachChild(WidgetRef(viewport_));
}

MenuPanel::~MenuPanel() {
  viewport_.clearContents();
  detachChild(&viewport_);
  groups_.clearGroups();
}

const MenuTokens& MenuPanel::tokens() const {
  if (policy_.variant == ListVariant::kBaseline) return kBaselineMenuTokens;
  return policy_.color_style == MenuColorStyle::kVibrant
             ? kExpressiveVibrantMenuTokens
             : kExpressiveStandardMenuTokens;
}

void MenuPanel::setPolicy(const MenuPolicy& policy) {
  policy_ = policy;
  groups_.setSeparatorMode(policy.separator_mode, policy.variant);
  invalidateInterior();
  requestLayout();
}

void MenuPanel::addGroup(MenuGroup& group) { groups_.addGroup(group); }

void MenuPanel::addGroup(std::unique_ptr<MenuGroup> group) {
  groups_.addGroup(std::move(group));
}

void MenuPanel::clearGroups() { groups_.clearGroups(); }

int MenuPanel::groupCount() const { return groups_.groupCount(); }

MenuGroup& MenuPanel::groupAt(int idx) { return groups_.groupAt(idx); }

const MenuGroup& MenuPanel::groupAt(int idx) const {
  return groups_.groupAt(idx);
}

bool MenuPanel::isScrolling() const { return scrolling_; }

MenuSeparatorMode MenuPanel::effectiveSeparatorMode() const {
  return groups_.separatorMode();
}

Color MenuPanel::background() const {
  return theme().material3Theme().color.resolve(tokens().panel_container);
}

BorderStyle MenuPanel::getBorderStyle() const {
  uint8_t radius = Scaled(tokens().panel_corner_radius_dp);
  return BorderStyle(radius, 0);
}

Widget* MenuPanel::preferredFocusChild() {
  return groups_.preferredFocusChild();
}

Widget* MenuGroupStack::preferredFocusChild() {
  for (MenuGroup* group : groups_) {
    Widget* preferred = group->preferredFocusChild();
    if (preferred != nullptr) return preferred;
  }
  return nullptr;
}

int MenuPanel::getChildrenCount() const { return 1; }

const Widget& MenuPanel::getChild(int idx) const {
  CHECK_EQ(idx, 0);
  return viewport_;
}

Widget& MenuPanel::getChild(int idx) {
  return const_cast<Widget&>(
      static_cast<const MenuPanel&>(*this).getChild(idx));
}

Dimensions MenuPanel::onMeasure(WidthSpec width, HeightSpec height) {
  // Re-evaluate the configured mode on every measure. A panel that stopped
  // scrolling must be able to recover its requested expressive gaps.
  groups_.setResolvedSeparatorMode(policy_.separator_mode, policy_.variant);
  int16_t max_width = Scaled(tokens().max_width_dp);
  int16_t available_width = width.kind() == UNSPECIFIED
                                ? max_width
                                : std::min<int16_t>(max_width, width.value());
  Dimensions desired = groups_.measure(WidthSpec::AtMost(available_width),
                                       HeightSpec::Unspecified(0));
  int16_t resolved_width =
      std::max<int16_t>(Scaled(tokens().min_width_dp), desired.width());
  resolved_width = width.resolveSize(resolved_width);
  Dimensions final_content = groups_.measure(WidthSpec::Exactly(resolved_width),
                                             HeightSpec::Unspecified(0));
  int32_t resolved_height = height.resolveSize(final_content.height());
  scrolling_ = final_content.height() > resolved_height;
  if (scrolling_ && policy_.separator_mode == MenuSeparatorMode::kGap) {
    // Gaps expose the panel between groups and become visually ambiguous while
    // content moves under a viewport. Scrollable menus therefore use stable
    // one-pixel dividers and remeasure before laying out the viewport.
    groups_.setResolvedSeparatorMode(MenuSeparatorMode::kDivider,
                                     policy_.variant);
    final_content = groups_.measure(WidthSpec::Exactly(resolved_width),
                                    HeightSpec::Unspecified(0));
  }
  viewport_.setVerticalScrollBarPresence(
      scrolling_ ? VerticalScrollBar::Presence::kAlwaysShown
                 : VerticalScrollBar::Presence::kAlwaysHidden);
  viewport_.measure(WidthSpec::Exactly(resolved_width),
                    HeightSpec::Exactly(resolved_height));
  return Dimensions(resolved_width, resolved_height);
}

void MenuPanel::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  viewport_.layout(Rect(0, 0, rect.width() - 1, rect.height() - 1));
}

MenuOverlay::MenuOverlay(ApplicationContext& context) : Container(context) {}

MenuOverlay::~MenuOverlay() { clearPanels(); }

void MenuOverlay::addPanel(MenuPanel& panel, const Rect& bounds) {
  CHECK(panel.parent() == nullptr);
  panels_.push_back(&panel);
  attachChild(WidgetRef(panel), bounds);
}

void MenuOverlay::setPanelBounds(MenuPanel& panel, const Rect& bounds) {
  CHECK(panel.parent() == this);
  panel.layout(bounds);
}

void MenuOverlay::removePanel(MenuPanel& panel) {
  auto found = std::find(panels_.begin(), panels_.end(), &panel);
  CHECK(found != panels_.end());
  panels_.erase(found);
  detachChild(&panel);
}

void MenuOverlay::clearPanels() {
  while (!panels_.empty()) removePanel(*panels_.back());
}

Color MenuOverlay::background() const {
  return roo_display::color::Transparent;
}

bool MenuOverlay::fullyCoversBoundsWithOpaqueColors() const { return false; }

Widget* MenuOverlay::preferredFocusChild() {
  if (panels_.empty()) return nullptr;
  return panels_.back()->preferredFocusChild();
}

void MenuOverlay::paint(PaintContext& ctx) const { (void)ctx; }

int MenuOverlay::getChildrenCount() const { return panels_.size(); }

const Widget& MenuOverlay::getChild(int idx) const {
  CHECK(idx >= 0 && static_cast<size_t>(idx) < panels_.size());
  return *panels_[idx];
}

Widget& MenuOverlay::getChild(int idx) {
  return const_cast<Widget&>(
      static_cast<const MenuOverlay&>(*this).getChild(idx));
}

Dimensions MenuOverlay::onMeasure(WidthSpec width, HeightSpec height) {
  return Dimensions(width.resolveSize(0), height.resolveSize(0));
}

void MenuOverlay::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  (void)rect;
}

static_assert(sizeof(MenuGroup) <= sizeof(Container) + 32,
              "menu group stays within its row-vector footprint");
static_assert(sizeof(MenuOverlay) <= sizeof(Container) + 32,
              "menu overlay stores only a bounded vector handle");

}  // namespace internal
}  // namespace roo_windows::material3
