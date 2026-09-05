#include "roo_windows/material3/menu/menu.h"

#include <algorithm>
#include <cstdio>
#include <new>

#include "roo_display/ui/text_label.h"
#include "roo_logging.h"
#include "roo_windows/core/display_window.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/core/task.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/transient_surface_host.h"
#include "roo_windows/material3/menu/menu_geometry.h"
#include "roo_windows/material3/menu/menu_surface.h"
#include "roo_windows/material3/menu/menu_tokens.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3 {
namespace {

const internal::MenuTokens& TokensFor(const ListEntryVisualContext& context) {
  if (context.variant == ListVariant::kBaseline) {
    return internal::kBaselineMenuTokens;
  }
  return internal::kExpressiveStandardMenuTokens;
}

int16_t ShortcutWidth(roo::string_view shortcut) {
  if (shortcut.empty()) return 0;
  return text_style_label_large()
      .font()
      .getHorizontalStringMetrics(shortcut,
                                  text_style_label_large().fontOptions())
      .advance();
}

MenuShowResult MapStartResult(PresentationStartResult result) {
  switch (result) {
    case PresentationStartResult::kStarted:
      return MenuShowResult::kShown;
    case PresentationStartResult::kHostBusy:
      return MenuShowResult::kHostBusy;
    case PresentationStartResult::kReentrantReplacement:
      return MenuShowResult::kReentrantReplacement;
    case PresentationStartResult::kInteractionOwnerUnavailable:
      return MenuShowResult::kInteractionOwnerUnavailable;
    case PresentationStartResult::kSurfaceUnavailable:
      return MenuShowResult::kSurfaceUnavailable;
  }
  return MenuShowResult::kSurfaceUnavailable;
}

class FrozenMenuTriggerPin final : public PresentationPin {
 public:
  FrozenMenuTriggerPin(
      const ::roo_windows::internal::TransientSourceGeometry& geometry,
      uint16_t corner_radius, uint32_t overlay_argb, uint8_t overlay_opacity)
      : bounds_(geometry.bounds_in_window),
        clip_(geometry.visible_bounds_in_window),
        corner_radius_(corner_radius),
        color_(roo_display::Color(overlay_argb).withA(overlay_opacity)) {}

 protected:
  Rect boundsInWindow() const override { return bounds_; }
  Rect clipBoundsInWindow() const override { return clip_; }

  void paint(PaintContext& ctx) const override {
    Rect settled =
        Rect::Intersect(Rect::Intersect(bounds_, clip_), ctx.localClip());
    if (settled.empty()) return;
    PaintDecoration decoration;
    decoration.bounds = settled;
    decoration.background = color_;
    decoration.corner_radii = {static_cast<uint8_t>(corner_radius_),
                               static_cast<uint8_t>(corner_radius_),
                               static_cast<uint8_t>(corner_radius_),
                               static_cast<uint8_t>(corner_radius_)};
    ctx.fillRect(settled, color_);
    ctx.addDecoration(decoration);
    ctx.addExclusion(settled);
  }

 private:
  Rect bounds_;
  Rect clip_;
  uint16_t corner_radius_;
  roo_display::Color color_;
};

}  // namespace

struct StandardMenuItem::TrailingPayload {
  roo::string_view shortcut;
  const roo_display::Drawable* icon = nullptr;
  MenuBadgeSpec badge;
  char numeric_badge[8] = {};
};

struct MenuEntry::AdornmentState {
  MenuTrailingAffordances content;
  Badge badge;
  Rect icon_bounds;
  Rect badge_anchor;
};

StandardMenuItem::StandardMenuItem(const StandardMenuItemInit& init)
    : headline_(init.headline),
      supporting_(init.supporting),
      leading_(init.leading),
      trailing_(),
      enabled_(init.enabled),
      selectable_(init.selectable),
      selected_(init.selected) {}

StandardMenuItem::~StandardMenuItem() = default;

roo::string_view StandardMenuItem::headlineText() const { return headline_; }

roo::string_view StandardMenuItem::supportingText() const {
  return supporting_;
}

Widget* StandardMenuItem::leading() { return leading_; }

const Widget* StandardMenuItem::leading() const { return leading_; }

bool StandardMenuItem::isEnabled() const { return enabled_; }

bool StandardMenuItem::isSelectable() const { return selectable_; }

