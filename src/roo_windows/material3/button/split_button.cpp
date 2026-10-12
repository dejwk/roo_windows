#include "roo_windows/material3/button/split_button.h"

#include <algorithm>
#include <cmath>

#include "roo_windows/core/click_animation.h"
#include "roo_windows/material3/button/internal/button_appearance.h"
#include "roo_windows/material3/button/internal/button_geometry.h"
#include "roo_windows/material3/button/internal/split_chevrons.h"
#include "roo_windows/material3/internal/density.h"

namespace roo_windows::material3 {
namespace {

struct SplitTokens {
  uint8_t height;
  uint8_t primary_start;
  uint8_t primary_end;
  uint8_t trailing_icon;
  uint8_t trailing_start;
  uint8_t trailing_end;
  uint8_t inner_rest;
  uint8_t inner_hover;
  uint8_t inner_pressed;
};

constexpr SplitTokens kTokens[] = {
    {32, 12, 10, 22, 13, 13, 4, 8, 8},     {40, 16, 12, 22, 13, 13, 4, 12, 12},
    {56, 24, 24, 26, 15, 15, 4, 12, 12},   {96, 48, 48, 38, 29, 29, 8, 20, 20},
    {136, 64, 64, 50, 43, 43, 12, 20, 20},
};

const SplitTokens& Tokens(ButtonSize size) {
  return kTokens[static_cast<uint8_t>(size)];
}

ButtonVariant ButtonAppearance(SplitButtonVariant variant) {
  switch (variant) {
    case SplitButtonVariant::kFilled:
      return ButtonVariant::kFilled;
    case SplitButtonVariant::kFilledTonal:
      return ButtonVariant::kFilledTonal;
    case SplitButtonVariant::kOutlined:
      return ButtonVariant::kOutlined;
    case SplitButtonVariant::kElevated:
      return ButtonVariant::kElevated;
  }
  return ButtonVariant::kFilled;
}

int TrailingWidth(ButtonSize size) {
  const SplitTokens& tokens = Tokens(size);
  return Scaled(tokens.trailing_start) + Scaled(tokens.trailing_icon) +
         Scaled(tokens.trailing_end);
}

uint8_t ClampedRadius(int radius, int width, int height) {
  int half = std::max(0, std::min(width, height) / 2);
  return static_cast<uint8_t>(std::min({radius, half, 255}));
}

uint8_t InterpolateRadius(uint8_t from, uint8_t to, float progress) {
  if (progress <= 0.0f) return from;
  if (progress >= 1.0f) return to;
  return static_cast<uint8_t>(std::lround(from + (to - from) * progress));
}

}  // namespace

SplitButton::SplitButton(ApplicationContext& context, Menu& menu,
                         roo::string_view label, SplitButtonVariant variant)
    : Container(context),
      primary_(context, label, ButtonAppearance(variant)),
      trailing_(context, {}, ButtonAppearance(variant)),
      menu_(&menu) {
  primary_.setParentClipMode(ParentClipMode::kUnclipped);
  trailing_.setParentClipMode(ParentClipMode::kUnclipped);
  attachChild(WidgetRef(primary_));
  attachChild(WidgetRef(trailing_));
}

SplitButton::~SplitButton() {
  dismissMenu();
  detachChild(&trailing_);
  detachChild(&primary_);
}

SplitButtonVariant SplitButton::variant() const {
  switch (primary_.variant()) {
    case ButtonVariant::kFilled:
      return SplitButtonVariant::kFilled;
    case ButtonVariant::kFilledTonal:
      return SplitButtonVariant::kFilledTonal;
    case ButtonVariant::kOutlined:
      return SplitButtonVariant::kOutlined;
    case ButtonVariant::kElevated:
      return SplitButtonVariant::kElevated;
    case ButtonVariant::kText:
      break;
  }
  return SplitButtonVariant::kFilled;
}

void SplitButton::setVariant(SplitButtonVariant variant) {
  if (variant == this->variant()) return;
  primary_.setVariant(ButtonAppearance(variant));
  trailing_.setVariant(ButtonAppearance(variant));
}

void SplitButton::setSize(ButtonSize size) {
  if (size == this->size()) return;
  dismissMenu();
  primary_.setSize(size);
  trailing_.setSize(size);
  requestLayout();
}

void SplitButton::setLabel(roo::string_view label) {
  if (label.data() == this->label().data() &&
      label.size() == this->label().size()) {
    return;
  }
  dismissMenu();
  primary_.setLabel(label);
  requestLayout();
}

void SplitButton::setIcon(const MonoIcon* icon) {
  if (icon == this->icon()) return;
  dismissMenu();
  primary_.setIcon(icon);
  requestLayout();
}

void SplitButton::setLayoutDirection(LayoutDirection direction) {
  if (direction == direction_) return;
  dismissMenu();
  direction_ = direction;
  requestLayout();
  invalidateInterior();
}

MenuShowResult SplitButton::showMenu() {
  if (isMenuOpen()) return MenuShowResult::kAlreadyPresented;
  Task* owner = getTask();
  if (!isEnabled() || (label().empty() && icon() == nullptr) ||
      !isPresented() || owner == nullptr) {
    return MenuShowResult::kInteractionOwnerUnavailable;
  }
  MenuShowResult result =
      menu_->show(*owner, trailing_, MenuPlacement::kBelowStart, this);
  if (result != MenuShowResult::kShown) return result;
  trailing_.setActivated(true);
  trailing_.invalidateInterior();
  if (!context().presentations().observe(*this)) {
    menu_->cancelPresentation();
    return MenuShowResult::kInteractionOwnerUnavailable;
  }
  return MenuShowResult::kShown;
}

void SplitButton::dismissMenu() {
  if (!isMenuOpen()) return;
  if (tryContext() != nullptr) {
    menu_->cancelPresentation();
  } else {
    trailing_.setActivated(false);
  }
}

void SplitButton::onMenuClosed(Menu& menu, PresentationFinishReason reason) {
  (void)menu;
  (void)reason;
  if (tryContext() != nullptr) {
    context().presentations().unobserve(*this);
  }
  trailing_.setActivated(false);
  trailing_.invalidateInterior();
}

void SplitButton::onPrimaryInvoked() { triggerInteractiveChange(); }

void SplitButton::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & kWidgetEnabled) != 0) {
    if (!isEnabled()) dismissMenu();
    primary_.setEnabled(isEnabled());
    trailing_.setEnabled(isEnabled());
  }
  Container::notifyStateChanged(state_diff);
}

