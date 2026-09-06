#include "roo_windows/material3/menu/menu.h"

#include <algorithm>
#include <cstdio>
#include <new>

#include "roo_display/ui/text_label.h"
#include "roo_icons/filled/24/navigation.h"
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

struct MenuAdornmentGeometry {
  Rect submenu;
  Rect badge_anchor;
  Rect icon;
  Rect checkmark;
  Rect shortcut;
};

Rect EmptyRect() { return Rect(0, 0, -1, -1); }

Rect TakeTrailingSlot(int16_t& right, int16_t width, int16_t height,
                      int16_t row_height, int16_t gap) {
  int16_t top = std::max<int16_t>(0, (row_height - height) / 2);
  Rect result(right - width + 1, top, right, top + height - 1);
  right = result.xMin() - gap - 1;
  return result;
}

MenuAdornmentGeometry ResolveMenuAdornmentGeometry(
    int16_t row_width, int16_t row_height, const internal::MenuTokens& tokens,
    bool has_submenu, bool has_badge, bool has_icon, bool is_selectable,
    int16_t shortcut_width) {
  MenuAdornmentGeometry result{EmptyRect(), EmptyRect(), EmptyRect(),
                               EmptyRect(), EmptyRect()};
  const int16_t icon_size = Scaled(tokens.icon_size_dp);
  const int16_t gap = Scaled(tokens.trailing_gap_dp);
  int16_t right = row_width - Scaled(tokens.horizontal_padding_dp) - 1;
  if (has_submenu) {
    result.submenu =
        TakeTrailingSlot(right, icon_size, icon_size, row_height, gap);
  }
  if (has_badge) {
    result.badge_anchor =
        TakeTrailingSlot(right, Scaled(24), icon_size, row_height, gap);
  }
  if (has_icon) {
    result.icon =
        TakeTrailingSlot(right, icon_size, icon_size, row_height, gap);
  }
  if (is_selectable) {
    result.checkmark =
        TakeTrailingSlot(right, icon_size, icon_size, row_height, gap);
  }
  if (shortcut_width > 0) {
    result.shortcut =
        TakeTrailingSlot(right, shortcut_width, icon_size, row_height, gap);
  }
  return result;
}

void PaintTintedIcon(PaintContext& ctx, const MonoIcon& source,
                     const Rect& bounds, Color color) {
  if (bounds.empty()) return;
  MonoIcon icon = source;
  icon.color_mode().setColor(color);
  PaintContext icon_context = ctx.clipped(bounds);
  icon_context.drawTiled(icon, bounds,
                         roo_display::kCenter | roo_display::kMiddle);
  ctx.addExclusion(bounds);
}

ListItemPosition PositionInGroup(size_t index, size_t count) {
  if (count <= 1) return ListItemPosition::kSingle;
  if (index == 0) return ListItemPosition::kFirst;
  if (index + 1 == count) return ListItemPosition::kLast;
  return ListItemPosition::kMiddle;
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
        corner_radius_(std::min<uint16_t>(corner_radius, UINT8_MAX)),
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
};

#if defined(ROO_WINDOWS_MENU_ABI_PROBE)
unsigned char
    StandardMenuItem::abi_probe_trailing_payload_[sizeof(TrailingPayload)] = {};
unsigned char MenuEntry::abi_probe_adornment_state_[sizeof(AdornmentState)] =
    {};
#endif

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
  return menu_ != nullptr && bound != nullptr && visualContext().enabled;
}

ColorToken MenuEntry::containerRole() const {
  if (!vibrant_) return ListEntry::containerRole();
  return visualContext().selected
             ? internal::kExpressiveVibrantMenuTokens.selected_container
             : internal::kExpressiveVibrantMenuTokens.panel_container;
}

Color MenuEntry::background() const {
  if (!vibrant_) return ListEntry::background();
  return theme().material3Theme().color.resolve(containerRole());
}