bool StandardMenuItem::isSelected() const { return selected_; }

void StandardMenuItem::setSelectedFromMenu(bool selected) {
  selected_ = selected;
}

MenuTrailingAffordances StandardMenuItem::trailingAffordances() const {
  if (!trailing_) return {};
  return MenuTrailingAffordances{trailing_->shortcut, trailing_->icon,
                                 trailing_->badge};
}

void StandardMenuItem::setSelected(bool selected) { selected_ = selected; }

void StandardMenuItem::setEnabled(bool enabled) { enabled_ = enabled; }

StandardMenuItem::TrailingPayload& StandardMenuItem::ensureTrailingPayload() {
  if (!trailing_) trailing_.reset(new TrailingPayload());
  return *trailing_;
}

void StandardMenuItem::releaseEmptyTrailingPayload() {
  if (!trailing_) return;
  if (trailing_->shortcut.empty() && trailing_->icon == nullptr &&
      trailing_->badge.mode == BadgeMode::kHidden) {
    trailing_.reset();
  }
}

void StandardMenuItem::setShortcut(roo_display::StringView shortcut) {
  ensureTrailingPayload().shortcut = shortcut;
  releaseEmptyTrailingPayload();
}

void StandardMenuItem::clearShortcut() {
  if (trailing_) trailing_->shortcut = {};
  releaseEmptyTrailingPayload();
}

void StandardMenuItem::setBadgeDot() {
  TrailingPayload& payload = ensureTrailingPayload();
  payload.badge = MenuBadgeSpec{BadgeMode::kDot, {}};
}

void StandardMenuItem::setBadgeText(roo::string_view text) {
  TrailingPayload& payload = ensureTrailingPayload();
  payload.badge = MenuBadgeSpec{BadgeMode::kText, text};
}

void StandardMenuItem::setBadgeValue(unsigned int value) {
  TrailingPayload& payload = ensureTrailingPayload();
  if (value > 999) {
    std::snprintf(payload.numeric_badge, sizeof(payload.numeric_badge), "999+");
  } else {
    std::snprintf(payload.numeric_badge, sizeof(payload.numeric_badge), "%u",
                  value);
  }
  payload.badge = MenuBadgeSpec{BadgeMode::kText, payload.numeric_badge};
}

void StandardMenuItem::clearBadge() {
  if (trailing_) trailing_->badge = {};
  releaseEmptyTrailingPayload();
}

void StandardMenuItem::setTrailingIcon(const roo_display::Drawable* icon) {
  if (icon == nullptr) {
    clearTrailingIcon();
    return;
  }
  ensureTrailingPayload().icon = icon;
}

void StandardMenuItem::clearTrailingIcon() {
  if (trailing_) trailing_->icon = nullptr;
  releaseEmptyTrailingPayload();
}

MenuEntry::MenuEntry(ApplicationContext& context)
    : ListEntry(context), adornments_() {}

MenuEntry::~MenuEntry() { prepareForItemDestruction(); }

void MenuEntry::setMenuItem(MenuItem& item) {
  CHECK(menu_ == nullptr);
  ListEntry::setItem(item);
  syncAdornments();
  ListEntryVisualContext visual = visualContext();
  visual.enabled = item.isEnabled();
  visual.selected = item.isSelected();
  setVisualContext(visual);
}

MenuItem* MenuEntry::menuItem() {
  return static_cast<MenuItem*>(ListEntry::item());
}

const MenuItem* MenuEntry::menuItem() const {
  return static_cast<const MenuItem*>(ListEntry::item());
}

bool MenuEntry::isClickable() const {
  const MenuItem* bound = menuItem();
  return menu_ != nullptr && bound != nullptr && bound->isEnabled();
}

void MenuEntry::prepareForItemDestruction() {
  unbindFromMenu();
  adornments_.reset();
  ListEntry::clearItem();
}

void MenuEntry::onSingleTapUp(XDim x, YDim y) {
  if (getMainWindow() != nullptr) Widget::onSingleTapUp(x, y);
  if (menu_ == nullptr) return;
  Menu* owner = menu_;
  uint8_t level = level_;
  uint16_t row = row_;
  uint16_t generation = level_generation_;
  suppress_next_click_dispatch_ = true;
  owner->invokeEntry(*this, level, row, generation);
}