void SplitButton::onPresentationChanged(const PresentationChange& change) {
  if (isMenuOpen() && (change.state != PresentationState::kPresented ||
                       change.detached_since_delivery)) {
    dismissMenu();
  }
}

void SplitButton::requestLayoutDescending() {
  dismissMenu();
  Container::requestLayoutDescending();
}

Dimensions SplitButton::onMeasure(WidthSpec width, HeightSpec height) {
  const SplitTokens& tokens = Tokens(size());
  int8_t density =
      internal::ResolveDensityLevel(theme().material3Theme().density);
  internal::ButtonContentMetrics content =
      internal::ResolveButtonContentMetrics(label(), icon(), size(), density);
  int chevron_height = Scaled(tokens.trailing_icon);
  int content_height = std::max<int>(content.content_height, chevron_height);
  int natural_height = std::max(
      Scaled(std::max(24, static_cast<int>(tokens.height) + 4 * density)),
      content_height + 2 * Scaled(4));
  int natural_width = Scaled(tokens.primary_start) + content.content_width +
                      Scaled(tokens.primary_end) + Scaled(2) +
                      TrailingWidth(size());
  Dimensions measured(width.resolveSize(natural_width),
                      height.resolveSize(natural_height));
  primary_.measure(WidthSpec::Exactly(measured.width()),
                   HeightSpec::Exactly(measured.height()));
  trailing_.measure(WidthSpec::Exactly(measured.width()),
                    HeightSpec::Exactly(measured.height()));
  return measured;
}