Color MenuEntry::headlineColor() const {
  if (!vibrant_) return ListEntry::headlineColor();
  const internal::MenuTokens& tokens = internal::kExpressiveVibrantMenuTokens;
  return theme().material3Theme().color.resolve(visualContext().selected
                                                    ? tokens.selected_content
                                                    : tokens.panel_content);
}

Color MenuEntry::supportingColor() const {
  return vibrant_ ? headlineColor() : ListEntry::supportingColor();
}

bool MenuEntry::onKeyEvent(const KeyEvent& event) {
  return menu_ != nullptr && menu_->handleEntryKey(*this, event);
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
                           uint16_t generation, bool vibrant) {
  CHECK(menu_ == nullptr || menu_ == &owner);
  menu_ = &owner;
  level_ = level;
  row_ = row;
  level_generation_ = generation;
  submenu_allowed_ = level < 3;
  vibrant_ = vibrant;
  suppress_next_click_dispatch_ = false;
  ListEntryVisualContext visual = visualContext();
  const MenuItem* bound = menuItem();
  visual.enabled = bound != nullptr && bound->isEnabled() &&
                   (submenu_allowed_ || !bound->hasSubmenu());
  visual.selected = bound != nullptr && bound->isSelected();
  setVisualContext(visual);
  syncAdornments();
}

void MenuEntry::unbindFromMenu() {
  bool was_vibrant = vibrant_;
  menu_ = nullptr;
  level_ = 0;
  row_ = 0;
  level_generation_ = 0;
  submenu_allowed_ = true;
  vibrant_ = false;
  suppress_next_click_dispatch_ = false;
  if (was_vibrant) refreshFromItem();
}