void MenuEntry::onClicked() {
  if (suppress_next_click_dispatch_) {
    suppress_next_click_dispatch_ = false;
    return;
  }
  if (menu_ != nullptr) {
    menu_->invokeEntry(*this, level_, row_, level_generation_);
  }
}

void MenuEntry::bindToMenu(Menu& owner, uint8_t level, uint16_t row,
                           uint16_t generation) {
  CHECK(menu_ == nullptr || menu_ == &owner);
  menu_ = &owner;
  level_ = level;
  row_ = row;
  level_generation_ = generation;
  suppress_next_click_dispatch_ = false;
  ListEntryVisualContext visual = visualContext();
  const MenuItem* bound = menuItem();
  visual.enabled = bound != nullptr && bound->isEnabled();
  visual.selected = bound != nullptr && bound->isSelected();
  setVisualContext(visual);
  syncAdornments();
}

void MenuEntry::unbindFromMenu() {
  menu_ = nullptr;
  level_ = 0;
  row_ = 0;
  level_generation_ = 0;
  suppress_next_click_dispatch_ = false;
}

void MenuEntry::syncAdornments() {
  const MenuItem* bound = menuItem();
  MenuTrailingAffordances content = bound == nullptr
                                        ? MenuTrailingAffordances{}
                                        : bound->trailingAffordances();
  if (content.shortcut.empty() && content.icon == nullptr &&
      content.badge.mode == BadgeMode::kHidden &&
      !(bound != nullptr && bound->isSelectable()) &&
      !(bound != nullptr && bound->hasSubmenu())) {
    adornments_.reset();
    return;
  }
  if (!adornments_) adornments_.reset(new AdornmentState());
  adornments_->content = content;
  switch (content.badge.mode) {
    case BadgeMode::kHidden:
      adornments_->badge.hide();
      break;
    case BadgeMode::kDot:
      adornments_->badge.setDot();
      break;
    case BadgeMode::kText:
      adornments_->badge.setText(content.badge.text);
      break;
  }
}

int16_t MenuEntry::trailingLaneWidth() const {
  if (!adornments_) return 0;
  const internal::MenuTokens& tokens = TokensFor(visualContext());
  int16_t width = 0;
  int count = 0;
  const MenuItem* bound = menuItem();
  if (bound != nullptr && bound->isSelectable()) {
    width += Scaled(tokens.icon_size_dp);
    ++count;
  }
  if (!adornments_->content.shortcut.empty()) {
    width += ShortcutWidth(adornments_->content.shortcut);
    ++count;
  }
  if (adornments_->content.icon != nullptr) {
    width += Scaled(tokens.icon_size_dp);
    ++count;
  }
  if (adornments_->content.badge.mode != BadgeMode::kHidden) {
    width += Scaled(24);
    ++count;
  }
  if (bound != nullptr && bound->hasSubmenu()) {
    width += Scaled(tokens.icon_size_dp);
    ++count;
  }
  if (count > 1) width += (count - 1) * Scaled(tokens.trailing_gap_dp);
  return width == 0 ? 0 : width + Scaled(tokens.trailing_gap_dp);
}

Dimensions MenuEntry::onMeasure(WidthSpec width, HeightSpec height) {
  syncAdornments();
  int16_t lane = trailingLaneWidth();
  WidthSpec content_width = width;
  if (width.kind() == AT_MOST) {
    content_width =
        WidthSpec::AtMost(std::max<int16_t>(0, width.value() - lane));
  } else if (width.kind() == EXACTLY) {
    content_width =
        WidthSpec::Exactly(std::max<int16_t>(0, width.value() - lane));
  }
  Dimensions base = ListEntry::onMeasure(content_width, height);
  int16_t resolved_width = base.width() + lane;
  if (width.kind() == EXACTLY) resolved_width = width.value();
  if (width.kind() == AT_MOST)
    resolved_width = std::min(resolved_width, width.value());
  const internal::MenuTokens& tokens = TokensFor(visualContext());
  return Dimensions(
      std::max<int16_t>(Scaled(tokens.min_width_dp), resolved_width),
      std::max<int16_t>(Scaled(tokens.min_item_height_dp), base.height()));
}