void SplitButton::onLayout(bool changed, const Rect& rect) {
  (void)rect;
  if (changed && isMenuOpen()) dismissMenu();
  int w = width();
  int h = height();
  int trailing_width = std::min<int>(TrailingWidth(size()), w);
  int gap = Scaled(2);
  int primary_width = std::max(0, w - trailing_width - gap);
  Rect primary_rect;
  Rect trailing_rect;
  if (direction_ == LayoutDirection::kLeftToRight) {
    primary_rect = Rect(0, 0, primary_width - 1, h - 1);
    trailing_rect = Rect(w - trailing_width, 0, w - 1, h - 1);
  } else {
    trailing_rect = Rect(0, 0, trailing_width - 1, h - 1);
    primary_rect = Rect(w - primary_width, 0, w - 1, h - 1);
  }
  if (isMenuOpen() && (primary_rect != primary_.parent_bounds() ||
                       trailing_rect != trailing_.parent_bounds())) {
    dismissMenu();
  }
  primary_.measure(WidthSpec::Exactly(primary_width), HeightSpec::Exactly(h));
  trailing_.measure(WidthSpec::Exactly(trailing_width), HeightSpec::Exactly(h));
  primary_.layout(primary_rect);
  trailing_.layout(trailing_rect);
}

const Widget& SplitButton::getChild(int index) const {
  return index == 0 ? static_cast<const Widget&>(primary_)
                    : static_cast<const Widget&>(trailing_);
}

Widget& SplitButton::getChild(int index) {
  return index == 0 ? static_cast<Widget&>(primary_)
                    : static_cast<Widget&>(trailing_);
}

bool SplitButton::SegmentButton::isTrailing() const {
  return owner() != nullptr && this == &owner()->trailing_;
}

Dimensions SplitButton::SegmentButton::getSuggestedMinimumDimensions() const {
  if (parent() == nullptr || !isTrailing()) return Dimensions(0, 0);
  int side = Scaled(Tokens(size()).trailing_icon);
  return Dimensions(side, side);
}

bool SplitButton::SegmentButton::isClickable() const {
  const SplitButton* split = owner();
  return split != nullptr && split->isEnabled() &&
         (!split->label().empty() || split->icon() != nullptr);
}

void SplitButton::SegmentButton::onClicked() {
  SplitButton* split = owner();
  if (split == nullptr || !isClickable() || !split->isPresented()) return;
  if (isTrailing()) {
    split->showMenu();
  } else {
    split->onPrimaryInvoked();
  }
}

void SplitButton::SegmentButton::paint(PaintContext& ctx) const {
  if (!isTrailing()) {
    Button::paint(ctx);
    return;
  }
  // Synchronous painting borrows this thread's scratch only for this draw.
  // Reusing its two inputs avoids allocation on subsequent animation frames.
  static thread_local internal::SplitChevron chevron;
  const ClickAnimation* animation = getClickAnimation();
  // Keep the terminal orientation through a held press and menu admission.
  // The shared controller supplies every intermediate frame and invalidation.
  float fraction = isActivated() || isPressed() ? 1.0f : 0.0f;
  if (!isActivated() && animation != nullptr) fraction = animation->progress();
  chevron.setGeometry(
      bounds().asBox(), Scaled(Tokens(size()).trailing_icon), fraction,
      internal::ResolveButtonAppearance(theme(), variant(), isEnabled())
          .content);
  ctx.canvas().drawTiled(chevron.image(), bounds(),
                         roo_display::kCenter | roo_display::kMiddle);
}

