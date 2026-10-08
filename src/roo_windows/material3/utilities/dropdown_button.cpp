#include "roo_windows/material3/utilities/dropdown_button.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>

#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_icons/filled/18/navigation.h"
#include "roo_icons/filled/24/navigation.h"
#include "roo_icons/filled/36/navigation.h"
#include "roo_icons/filled/48/navigation.h"
#include "roo_scheduler.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/button/internal/button_appearance.h"
#include "roo_windows/material3/button/internal/button_geometry.h"
#include "roo_windows/material3/internal/density.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3 {
namespace {

const MonoIcon& Chevron(ButtonSize size) {
  static const MonoIcon extra_small = ic_filled_18_navigation_expand_more();
  static const MonoIcon standard = ic_filled_24_navigation_expand_more();
  static const MonoIcon large = ic_filled_36_navigation_expand_more();
  static const MonoIcon extra_large = ic_filled_48_navigation_expand_more();
  switch (size) {
    case ButtonSize::kExtraSmall:
      return extra_small;
    case ButtonSize::kSmall:
    case ButtonSize::kMedium:
      return standard;
    case ButtonSize::kLarge:
      return large;
    case ButtonSize::kExtraLarge:
      return extra_large;
  }
  return standard;
}

// Use one text block for both advance and ink; preserve negative bearings.
std::pair<int, int> TextWidthAndOrigin(roo::string_view text) {
  if (text.empty()) return {0, 0};
  const TextStyle& style = text_style_label_large();
  auto metrics =
      style.font().getHorizontalStringMetrics(text, style.fontOptions());
  int left = std::min<int>(0, metrics.screen_extents().xMin());
  int right =
      std::max<int>(metrics.advance(), metrics.screen_extents().xMax() + 1);
  return {std::max(0, right - left), -left};
}

int16_t ClampedDimension(int value) {
  return static_cast<int16_t>(
      std::max(0, std::min<int>(value, std::numeric_limits<int16_t>::max())));
}

}  // namespace

/// One active menu and its deferred completion, borrowed by its trigger.
class DropdownButton::Session final : public Menu,
                                      private roo_scheduler::Executable {
 public:
  class Item final : public StandardMenuItem {
   public:
    Item(Session& session, size_t index, roo::string_view label, bool selected)
        : StandardMenuItem(StandardMenuItemInit{
              label,
              {},
              nullptr,
              StandardMenuItemFlags::kSelectable |
                  (selected ? StandardMenuItemFlags::kSelected
                            : StandardMenuItemFlags::kDefault)}),
          session_(session),
          index_(index) {}

    void onInvoked() override { session_.pending_index_ = index_; }

   private:
    Session& session_;
    size_t index_;
  };

  explicit Session(DropdownButton& owner)
      : Menu(owner.context()), owner_(owner) {}

  ~Session() override {
    suppress_ = true;
    if (pending_execution_ >= 0 && owner_.tryContext() != nullptr) {
      owner_.context().scheduler().cancel(pending_execution_);
    }
    prepareForDerivedDestruction();
  }

  void populate(const char* const* items, size_t count, size_t selected) {
    auto group = std::make_unique<MenuGroup>(owner_.context());
    for (size_t i = 0; i < count; ++i) {
      auto row = std::make_unique<MenuRow<Item>>(owner_.context(), *this, i,
                                                 roo::string_view(items[i]),
                                                 i == selected);
      if (i == selected) selected_row_ = row.get();
      group->add(std::move(row));
    }
    MenuPolicy policy;
    policy.selection_mode = SelectionMode::kSingle;
    policy.density = owner_.density_;
    setPolicy(policy);
    addGroup(std::move(group));
  }

  MenuEntry* selectedRow() const { return selected_row_; }
  size_t pendingIndex() const { return pending_index_; }
  PresentationFinishReason finishReason() const { return finish_reason_; }
  bool isOpen() const { return open_; }
  void setOpen(bool open) { open_ = open; }
  void suppress() { suppress_ = true; }

 protected:
  void onFinished(PresentationFinishReason reason) override {
    open_ = false;
    finish_reason_ = reason;
    if (!suppress_ && owner_.tryContext() != nullptr) {
      pending_execution_ = owner_.context().scheduler().scheduleOn(
          roo_time::Uptime::Now(), *this, roo_scheduler::PRIORITY_NORMAL);
    }
  }

 private:
  void execute(roo_scheduler::ExecutionID id) override {
    if (id != pending_execution_) return;
    pending_execution_ = -1;
    owner_.completeSession();  // This call destroys this Session.
  }

  DropdownButton& owner_;
  MenuEntry* selected_row_ = nullptr;
  size_t pending_index_ = kNoSelection;
  roo_scheduler::ExecutionID pending_execution_ = -1;
  PresentationFinishReason finish_reason_ = PresentationFinishReason::kCancel;
  bool open_ = false;
  bool suppress_ = false;
};