void MenuEntry::onLayout(bool changed, const Rect& rect) {
  int16_t lane = trailingLaneWidth();
  Rect content_rect(rect.xMin(), rect.yMin(),
                    lane > 0
                        ? std::max<XDim>(rect.xMin() - 1, rect.xMax() - lane)
                        : rect.xMax(),
                    rect.yMax());
  ListEntry::onLayout(changed, content_rect);

  if (!adornments_) return;
  const internal::MenuTokens& tokens = TokensFor(visualContext());
  int16_t size = Scaled(tokens.icon_size_dp);
  int16_t end = rect.width() - Scaled(tokens.horizontal_padding_dp) - 1;
  int16_t top = std::max<int16_t>(0, (rect.height() - size) / 2);
  adornments_->icon_bounds = Rect(end - size + 1, top, end, top + size - 1);
  adornments_->badge_anchor = adornments_->icon_bounds;
  adornments_->badge.layoutForIcon(adornments_->badge_anchor);
}

void MenuEntry::paintWidgetContents(PaintContext& ctx) {
  ListEntry::paintWidgetContents(ctx);
  if (!adornments_) return;

  const Theme& current_theme = theme();
  Color color = current_theme.material3Theme().color.onSurfaceVariant;
  Rect cursor = adornments_->icon_bounds;
  const MenuItem* bound = menuItem();

  if (bound != nullptr && bound->hasSubmenu()) {
    int16_t mid_x = (cursor.xMin() + cursor.xMax()) / 2;
    int16_t mid_y = (cursor.yMin() + cursor.yMax()) / 2;
    int16_t arm = std::max<int16_t>(2, Scaled(4));
    for (int16_t i = 0; i <= arm; ++i) {
      ctx.fillRect(mid_x - arm + i, mid_y - arm + i, mid_x - arm + i,
                   mid_y - arm + i, color);
      ctx.fillRect(mid_x - arm + i, mid_y + arm - i, mid_x - arm + i,
                   mid_y + arm - i, color);
    }
    ctx.addExclusion(cursor);
  } else if (adornments_->content.icon != nullptr) {
    ctx.drawTiled(*adornments_->content.icon, cursor,
                  roo_display::kCenter | roo_display::kMiddle, false);
    ctx.addExclusion(cursor);
  }

  if (bound != nullptr && bound->isSelectable() && bound->isSelected()) {
    int16_t x = cursor.xMin() - Scaled(28);
    int16_t y = (cursor.yMin() + cursor.yMax()) / 2;
    ctx.drawHLine(x, y, x + Scaled(4), color);
    ctx.drawHLine(x + Scaled(4), y, x + Scaled(9), color);
    ctx.addExclusion(Rect(x, y - Scaled(5), x + Scaled(9), y + Scaled(5)));
  }

  if (!adornments_->content.shortcut.empty()) {
    const TextStyle& style = text_style_label_large();
    roo_display::StringViewLabel label(adornments_->content.shortcut,
                                       style.font(), color,
                                       style.fontOptions());
    Rect text_bounds(Scaled(12), 0, Scaled(12) + label.extents().width() - 1,
                     height() - 1);
    ctx.drawTiled(label, text_bounds,
                  roo_display::kRight | roo_display::kMiddle, false);
    ctx.addExclusion(text_bounds);
  }

  if (adornments_->badge.visible()) {
    adornments_->badge.paint(ctx, current_theme);
  }
}

static_assert(sizeof(MenuEntry) <= sizeof(ListEntry) + 24,
              "menu rows stay within the designed adornment/binding delta");

class Menu::Impl {
 public:
  static constexpr uint8_t kMaxLevels = 4;

  class Registration final : public TransientPresentationRegistration {
   public:
    explicit Registration(Menu& owner) : owner_(owner) {}

    void CancelPresentation() { cancel(); }
    void DisablePresentationInput() { disableHostedInput(); }

   protected:
    void detachPresentation(PresentationFinishReason reason) override {
      (void)reason;
      owner_.unbindAllEntries();
    }

    void onFinished(PresentationFinishReason reason) override {
      Impl& impl = *owner_.impl_;
      impl.interaction_owner = nullptr;
      impl.focus_scope.clearRememberedFocus();
      owner_.onFinished(reason);
    }

    BackResult onBackRequested(BackSource source) override {
      (void)source;
      finish(PresentationFinishReason::kBack);
      return BackResult::kHandled;
    }