BorderStyle SplitButton::SegmentButton::resolveShape(bool pressed, bool hover,
                                                     bool open,
                                                     float progress) const {
  if (width() <= 0 || height() <= 0) return BorderStyle();
  const SplitTokens& tokens = Tokens(size());
  uint8_t outer = ClampedRadius(height() / 2, width(), height());
  int resting = Scaled(tokens.inner_rest);
  int hovered = Scaled(tokens.inner_hover);
  int pressed_radius = isTrailing() ? 100000 : Scaled(tokens.inner_pressed);
  uint8_t from = ClampedRadius(open && isTrailing() ? height() / 2
                               : hover              ? hovered
                                                    : resting,
                               width(), height());
  uint8_t to = ClampedRadius(pressed_radius, width(), height());
  uint8_t inner = pressed ? to : from;
  if (progress >= 0.0f) {
    inner = InterpolateRadius(from, to, std::min(1.0f, 3.0f * progress));
  }
  if (open && isTrailing() && !pressed && progress < 0.0f) inner = outer;
  SmallNumber outline = variant() == ButtonVariant::kOutlined
                            ? Scaled(SmallNumber(1))
                            : SmallNumber(0);
  bool left_outer =
      isTrailing() == (owner()->direction_ == LayoutDirection::kRightToLeft);
  return left_outer ? BorderStyle(outer, inner, inner, outer, outline)
                    : BorderStyle(inner, outer, outer, inner, outline);
}

BorderStyle SplitButton::SegmentButton::resolveBorderStyle(
    bool pressed, float progress) const {
  return resolveShape(pressed, isHover(), isActivated(), progress);
}

Rect SplitButton::SegmentButton::resolveContentBounds(
    const Rect& bounds) const {
  if (bounds.empty() || owner() == nullptr) return bounds;
  const SplitTokens& tokens = Tokens(size());
  int logical_start =
      isTrailing() ? tokens.trailing_start : tokens.primary_start;
  int logical_end = isTrailing() ? tokens.trailing_end : tokens.primary_end;
  int start = Scaled(logical_start);
  int end = Scaled(logical_end);
  if (owner()->direction_ == LayoutDirection::kRightToLeft) {
    std::swap(start, end);
  }
  int left = std::min<int>(start, bounds.width());
  int right = std::min<int>(end, std::max(0, bounds.width() - left));
  Rect content(bounds.xMin() + left, bounds.yMin(), bounds.xMax() - right,
               bounds.yMax());
  return content;
}

void SplitButton::SegmentButton::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & (kWidgetHover | kWidgetActivated)) != 0 &&
      owner() != nullptr) {
    const ClickAnimation* animation = getClickAnimation();
    float progress = animation == nullptr ? -1.0f : animation->progress();
    BorderStyle prior = resolveShape(
        isPressed(), (state_diff & kWidgetHover) != 0 ? !isHover() : isHover(),
        (state_diff & kWidgetActivated) != 0 ? !isActivated() : isActivated(),
        progress);
    borderStyleChanged(prior);
  }
  Button::notifyStateChanged(state_diff);
}

Rect SplitButton::SegmentButton::getParentDecorationBounds() const {
  Rect coverage = SurfaceWidget::getParentDecorationBounds();
  if (owner() == nullptr || width() == 0 || height() == 0) return coverage;
  const SegmentButton& other =
      isTrailing() ? owner()->primary_ : owner()->trailing_;
  if (other.width() == 0 || other.height() == 0) return coverage;
  bool on_left = offsetLeft() < other.offsetLeft();
  int a = on_left ? offsetLeft() + width() : other.offsetLeft() + other.width();
  int b = on_left ? other.offsetLeft() : offsetLeft();
  int midpoint = a + (b - a) / 2;
  Rect half = on_left
                  ? Rect(-16384, coverage.yMin(), midpoint - 1, coverage.yMax())
                  : Rect(midpoint, coverage.yMin(), 16383, coverage.yMax());
  return Rect::Intersect(coverage, half);
}

void SplitButton::SegmentButton::emitPersistentDecoration(
    PaintContext& ctx) const {
  if (width() == 0 || height() == 0) return;
  Rect parent_clip = getParentDecorationBounds();
  PaintContext clipped =
      ctx.clipped(parent_clip.translate(-offsetLeft(), -offsetTop()));
  SurfaceWidget::emitPersistentDecoration(clipped);
}

ClickActivationPolicy SplitButton::SegmentButton::getClickActivationPolicy()
    const {
  return isTrailing() ? ClickActivationPolicy::kAfterForcedFinalFrame
                      : ClickActivationPolicy::kImmediateContinueAnimation;
}

}  // namespace roo_windows::material3