void MenuEntry::syncAdornments() {
  const MenuItem* bound = menuItem();
  MenuTrailingAffordances content = bound == nullptr
                                        ? MenuTrailingAffordances{}
                                        : bound->trailingAffordances();
  if (content.shortcut.empty() && content.icon == nullptr &&
      content.badge.mode == BadgeMode::kHidden &&
      !(bound != nullptr && bound->isSelectable()) &&
      !(bound != nullptr && bound->hasSubmenu() && submenu_allowed_)) {
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
  if (bound != nullptr && bound->hasSubmenu() && submenu_allowed_) {
    width += Scaled(tokens.icon_size_dp);
    ++count;
  }
  if (count > 1) width += (count - 1) * Scaled(tokens.trailing_gap_dp);
  return width == 0 ? 0 : width + Scaled(tokens.trailing_gap_dp);
}

Dimensions MenuEntry::onMeasure(WidthSpec width, HeightSpec height) {
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
  // Minimum menu width belongs to MenuPanel so its content padding remains
  // inside that minimum. A row simply honors the content-width constraint it
  // receives from the already-inset viewport.
  return Dimensions(
      resolved_width,
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
  const MenuItem* bound = menuItem();
  MenuAdornmentGeometry geometry = ResolveMenuAdornmentGeometry(
      rect.width(), rect.height(), tokens,
      bound != nullptr && bound->hasSubmenu() && submenu_allowed_,
      adornments_->badge.visible(), adornments_->content.icon != nullptr,
      bound != nullptr && bound->isSelectable(),
      ShortcutWidth(adornments_->content.shortcut));
  adornments_->badge.layoutForIcon(geometry.badge_anchor);
}

void MenuEntry::paint(PaintContext& ctx) const {
  if (!adornments_) {
    ListEntry::paint(ctx);
    return;
  }

  const Theme& current_theme = theme();
  Color color = headlineColor();
  const MenuItem* bound = menuItem();
  MenuAdornmentGeometry geometry = ResolveMenuAdornmentGeometry(
      width(), height(), TokensFor(visualContext()),
      bound != nullptr && bound->hasSubmenu() && submenu_allowed_,
      adornments_->badge.visible(), adornments_->content.icon != nullptr,
      bound != nullptr && bound->isSelectable(),
      ShortcutWidth(adornments_->content.shortcut));

  if (bound != nullptr && bound->hasSubmenu() && submenu_allowed_) {
    PaintTintedIcon(ctx, ic_filled_24_navigation_chevron_right(),
                    geometry.submenu, color);
  }
  if (adornments_->content.icon != nullptr) {
    PaintContext icon_context = ctx.clipped(geometry.icon);
    icon_context.drawTiled(*adornments_->content.icon, geometry.icon,
                           roo_display::kCenter | roo_display::kMiddle);
    ctx.addExclusion(geometry.icon);
  }

  if (bound != nullptr && bound->isSelectable() && bound->isSelected()) {
    PaintTintedIcon(ctx, ic_filled_24_navigation_check(), geometry.checkmark,
                    color);
  }

  if (!adornments_->content.shortcut.empty()) {
    const TextStyle& style = text_style_label_large();
    roo_display::StringViewLabel label(adornments_->content.shortcut,
                                       style.font(), color,
                                       style.fontOptions());
    ctx.drawTiled(label, geometry.shortcut,
                  roo_display::kRight | roo_display::kMiddle);
    ctx.addExclusion(geometry.shortcut);
  }

  if (adornments_->badge.visible()) {
    adornments_->badge.paint(ctx, current_theme);
  }

  // The row surface is lower-z than every adornment. Exclusions above keep
  // this final pass disjoint from already-settled foreground pixels.
  ListEntry::paint(ctx);
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
      owner_.closeLevelsFrom(1, false);
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
      if (owner_.closeDeepestLevel()) return BackResult::kHandled;
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
        context(context),
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
    CHECK(level < kMaxLevels);
    CHECK_EQ(level, population_level);
    CHECK_EQ(generation, level_generation[level]);
    CHECK(level_panels[level] != nullptr);
    level_panels[level]->addGroup(group);
  }

  void AddLevelGroup(uint8_t level, uint16_t generation,
                     std::unique_ptr<MenuGroup> group) {
    CHECK(population_active);
    CHECK(level < kMaxLevels);
    CHECK_EQ(level, population_level);
    CHECK_EQ(generation, level_generation[level]);
    CHECK(level_panels[level] != nullptr);
    level_panels[level]->addGroup(std::move(group));
  }

  MenuPolicy policy;
  Menu& presenter;
  ApplicationContext& context;
  internal::MenuPanel root_panel;
  internal::MenuOverlay overlay;
  std::unique_ptr<internal::MenuPanel> child_panels[kMaxLevels];
  internal::MenuPanel* level_panels[kMaxLevels] = {&root_panel, nullptr,
                                                   nullptr, nullptr};
  uint16_t level_generation[kMaxLevels] = {1, 1, 1, 1};
  uint16_t opener_row[kMaxLevels] = {0, 0, 0, 0};
  uint8_t parent_level[kMaxLevels] = {0, 0, 0, 0};
  bool cascading[kMaxLevels] = {false, false, false, false};
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

#if defined(ROO_WINDOWS_MENU_ABI_PROBE)
unsigned char Menu::abi_probe_implementation_[sizeof(Impl)] = {};
#endif

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

void Menu::bindRootEntries() { bindLevelEntries(0); }

void Menu::bindLevelEntries(uint8_t level) {
  CHECK(level < Impl::kMaxLevels);
  internal::MenuPanel* panel = impl_->level_panels[level];
  CHECK(panel != nullptr);
  uint16_t row_index = 0;
  for (int group_index = 0; group_index < panel->groupCount(); ++group_index) {
    MenuGroup& group = panel->groupAt(group_index);
    for (size_t group_row = 0; group_row < group.entries_.size(); ++group_row) {
      MenuEntry* entry = group.entries_[group_row];
      entry->bindToMenu(
          *this, level, row_index++, impl_->level_generation[level],
          impl_->policy.variant == ListVariant::kExpressive &&
              impl_->policy.color_style == MenuColorStyle::kVibrant);
      ListEntryVisualContext visual = entry->visualContext();
      visual.variant = impl_->policy.variant;
      visual.style = impl_->policy.variant == ListVariant::kExpressive
                         ? ListStyle::kSegmented
                         : ListStyle::kStandard;
      visual.position = PositionInGroup(group_row, group.entries_.size());
      if (impl_->policy.selection_mode == SelectionMode::kNone &&
          entry->menuItem() != nullptr && entry->menuItem()->isSelectable()) {
        visual.selected = false;
        LOG(WARNING) << "Selectable menu item bound in SelectionMode::kNone";
      }
      entry->setVisualContext(visual);
      entry->refreshFromItem();
    }
  }
}

void Menu::unbindLevelEntries(uint8_t level) {
  if (level >= Impl::kMaxLevels || impl_->level_panels[level] == nullptr)
    return;
  internal::MenuPanel& panel = *impl_->level_panels[level];
  for (int group_index = 0; group_index < panel.groupCount(); ++group_index) {
    MenuGroup& group = panel.groupAt(group_index);
    for (MenuEntry* entry : group.entries_) entry->unbindFromMenu();
  }
}

void Menu::unbindAllEntries() {
  for (int level = Impl::kMaxLevels - 1; level >= 0; --level) {
    unbindLevelEntries(level);
  }
}

void Menu::closeLevelsFrom(uint8_t first_level, bool restore_focus) {
  if (first_level == 0 || first_level >= Impl::kMaxLevels) return;
  uint8_t restore_parent = impl_->parent_level[first_level];
  uint16_t restore_row = impl_->opener_row[first_level];
  bool closed_any = false;
  for (int level = Impl::kMaxLevels - 1; level >= first_level; --level) {
    internal::MenuPanel* panel = impl_->level_panels[level];
    if (panel == nullptr) continue;
    closed_any = true;
    unbindLevelEntries(level);
    impl_->overlay.removePanel(*panel);
    panel->clearGroups();
    uint8_t parent = impl_->parent_level[level];
    if (!impl_->cascading[level] && impl_->level_panels[parent] != nullptr) {
      impl_->level_panels[parent]->setVisibility(Visibility::kVisible);
    }
    impl_->level_panels[level] = nullptr;
    impl_->child_panels[level].reset();
    ++impl_->level_generation[level];
  }
  if (!closed_any || !restore_focus || impl_->interaction_owner == nullptr ||
      impl_->level_panels[restore_parent] == nullptr) {
    return;
  }
  uint16_t index = 0;
  internal::MenuPanel& parent = *impl_->level_panels[restore_parent];
  for (int group_index = 0; group_index < parent.groupCount(); ++group_index) {
    MenuGroup& group = parent.groupAt(group_index);
    for (MenuEntry* entry : group.entries_) {
      if (index++ == restore_row) {
        impl_->interaction_owner->focus().requestFocus(*entry);
        return;
      }
    }
  }
}

bool Menu::closeDeepestLevel() {
  for (int level = Impl::kMaxLevels - 1; level > 0; --level) {
    if (impl_->level_panels[level] != nullptr) {
      closeLevelsFrom(level, true);
      return true;
    }
  }
  return false;
}

void Menu::openSubmenu(MenuEntry& entry, MenuItem& item, uint8_t level,
                       uint16_t row, uint16_t generation) {
  if (level + 1 >= Impl::kMaxLevels ||
      generation != impl_->level_generation[level] ||
      impl_->interaction_owner == nullptr) {
    return;
  }
  const uint8_t child_level = level + 1;
  closeLevelsFrom(child_level, false);
  std::unique_ptr<internal::MenuPanel> child(
      new (std::nothrow) internal::MenuPanel(impl_->context));
  if (!child) return;
  child->setPolicy(impl_->policy);
  ++impl_->level_generation[child_level];
  if (impl_->level_generation[child_level] == 0) {
    ++impl_->level_generation[child_level];
  }
  impl_->child_panels[child_level] = std::move(child);
  impl_->level_panels[child_level] = impl_->child_panels[child_level].get();
  impl_->parent_level[child_level] = level;
  impl_->opener_row[child_level] = row;
  impl_->population_level = child_level;
  impl_->population_active = true;
  MenuLevelBuilder builder(*this, child_level,
                           impl_->level_generation[child_level]);
  item.populateSubmenu(builder);
  impl_->population_active = false;
  if (impl_->level_panels[child_level]->groupCount() == 0) {
    impl_->level_panels[child_level] = nullptr;
    impl_->child_panels[child_level].reset();
    return;
  }

  MainWindow& window = impl_->interaction_owner->window().root();
  const internal::MenuTokens& tokens = impl_->RootTokens();
  int16_t margin = Scaled(tokens.viewport_margin_dp);
  Rect viewport(margin, margin, window.width() - margin - 1,
                window.height() - margin - 1);
  internal::MenuPanel& panel = *impl_->level_panels[child_level];
  bindLevelEntries(child_level);
  Dimensions desired = panel.measure(WidthSpec::AtMost(viewport.width()),
                                     HeightSpec::AtMost(viewport.height()));
  int32_t x = 0;
  int32_t y = 0;
  Widget* current = &entry;
  while (current != &impl_->overlay) {
    if (current == nullptr) {
      unbindLevelEntries(child_level);
      impl_->level_panels[child_level] = nullptr;
      impl_->child_panels[child_level].reset();
      return;
    }
    x += current->bounds().xMin();
    y += current->bounds().yMin();
    current = current->parent();
  }
  Rect opener(x, y, x + entry.width() - 1, y + entry.height() - 1);
  Rect parent = impl_->level_panels[level]->bounds();
  internal::SubmenuPlacementResult resolved = internal::ResolveSubmenuPlacement(
      viewport, opener, parent, desired, Scaled(tokens.min_width_dp),
      Scaled(tokens.submenu_gutter_dp), impl_->policy.layout_direction);
  panel.measure(WidthSpec::Exactly(resolved.bounds.width()),
                HeightSpec::Exactly(resolved.bounds.height()));
  panel.layout(resolved.bounds);
  impl_->cascading[child_level] = resolved.cascading;
  if (!resolved.cascading) {
    impl_->level_panels[level]->setVisibility(Visibility::kGone);
  }
  impl_->overlay.addPanel(panel, resolved.bounds);
  if (Widget* preferred = panel.preferredFocusChild(); preferred != nullptr) {
    impl_->interaction_owner->focus().requestFocus(*preferred);
  } else {
    closeLevelsFrom(child_level, true);
  }
}

bool Menu::handleEntryKey(MenuEntry& entry, const KeyEvent& event) {
  if (!impl_->registration.isActive() || entry.menu_ != this ||
      (event.phase != KeyPhase::kDown && event.phase != KeyPhase::kRepeat)) {
    return false;
  }
  uint8_t level = entry.level_;
  if (level >= Impl::kMaxLevels || impl_->level_panels[level] == nullptr ||
      entry.level_generation_ != impl_->level_generation[level]) {
    return false;
  }
  const bool backwards = event.code == KeyCode::kUp ||
                         (event.code == KeyCode::kTab &&
                          (event.modifiers & kKeyModifierShift) != 0);
  if (event.code == KeyCode::kUp || event.code == KeyCode::kDown ||
      event.code == KeyCode::kTab || event.code == KeyCode::kHome ||
      event.code == KeyCode::kEnd) {
    internal::MenuPanel& panel = *impl_->level_panels[level];
    int eligible_count = 0;
    int current_index = -1;
    for (int group_index = 0; group_index < panel.groupCount(); ++group_index) {
      MenuGroup& group = panel.groupAt(group_index);
      for (MenuEntry* candidate : group.entries_) {
        if (!candidate->isClickable()) continue;
        if (candidate == &entry) current_index = eligible_count;
        ++eligible_count;
      }
    }
    if (eligible_count == 0) return true;
    int target_index;
    if (event.code == KeyCode::kHome) {
      target_index = 0;
    } else if (event.code == KeyCode::kEnd) {
      target_index = eligible_count - 1;
    } else {
      target_index = current_index < 0 ? 0 : current_index;
      target_index += backwards ? -1 : 1;
      if (target_index < 0) target_index = eligible_count - 1;
      if (target_index >= eligible_count) target_index = 0;
    }
    int index = 0;
    for (int group_index = 0; group_index < panel.groupCount(); ++group_index) {
      MenuGroup& group = panel.groupAt(group_index);
      for (MenuEntry* candidate : group.entries_) {
        if (!candidate->isClickable()) continue;
        if (index++ == target_index) {
          impl_->interaction_owner->focus().requestFocus(*candidate);
          return true;
        }
      }
    }
    return true;
  }
  const KeyCode after =
      impl_->policy.layout_direction == LayoutDirection::kLeftToRight
          ? KeyCode::kRight
          : KeyCode::kLeft;
  const KeyCode before =
      impl_->policy.layout_direction == LayoutDirection::kLeftToRight
          ? KeyCode::kLeft
          : KeyCode::kRight;
  if (event.code == after && entry.menuItem() != nullptr &&
      entry.menuItem()->hasSubmenu()) {
    invokeEntry(entry, entry.level_, entry.row_, entry.level_generation_);
    return true;
  }
  if (event.code == before && level > 0) {
    closeLevelsFrom(level, true);
    return true;
  }
  return false;
}

void Menu::invokeEntry(MenuEntry& entry, uint8_t level, uint16_t row,
                       uint16_t generation) {
  if (!impl_->registration.isActive() || level >= Impl::kMaxLevels ||
      impl_->level_panels[level] == nullptr ||
      generation != impl_->level_generation[level] || entry.menu_ != this) {
    return;
  }

  internal::MenuPanel& active_panel = *impl_->level_panels[level];
  MenuEntry* resolved = nullptr;
  uint16_t current_row = 0;
  for (int group_index = 0; group_index < active_panel.groupCount();
       ++group_index) {
    MenuGroup& group = active_panel.groupAt(group_index);
    for (MenuEntry* candidate : group.entries_) {
      if (current_row++ == row) resolved = candidate;
    }
  }
  if (resolved != &entry) return;
  MenuItem* item = entry.menuItem();
  if (item == nullptr || !item->isEnabled() || !entry.isClickable()) return;
  if (item->hasSubmenu()) {
    openSubmenu(entry, *item, level, row, generation);
    return;
  }

  const bool selectable = item->isSelectable() &&
                          impl_->policy.selection_mode != SelectionMode::kNone;
  const bool was_selected = item->isSelected();
  const SelectionMode selection_mode = impl_->policy.selection_mode;
  const MenuLeafDismissal dismissal = item->leafDismissal();

  if (selectable && selection_mode == SelectionMode::kSingle) {
    for (int group_index = 0; group_index < active_panel.groupCount();
         ++group_index) {
      MenuGroup& group = active_panel.groupAt(group_index);
      for (MenuEntry* candidate : group.entries_) {
        MenuItem* candidate_item = candidate->menuItem();
        if (candidate_item != nullptr && candidate_item->isSelectable()) {
          candidate_item->setSelectedFromMenu(candidate == &entry);
        }
        candidate->refreshFromItem();
        ListEntryVisualContext visual = candidate->visualContext();
        visual.enabled = candidate_item != nullptr &&
                         candidate_item->isEnabled() &&
                         (level < 3 || !candidate_item->hasSubmenu());
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
      generation != impl_->level_generation[level] || entry.menu_ != this) {
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