    void onOutsideInteraction() override {
      finish(PresentationFinishReason::kOutsideInteraction);
    }

   private:
    Menu& owner_;
  };

  class Preparation final
      : public ::roo_windows::internal::TransientSurfacePreparation {
   public:
    Preparation(Impl& impl, ::roo_windows::Task& owner, const Rect& anchor,
                MenuPlacement placement)
        : impl_(impl), owner_(owner), anchor_(anchor), placement_(placement) {}

   private:
    bool createAndResolveBounds(Rect& root_bounds_in_window) override {
      MainWindow& window = owner_.window().root();
      if (window.bounds().empty() || impl_.root_panel.groupCount() == 0) {
        return false;
      }
      ++impl_.level_generation[0];
      if (impl_.level_generation[0] == 0) ++impl_.level_generation[0];
      impl_.presenter.bindRootEntries();
      const internal::MenuTokens& tokens = impl_.RootTokens();
      int16_t margin = Scaled(tokens.viewport_margin_dp);
      if (window.width() <= 2 * margin || window.height() <= 2 * margin) {
        return false;
      }
      Rect viewport(margin, margin, window.width() - margin - 1,
                    window.height() - margin - 1);
      Dimensions desired =
          impl_.root_panel.measure(WidthSpec::AtMost(viewport.width()),
                                   HeightSpec::AtMost(viewport.height()));
      internal::MenuPlacementResult placement =
          internal::ResolveRootMenuPlacement(viewport, anchor_, desired,
                                             placement_,
                                             impl_.policy.layout_direction);
      if (placement.bounds.empty()) return false;
      impl_.root_panel.measure(WidthSpec::Exactly(placement.bounds.width()),
                               HeightSpec::Exactly(placement.bounds.height()));
      impl_.root_panel.layout(placement.bounds);
      impl_.overlay.measure(WidthSpec::Exactly(window.width()),
                            HeightSpec::Exactly(window.height()));
      impl_.overlay.layout(Rect(0, 0, window.width() - 1, window.height() - 1));
      impl_.anchor = anchor_;
      impl_.placement = placement_;
      root_bounds_in_window = window.bounds();
      return true;
    }

    void deleteAfterFailedAdmission() override {
      impl_.presenter.unbindAllEntries();
    }

    Impl& impl_;
    ::roo_windows::Task& owner_;
    Rect anchor_;
    MenuPlacement placement_;
  };

  explicit Impl(Menu& owner, ApplicationContext& context)
      : policy(),
        presenter(owner),
        root_panel(context),
        overlay(context),
        registration(owner) {
    root_panel.setPolicy(policy);
    overlay.addPanel(root_panel, Rect());
  }

  const internal::MenuTokens& RootTokens() const {
    if (policy.variant == ListVariant::kBaseline) {
      return internal::kBaselineMenuTokens;
    }
    return policy.color_style == MenuColorStyle::kVibrant
               ? internal::kExpressiveVibrantMenuTokens
               : internal::kExpressiveStandardMenuTokens;
  }

  bool RelayoutRoot(const Rect& new_anchor, MenuPlacement new_placement) {
    if (interaction_owner == nullptr) return false;
    MainWindow& window = interaction_owner->window().root();
    const internal::MenuTokens& tokens = RootTokens();
    int16_t margin = Scaled(tokens.viewport_margin_dp);
    if (window.width() <= 2 * margin || window.height() <= 2 * margin) {
      return false;
    }
    Rect viewport(margin, margin, window.width() - margin - 1,
                  window.height() - margin - 1);
    Dimensions desired =
        root_panel.measure(WidthSpec::AtMost(viewport.width()),
                           HeightSpec::AtMost(viewport.height()));
    internal::MenuPlacementResult resolved = internal::ResolveRootMenuPlacement(
        viewport, new_anchor, desired, new_placement, policy.layout_direction);
    if (resolved.bounds.empty()) return false;
    root_panel.measure(WidthSpec::Exactly(resolved.bounds.width()),
                       HeightSpec::Exactly(resolved.bounds.height()));
    overlay.setPanelBounds(root_panel, resolved.bounds);
    anchor = new_anchor;
    placement = new_placement;
    return true;
  }