DropdownButton::DropdownButton(ApplicationContext& context,
                               const char* const* items, size_t count,
                               ButtonVariant variant)
    : SurfaceWidget(context),
      variant_(static_cast<uint8_t>(variant)),
      size_(static_cast<uint8_t>(ButtonSize::kSmall)),
      shape_(static_cast<uint8_t>(ButtonShape::kRound)) {
  assert(validItems(items, count));
  if (validItems(items, count)) {
    items_ = items;
    count_ = count;
    selected_index_ = count == 0 ? kNoSelection : 0;
    text_metrics_ = measureItems(items, count);
  }
}

DropdownButton::~DropdownButton() { dismissMenu(); }

bool DropdownButton::validItems(const char* const* items, size_t count) {
  if (count > UINT16_MAX || (count > 0 && items == nullptr)) return false;
  for (size_t i = 0; i < count; ++i) {
    if (items[i] == nullptr) return false;
  }
  return true;
}

Dimensions DropdownButton::measureItems(const char* const* items,
                                        size_t count) {
  int width = 0;
  int height = 0;
  for (size_t i = 0; i < count; ++i) {
    roo::string_view label(items[i]);
    if (label.empty()) continue;
    width = std::max(width, TextWidthAndOrigin(label).first);
    height = std::max<int>(height, text_style_label_large().lineHeight());
  }
  return {ClampedDimension(width), ClampedDimension(height)};
}

bool DropdownButton::setItems(const char* const* items, size_t count,
                              size_t selected_index) {
  if (!validItems(items, count) || (count > 0 && selected_index >= count) ||
      (count == 0 && selected_index != 0 && selected_index != kNoSelection)) {
    return false;
  }
  Dimensions metrics = measureItems(items, count);
  dismissMenu();
  items_ = items;
  count_ = count;
  selected_index_ = count == 0 ? kNoSelection : selected_index;
  text_metrics_ = metrics;
  requestLayout();
  invalidateInterior();
  return true;
}

roo::string_view DropdownButton::selectedText() const {
  return selected_index_ == kNoSelection
             ? roo::string_view()
             : roo::string_view(items_[selected_index_]);
}

bool DropdownButton::setSelectedIndex(size_t index) {
  if (index >= count_) return false;
  dismissMenu();
  if (selected_index_ != index) {
    selected_index_ = index;
    invalidateInterior();
  }
  return true;
}

void DropdownButton::setSize(ButtonSize size) {
  if (size == this->size()) return;
  dismissMenu();
  size_ = static_cast<uint8_t>(size);
  requestLayout();
  invalidateInterior();
}

void DropdownButton::setShape(ButtonShape shape) {
  if (shape == this->shape()) return;
  dismissMenu();
  shape_ = static_cast<uint8_t>(shape);
  invalidateInterior();
}

void DropdownButton::setVariant(ButtonVariant variant) {
  if (variant == this->variant()) return;
  dismissMenu();
  uint8_t old_elevation = getElevation();
  variant_ = static_cast<uint8_t>(variant);
  uint8_t new_elevation = getElevation();
  if (old_elevation != new_elevation && isVisible()) {
    elevationChanged(std::max(old_elevation, new_elevation));
  }
  invalidateInterior();
}

void DropdownButton::setDensityOverride(DensityOverride density) {
  if (density == density_) return;
  dismissMenu();
  density_ = density;
  requestLayout();
  invalidateInterior();
}

int8_t DropdownButton::resolvedDensityLevel() const {
  return internal::ResolveDensityLevel(
      density_.resolve(theme().material3Theme().density));
}