  void AddLevelGroup(uint8_t level, uint16_t generation, MenuGroup& group) {
    CHECK(population_active);
    CHECK_EQ(level, population_level);
    CHECK_EQ(generation, level_generation[level]);
    CHECK(level < kMaxLevels);
    CHECK(level_panels[level] != nullptr);
    level_panels[level]->addGroup(group);
  }

  void AddLevelGroup(uint8_t level, uint16_t generation,
                     std::unique_ptr<MenuGroup> group) {
    CHECK(population_active);
    CHECK_EQ(level, population_level);
    CHECK_EQ(generation, level_generation[level]);
    CHECK(level < kMaxLevels);
    CHECK(level_panels[level] != nullptr);
    level_panels[level]->addGroup(std::move(group));
  }

  MenuPolicy policy;
  Menu& presenter;
  internal::MenuPanel root_panel;
  internal::MenuOverlay overlay;
  internal::MenuPanel* level_panels[kMaxLevels] = {&root_panel, nullptr,
                                                   nullptr, nullptr};
  uint16_t level_generation[kMaxLevels] = {1, 1, 1, 1};
  uint8_t population_level = 0;
  bool population_active = false;
  FocusScope focus_scope;
  ::roo_windows::Task* interaction_owner = nullptr;
  Rect anchor;
  MenuPlacement placement = MenuPlacement::kBelowStart;

  // Must remain last so presenter destruction vacates the shared host before
  // any borrowed root structure or focus record can die.
  Registration registration;
};

MenuLevelBuilder::MenuLevelBuilder(Menu& owner, uint8_t level,
                                   uint16_t generation)
    : owner_(&owner), generation_(generation), level_(level) {}

void MenuLevelBuilder::addGroup(MenuGroup& group) {
  owner_->impl_->AddLevelGroup(level_, generation_, group);
}

void MenuLevelBuilder::addGroup(std::unique_ptr<MenuGroup> group) {
  owner_->impl_->AddLevelGroup(level_, generation_, std::move(group));
}

Menu::Menu(ApplicationContext& context)
    : impl_(new Impl(*this, context)), admission_in_progress_(false) {}

Menu::~Menu() { prepareForDerivedDestruction(); }

void Menu::setPolicy(const MenuPolicy& policy) {
  CHECK(!admission_in_progress_);
  CHECK(!impl_->registration.isActive());
  impl_->policy = policy;
  impl_->root_panel.setPolicy(policy);
}

void Menu::addGroup(MenuGroup& group) {
  CHECK(!admission_in_progress_);
  CHECK(!impl_->registration.isActive());
  impl_->focus_scope.clearRememberedFocus();
  impl_->root_panel.addGroup(group);
}

void Menu::addGroup(std::unique_ptr<MenuGroup> group) {
  CHECK(!admission_in_progress_);
  CHECK(!impl_->registration.isActive());
  impl_->focus_scope.clearRememberedFocus();
  impl_->root_panel.addGroup(std::move(group));
}

void Menu::clearGroups() {
  CHECK(!admission_in_progress_);
  CHECK(!impl_->registration.isActive());
  impl_->focus_scope.clearRememberedFocus();
  impl_->root_panel.clearGroups();
}

void Menu::bindRootEntries() {
  uint16_t row_index = 0;
  for (int group_index = 0; group_index < impl_->root_panel.groupCount();
       ++group_index) {
    MenuGroup& group = impl_->root_panel.groupAt(group_index);
    for (MenuEntry* entry : group.entries_) {
      entry->bindToMenu(*this, 0, row_index++, impl_->level_generation[0]);
      ListEntryVisualContext visual = entry->visualContext();
      visual.variant = impl_->policy.variant;
      visual.style = impl_->policy.variant == ListVariant::kExpressive
                         ? ListStyle::kSegmented
                         : ListStyle::kStandard;
      if (impl_->policy.selection_mode == SelectionMode::kNone &&
          entry->menuItem() != nullptr && entry->menuItem()->isSelectable()) {
        visual.selected = false;
        LOG(WARNING) << "Selectable menu item bound in SelectionMode::kNone";
      }
      entry->setVisualContext(visual);
    }
  }
}

void Menu::unbindAllEntries() {
  for (int group_index = 0; group_index < impl_->root_panel.groupCount();
       ++group_index) {
    MenuGroup& group = impl_->root_panel.groupAt(group_index);
    for (MenuEntry* entry : group.entries_) entry->unbindFromMenu();
  }
}

void Menu::invokeEntry(MenuEntry& entry, uint8_t level, uint16_t row,
                       uint16_t generation) {
  if (!impl_->registration.isActive() || level != 0 ||
      generation != impl_->level_generation[0] || entry.menu_ != this) {
    return;
  }

  MenuEntry* resolved = nullptr;
  uint16_t current_row = 0;
  for (int group_index = 0; group_index < impl_->root_panel.groupCount();
       ++group_index) {
    MenuGroup& group = impl_->root_panel.groupAt(group_index);
    for (MenuEntry* candidate : group.entries_) {
      if (current_row++ == row) resolved = candidate;
    }
  }
  if (resolved != &entry) return;
  MenuItem* item = entry.menuItem();
  if (item == nullptr || !item->isEnabled()) return;
  if (item->hasSubmenu()) return;

  const bool selectable = item->isSelectable() &&
                          impl_->policy.selection_mode != SelectionMode::kNone;
  const bool was_selected = item->isSelected();
  const SelectionMode selection_mode = impl_->policy.selection_mode;
  const MenuLeafDismissal dismissal = item->leafDismissal();

  if (selectable && selection_mode == SelectionMode::kSingle) {
    for (int group_index = 0; group_index < impl_->root_panel.groupCount();
         ++group_index) {
      MenuGroup& group = impl_->root_panel.groupAt(group_index);
      for (MenuEntry* candidate : group.entries_) {
        MenuItem* candidate_item = candidate->menuItem();
        if (candidate_item != nullptr && candidate_item->isSelectable()) {
          candidate_item->setSelectedFromMenu(candidate == &entry);
        }
        candidate->refreshFromItem();
        ListEntryVisualContext visual = candidate->visualContext();
        visual.enabled =
            candidate_item != nullptr && candidate_item->isEnabled();
        visual.selected =
            candidate_item != nullptr && candidate_item->isSelected();
        candidate->setVisualContext(visual);
        candidate->syncAdornments();
      }
    }
  } else if (selectable && selection_mode == SelectionMode::kMultiple) {
    item->setSelectedFromMenu(!was_selected);
    entry.refreshFromItem();
    ListEntryVisualContext visual = entry.visualContext();
    visual.enabled = item->isEnabled();
    visual.selected = item->isSelected();
    entry.setVisualContext(visual);
    entry.syncAdornments();
  }

  item->onInvoked();
  // Invocation is application code: it may replace the active presenter or
  // otherwise invalidate this row. Revalidate registration and generation
  // before consulting presenter state, and never dereference `item` again.
  if (!impl_->registration.isActive() ||
      generation != impl_->level_generation[0] || entry.menu_ != this) {
    return;
  }

  bool should_dismiss = dismissal == MenuLeafDismissal::kDismiss;
  if (dismissal == MenuLeafDismissal::kDefault) {
    should_dismiss =
        !(selectable && selection_mode == SelectionMode::kMultiple);
  }
  if (should_dismiss) {
    impl_->registration.finish(PresentationFinishReason::kAction);
  }
}

MenuShowResult Menu::show(::roo_windows::Task& interaction_owner,
                          const Widget& placement_source,
                          MenuPlacement placement,
                          const MenuTriggerPaintSource* trigger) {
  if (impl_->registration.isActive()) return MenuShowResult::kAlreadyPresented;
  if (admission_in_progress_) return MenuShowResult::kReentrantReplacement;
  admission_in_progress_ = true;
  ::roo_windows::internal::TransientSourceGeometry placement_geometry;
  if (!::roo_windows::internal::CaptureTransientSourceGeometry(
          interaction_owner, placement_source, placement_geometry)) {
    admission_in_progress_ = false;
    return MenuShowResult::kAnchorUnavailable;
  }
  MenuShowResult result =
      showCaptured(interaction_owner, placement_geometry.bounds_in_window,
                   placement, trigger);
  admission_in_progress_ = false;
  return result;
}

MenuShowResult Menu::showFromRect(::roo_windows::Task& interaction_owner,
                                  const Rect& bounds_in_window,
                                  MenuPlacement placement,
                                  const MenuTriggerPaintSource* trigger) {
  if (impl_->registration.isActive()) return MenuShowResult::kAlreadyPresented;
  if (admission_in_progress_) return MenuShowResult::kReentrantReplacement;
  admission_in_progress_ = true;
  MenuShowResult result =
      showCaptured(interaction_owner, bounds_in_window, placement, trigger);
  admission_in_progress_ = false;
  return result;
}