Dimensions DropdownButton::iconSlot() const {
  const internal::ButtonGeometryTokens& tokens =
      internal::ButtonGeometryTokensFor(size());
  const MonoIcon& icon = Chevron(size());
  return {std::max<int16_t>(Scaled(tokens.icon_size_dp),
                            icon.anchorExtents().width()),
          std::max<int16_t>(Scaled(tokens.icon_size_dp),
                            icon.anchorExtents().height())};
}

Dimensions DropdownButton::getSuggestedMinimumDimensions() const {
  Dimensions icon = iconSlot();
  int gap = text_metrics_.width() > 0
                ? Scaled(internal::ButtonGeometryTokensFor(size()).icon_gap_dp)
                : 0;
  return {ClampedDimension(int(text_metrics_.width()) + gap + icon.width()),
          std::max(text_metrics_.height(), icon.height())};
}

Padding DropdownButton::getPadding() const {
  return internal::ResolveButtonPadding(
      size(), SmallButtonPadding::kReduced,
      getSuggestedMinimumDimensions().height(), resolvedDensityLevel());
}

Dimensions DropdownButton::onMeasure(WidthSpec width, HeightSpec height) {
  Dimensions natural = getNaturalDimensions();
  return {width.resolveSize(natural.width()),
          height.resolveSize(natural.height())};
}

void DropdownButton::requestLayoutDescending() {
  dismissMenu();
  text_metrics_ = measureItems(items_, count_);
  SurfaceWidget::requestLayoutDescending();
}

ColorToken DropdownButton::containerRole() const {
  return internal::ResolveButtonContainerRole(theme(), variant());
}

Color DropdownButton::background() const {
  return internal::ResolveButtonAppearance(theme(), variant(), isEnabled())
      .container;
}

Color DropdownButton::getOutlineColor() const {
  return internal::ResolveButtonAppearance(theme(), variant(), isEnabled())
      .outline;
}

BorderStyle DropdownButton::getBorderStyle() const {
  return internal::ResolveButtonBorderStyle(size(), shape(), variant(), true,
                                            isPressed(), getClickAnimation(),
                                            {width(), height()});
}

uint8_t DropdownButton::getElevation() const {
  return internal::ResolveButtonElevation(variant(), isEnabled(), isPressed());
}

void DropdownButton::paint(PaintContext& ctx) const {
  Rect area = bounds();
  if (area.empty()) return;
  Padding padding = getPadding();
  int left = std::min<int>(area.width(), padding.left());
  int right = std::max(left, area.width() - padding.right());
  int top = std::min<int>(area.height(), padding.top());
  int bottom = std::max(top, area.height() - padding.bottom());
  Rect content(left, top, right - 1, bottom - 1);
  if (content.empty()) {
    ctx.clearRect(area);
    return;
  }
  // Every part of the surface is settled once. The text/icon tiles own their
  // slots; these disjoint strips cover padding and the inter-slot gap.
  if (content.yMin() > area.yMin()) {
    ctx.clearRect(
        Rect(area.xMin(), area.yMin(), area.xMax(), content.yMin() - 1));
  }
  if (content.yMax() < area.yMax()) {
    ctx.clearRect(
        Rect(area.xMin(), content.yMax() + 1, area.xMax(), area.yMax()));
  }
  if (content.xMin() > area.xMin()) {
    ctx.clearRect(
        Rect(area.xMin(), content.yMin(), content.xMin() - 1, content.yMax()));
  }
  if (content.xMax() < area.xMax()) {
    ctx.clearRect(
        Rect(content.xMax() + 1, content.yMin(), area.xMax(), content.yMax()));
  }
  Dimensions icon = iconSlot();
  int icon_left =
      std::max<int>(content.xMin(), content.xMax() - icon.width() + 1);
  Rect icon_rect(icon_left, content.yMin(), content.xMax(), content.yMax());
  int gap = text_metrics_.width() > 0
                ? Scaled(internal::ButtonGeometryTokensFor(size()).icon_gap_dp)
                : 0;
  int text_right = std::max<int>(content.xMin() - 1, icon_left - gap - 1);
  Rect text_rect(content.xMin(), content.yMin(), text_right, content.yMax());
  if (text_right + 1 < icon_left) {
    ctx.clearRect(
        Rect(text_right + 1, content.yMin(), icon_left - 1, content.yMax()));
  }
  Color color =
      internal::ResolveButtonAppearance(theme(), variant(), isEnabled())
          .content;
  if (!text_rect.empty() && !selectedText().empty()) {
    const TextStyle& style = text_style_label_large();
    roo_display::StringViewLabel text(selectedText(), style.font(), color,
                                      style.fontOptions());
    PaintContext part = ctx.clipped(text_rect);
    part.setBgcolor(ctx.bgcolor());
    int origin = TextWidthAndOrigin(selectedText()).second;
    roo_display::Tile tile(
        &text, text_rect.asBox(),
        roo_display::kLeft.shiftBy(origin) | roo_display::kMiddle,
        ctx.bgcolor());
    part.drawObject(tile);
    ctx.addExclusion(text_rect);
  } else if (!text_rect.empty()) {
    ctx.clearRect(text_rect);
  }
  if (!icon_rect.empty()) {
    MonoIcon icon = Chevron(size());
    icon.color_mode().setColor(color);
    PaintContext part = ctx.clipped(icon_rect);
    part.setBgcolor(ctx.bgcolor());
    roo_display::Tile tile(&icon, icon_rect.asBox(),
                           roo_display::kCenter | roo_display::kMiddle,
                           ctx.bgcolor());
    part.drawObject(tile);
    ctx.addExclusion(icon_rect);
  }
}