MenuShowResult Menu::showCaptured(::roo_windows::Task& interaction_owner,
                                  const Rect& bounds_in_window,
                                  MenuPlacement placement,
                                  const MenuTriggerPaintSource* trigger) {
  if (bounds_in_window.empty()) return MenuShowResult::kAnchorUnavailable;

  ::roo_windows::internal::TransientSourceGeometry trigger_geometry;
  bool has_trigger = trigger != nullptr &&
                     ::roo_windows::internal::CaptureTransientSourceGeometry(
                         interaction_owner, trigger->widget, trigger_geometry);
  const TransientSurfaceSpec spec{TransientBarrierPaint::kTransparent,
                                  TransientAdmissionPolicy::kReplaceReplaceable,
                                  OutsideInteractionPolicy::kPresenterHandled,
                                  TransientPresentationPolicy(true, true),
                                  true};
  Impl::Preparation preparation(*impl_, interaction_owner, bounds_in_window,
                                placement);
  PresentationStartResult started =
      ::roo_windows::internal::GetTransientSurfaceHost(interaction_owner)
          .showPrepared(impl_->registration, interaction_owner, impl_->overlay,
                        impl_->focus_scope, spec, preparation);
  if (started != PresentationStartResult::kStarted) {
    return MapStartResult(started);
  }
  impl_->interaction_owner = &interaction_owner;
  if (has_trigger) {
    std::unique_ptr<PresentationPin> pin(
        new (std::nothrow) FrozenMenuTriggerPin(
            trigger_geometry, trigger->corner_radius, trigger->overlay_argb,
            trigger->overlay_opacity));
    ::roo_windows::internal::GetTransientSurfaceHost(interaction_owner)
        .showPresentationPin(impl_->registration, std::move(pin));
  }
  return MenuShowResult::kShown;
}

bool Menu::reanchor(const Widget& placement_source, MenuPlacement placement,
                    const MenuTriggerPaintSource* trigger) {
  if (!impl_->registration.isActive() || impl_->interaction_owner == nullptr) {
    return false;
  }
  ::roo_windows::internal::TransientSourceGeometry geometry;
  if (!::roo_windows::internal::CaptureTransientSourceGeometry(
          *impl_->interaction_owner, placement_source, geometry)) {
    return false;
  }
  return reanchorFromRect(geometry.bounds_in_window, placement, trigger);
}

bool Menu::reanchorFromRect(const Rect& bounds_in_window,
                            MenuPlacement placement,
                            const MenuTriggerPaintSource* trigger) {
  if (!impl_->registration.isActive() || impl_->interaction_owner == nullptr ||
      bounds_in_window.empty()) {
    return false;
  }
  ::roo_windows::internal::TransientSourceGeometry trigger_geometry;
  bool has_trigger =
      trigger != nullptr &&
      ::roo_windows::internal::CaptureTransientSourceGeometry(
          *impl_->interaction_owner, trigger->widget, trigger_geometry);
  if (!impl_->RelayoutRoot(bounds_in_window, placement)) return false;
  auto& host = ::roo_windows::internal::GetTransientSurfaceHost(
      *impl_->interaction_owner);
  host.hidePresentationPin(impl_->registration);
  if (has_trigger) {
    std::unique_ptr<PresentationPin> pin(
        new (std::nothrow) FrozenMenuTriggerPin(
            trigger_geometry, trigger->corner_radius, trigger->overlay_argb,
            trigger->overlay_opacity));
    host.showPresentationPin(impl_->registration, std::move(pin));
  }
  return true;
}

void Menu::dismissChain() {
  if (impl_->registration.isActive()) {
    impl_->registration.finish(PresentationFinishReason::kCancel);
  }
}

void Menu::prepareForDerivedDestruction() {
  if (!impl_) return;
  if (impl_->registration.isActive()) {
    impl_->registration.DisablePresentationInput();
    impl_->registration.CancelPresentation();
  }
  impl_->focus_scope.clearRememberedFocus();
  impl_->root_panel.clearGroups();
}

}  // namespace roo_windows::material3