MenuShowResult DropdownButton::showMenu() {
  if (session_ != nullptr) return MenuShowResult::kAlreadyPresented;
  Task* owner = getTask();
  if (!isEnabled() || count_ == 0 || owner == nullptr) {
    return MenuShowResult::kInteractionOwnerUnavailable;
  }
  auto session = std::make_unique<Session>(*this);
  session->populate(items_, count_, selected_index_);
  MenuShowResult result =
      session->show(*owner, *this, MenuPlacement::kBelowStart);
  if (result != MenuShowResult::kShown) return result;
  session->setOpen(true);
  MenuEntry* selected_row = session->selectedRow();
  session_ = std::move(session);
  context().presentations().observe(*this);
  if (selected_row != nullptr) selected_row->requestFocus();
  return MenuShowResult::kShown;
}

void DropdownButton::dismissMenu() {
  if (session_ == nullptr) return;
  context().presentations().unobserve(*this);
  session_->suppress();
  session_->dismissChain();
  session_.reset();
}

bool DropdownButton::isMenuOpen() const {
  return session_ != nullptr && session_->isOpen();
}

void DropdownButton::completeSession() {
  if (session_ == nullptr) return;
  size_t index = session_->pendingIndex();
  PresentationFinishReason reason = session_->finishReason();
  context().presentations().unobserve(*this);
  session_.reset();
  if (reason != PresentationFinishReason::kAction || index >= count_ ||
      presentationState() != PresentationState::kPresented || !isEnabled()) {
    return;
  }
  if (selected_index_ == index) return;
  selected_index_ = index;
  invalidateInterior();
  triggerInteractiveChange();
}

void DropdownButton::onClicked() { showMenu(); }

bool DropdownButton::onKeyEvent(const KeyEvent& event) {
  if (event.code != KeyCode::kDown || event.phase != KeyPhase::kDown) {
    return false;
  }
  showMenu();
  return true;
}

void DropdownButton::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & kWidgetEnabled) != 0 && !isEnabled()) dismissMenu();
  if ((state_diff & kWidgetPressed) != 0) invalidateInterior();
  if ((state_diff & (kWidgetEnabled | kWidgetPressed)) != 0) {
    bool old_enabled =
        (state_diff & kWidgetEnabled) != 0 ? !isEnabled() : isEnabled();
    bool old_pressed =
        (state_diff & kWidgetPressed) != 0 ? !isPressed() : isPressed();
    uint8_t old_elevation =
        internal::ResolveButtonElevation(variant(), old_enabled, old_pressed);
    uint8_t new_elevation = getElevation();
    if (old_elevation != new_elevation && isVisible()) {
      elevationChanged(std::max(old_elevation, new_elevation));
    }
  }
  SurfaceWidget::notifyStateChanged(state_diff);
}

void DropdownButton::onPresentationChanged(const PresentationChange& change) {
  if (session_ != nullptr && (change.state != PresentationState::kPresented ||
                              change.detached_since_delivery)) {
    dismissMenu();
  }
}

}  // namespace roo_windows::material3
