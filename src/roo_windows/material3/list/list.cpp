#include "roo_windows/material3/list/list.h"

#include <algorithm>
#include <limits>
#include <new>

#include "roo_display/shape/smooth.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_icons/filled/24/navigation.h"
#include "roo_logging.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/child_layout.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/internal/density.h"
#include "roo_windows/material3/list/dynamic_list.h"
#include "roo_windows/material3/list/internal/list_row_geometry.h"
#include "roo_windows/material3/list/list_geometry.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_block.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows {
namespace material3 {

namespace {

// Matches Jetpack Compose ListItemDefaults.SegmentedGap.
constexpr int16_t kSegmentedListGapDp = 2;
constexpr int16_t kExpressiveOuterCornerRadiusDp = 12;
constexpr int16_t kExpressiveInnerCornerRadiusDp = 4;
constexpr int16_t kExpressiveStandardSeparatorDp = 2;
constexpr int16_t kDividerThicknessDp = 1;
constexpr int16_t kAvatarSizeDp = 40;

using RowTokens = internal::ListRowTokens;
using TextSlotMetrics = internal::ListTextSlotMetrics;
using RowLayoutMetrics = internal::ListRowLayoutMetrics;

int16_t DividerThicknessPx() {
  return std::max<int16_t>(1, Scaled(kDividerThicknessDp));
}

BorderStyle BorderStyleFor(const ListEntryVisualContext& context) {
  if (context.variant == ListVariant::kBaseline) {
    return BorderStyle(0, 0);
  }

  uint8_t outer_radius =
      static_cast<uint8_t>(Scaled(kExpressiveOuterCornerRadiusDp));
  uint8_t inner_radius =
      static_cast<uint8_t>(Scaled(kExpressiveInnerCornerRadiusDp));

  // Selected expressive rows keep a fully rounded item-local outline instead
  // of inheriting segmented first/middle/last corner trimming.
  if (context.selected) {
    return BorderStyle(outer_radius, outer_radius, outer_radius, outer_radius,
                       0);
  }

  switch (context.position) {
    case ListItemPosition::kSingle:
      return BorderStyle(outer_radius, outer_radius, outer_radius, outer_radius,
                         0);
    case ListItemPosition::kFirst:
      return BorderStyle(outer_radius, outer_radius, inner_radius, inner_radius,
                         0);
    case ListItemPosition::kMiddle:
      return BorderStyle(inner_radius, inner_radius, inner_radius, inner_radius,
                         0);
    case ListItemPosition::kLast:
      return BorderStyle(inner_radius, inner_radius, outer_radius, outer_radius,
                         0);
  }
  return BorderStyle(outer_radius, 0);
}

const TextStyle& FontForOverline() { return text_style_label_small(); }

const TextStyle& FontForHeadline() { return text_style_body_large(); }

const TextStyle& FontForSupporting() { return text_style_body_medium(); }

// Converts descriptor text to an owning std::string for TextBlock.
std::string ToString(roo::string_view text) {
  return std::string(text.data(), text.size());
}

// Phase 7 policy split: single-line truncate stays on the cheap label path;
// any wrap or multi-line mode opts into the heavier block-text widget.
bool UsesBlockSlot(ListTextPolicy policy) {
  return policy.max_lines > 1 || policy.overflow == TextOverflowPolicy::kWrap;
}

Dimensions SuggestedMinimumChild(const Widget* child) {
  if (child == nullptr || child->isGone()) return Dimensions(0, 0);
  return AddChildMargins(*child, child->getSuggestedMinimumDimensions());
}

}  // namespace

namespace internal {
DividerMetrics ResolveDividerMetrics(const ListEntryVisualContext& context,
                                     DividerInsetHint hint, XDim width,
                                     XDim x_offset, YDim y) {
  if (!context.show_divider || context.divider_mode == DividerMode::kNone ||
      width <= 0) {
    return DividerMetrics{0, -1, 0, false};
  }

  int16_t start_inset = 0;
  int16_t end_inset = 0;
  if (context.divider_mode == DividerMode::kInset) {
    start_inset = context.divider_start_inset;
    end_inset = context.divider_end_inset;
    start_inset = std::max<int16_t>(start_inset, hint.start_inset);
    end_inset = std::max<int16_t>(end_inset, hint.end_inset);
  }

  int16_t start_x = x_offset + start_inset;
  int16_t end_x = x_offset + width - 1 - end_inset;
  if (start_x > end_x) return DividerMetrics{0, -1, y, false};
  return DividerMetrics{start_x, end_x, y, true};
}

ListItemPosition PositionForIndex(int idx, int count) {
  if (count <= 1) return ListItemPosition::kSingle;
  if (idx == 0) return ListItemPosition::kFirst;
  if (idx == count - 1) return ListItemPosition::kLast;
  return ListItemPosition::kMiddle;
}

bool ShouldShowDivider(const ListDividerPolicy& divider_policy, int idx,
                       int count, bool selected, bool next_selected) {
  if (divider_policy.mode == DividerMode::kNone || idx >= count - 1) {
    return false;
  }
  // Selecting or deselecting a neighbor can hide or restore this divider when
  // both rows may be selected together (multiple selection or independent
  // selection groups). Within one single-selection group, this cannot happen.
  if (divider_policy.suppress_between_selected && selected && next_selected) {
    return false;
  }
  return true;
}

int16_t ResolveGap(ListStyle style, DividerMode divider_mode,
                   const ListEntryVisualContext& previous,
                   const ListEntryVisualContext& next) {
  int16_t gap = 0;
  if (style == ListStyle::kSegmented && divider_mode == DividerMode::kNone) {
    gap = Scaled(kSegmentedListGapDp);
  } else if (previous.variant == ListVariant::kExpressive &&
             previous.style == ListStyle::kStandard &&
             (previous.show_divider || (previous.selected && next.selected))) {
    gap = Scaled(kExpressiveStandardSeparatorDp);
  }

  if (previous.show_divider) {
    gap += DividerThicknessPx();
  }
  return gap;
}

}  // namespace internal

using internal::DividerMetrics;
using internal::PositionForIndex;
using internal::ResolveDividerMetrics;
using internal::ShouldShowDivider;

// Keeps the avatar-specific paint logic private to the convenience item layer
// instead of introducing a broader public widget before the API needs one.
class AvatarVisual : public Widget {
 public:
  AvatarVisual(ApplicationContext& context, roo::string_view initials)
      : Widget(context), initials_(initials) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    int16_t side = Scaled(kAvatarSizeDp);
    return Dimensions(side, side);
  }

  void paint(PaintContext& ctx) const override {
    Rect rect = bounds();
    if (rect.empty()) return;

    int16_t diameter = std::min<int16_t>(rect.width(), rect.height());
    roo_display::Color original_bg = ctx.bgcolor();
    auto background = roo_display::SmoothFilledCircle(
        {0.5f * static_cast<float>(diameter - 1),
         0.5f * static_cast<float>(diameter - 1)},
        0.5f * static_cast<float>(diameter),
        theme().material3Theme().color.primaryContainer);

    if (!initials_.empty()) {
      roo_display::ClippedStringViewLabel initials_label(
          initials_, FontForHeadline().font(),
          theme().material3Theme().color.onPrimaryContainer);
      roo_display::Offset text_offset =
          (roo_display::kCenter | roo_display::kMiddle)
              .resolveOffset(roo_display::Box(0, 0, diameter - 1, diameter - 1),
                             initials_label.anchorExtents());
      Rect text_bounds = Rect(initials_label.extents())
                             .translate(text_offset.dx, text_offset.dy)
                             .translate(rect.xMin(), rect.yMin());
      ctx.setBgcolor(theme().material3Theme().color.primaryContainer);
      ctx.drawTiled(initials_label, text_bounds,
                    roo_display::kCenter | roo_display::kMiddle);
      ctx.addExclusion(text_bounds);
    }

    ctx.setBgcolor(original_bg);
    ctx.drawTiled(background, rect, roo_display::kCenter | roo_display::kMiddle,
                  false);
  }

  roo::string_view initials() const { return initials_; }

  void setInitials(roo::string_view initials) {
    if (initials_ == initials) return;
    initials_ = initials;
    setDirty();
  }

 private:
  roo::string_view initials_;
};

StandardListItemInit StandardListItemInit::OneLine(roo::string_view headline,
                                                   Widget* leading,
                                                   Widget* trailing) {
  StandardListItemInit init;
  init.headline = headline;
  init.leading = leading;
  init.trailing = trailing;
  return init;
}

StandardListItemInit StandardListItemInit::TwoLine(roo::string_view headline,
                                                   roo::string_view supporting,
                                                   Widget* leading,
                                                   Widget* trailing) {
  StandardListItemInit init = OneLine(headline, leading, trailing);
  init.supporting = supporting;
  return init;
}

StandardListItemInit StandardListItemInit::ThreeLine(
    roo::string_view headline, roo::string_view supporting,
    roo::string_view overline, Widget* leading, Widget* trailing,
    Widget* body) {
  StandardListItemInit init = TwoLine(headline, supporting, leading, trailing);
  init.overline = overline;
  init.body = body;
  init.supporting_policy.max_lines = overline.empty() ? 2 : 1;
  init.leading_alignment = VerticalVisualAlignment::kTop;
  init.trailing_alignment = VerticalVisualAlignment::kTop;
  init.prefer_top_text_alignment = true;
  return init;
}

ExpandablePanel::ExpandablePanel(ApplicationContext& context)
    : Container(context),
      content_(),
      expansion_fraction_(0.0f),
      animation_duration_millis_(0),
      expanded_(false),
      snap_when_presented_(false) {
  context.presentations().observe(*this);
}

void ExpandablePanel::setContent(WidgetRef content) {
  Widget* incoming = content.get();
  if (incoming == content_.get()) {
    return;
  }

  if (content_.get() != nullptr) {
    detachChild(content_.get());
  }

  content_.~WidgetRef();
  new (&content_) WidgetRef(std::move(content));
  if (content_.get() != nullptr) {
    CHECK(content_.get()->parent() == nullptr);
    attachChild(WidgetRef(*content_.get()));
  }

  requestLayout();
  invalidateInterior();
}

void ExpandablePanel::clearContent() {
  if (content_.get() == nullptr) return;
  detachChild(content_.get());
  content_.~WidgetRef();
  new (&content_) WidgetRef();
  requestLayout();
  invalidateInterior();
}

void ExpandablePanel::setExpanded(bool expanded, bool animate) {
  const float requested_fraction = expanded ? 1.0f : 0.0f;
  if (expanded_ == expanded &&
      (animate || expansion_fraction_ == requested_fraction)) {
    return;
  }

  expanded_ = expanded;
  if (!animate || animation_duration_millis_ == 0) {
    snapToRequestedState();
  } else {
    animateToRequestedState();
  }

  requestLayout();
  invalidateInterior();
}

bool ExpandablePanel::isExpanded() const { return expanded_; }

bool ExpandablePanel::isAnimating() const {
  if (animation_duration_millis_ == 0 || snap_when_presented_) return false;
  return expansion_fraction_ != (expanded_ ? 1.0f : 0.0f);
}

void ExpandablePanel::setAnimationDuration(uint16_t millis) {
  if (animation_duration_millis_ == millis) return;
  animation_duration_millis_ = millis;
  if (animation_duration_millis_ == 0) {
    snapToRequestedState();
  } else if (isAnimating()) {
    animateToRequestedState();
  }
  requestLayout();
  invalidateInterior();
}

Dimensions ExpandablePanel::getSuggestedMinimumDimensions() const {
  const Widget* content = content_.get();
  if (content == nullptr || content->isGone()) {
    return Dimensions(0, 0);
  }
  Dimensions suggested =
      AddChildMargins(*content, content->getSuggestedMinimumDimensions());
  return Dimensions(suggested.width(),
                    resolveVisibleHeight(suggested.height()));
}

void ExpandablePanel::animateToRequestedState() {
  AnimationRegistry& animations = context().animations();
  const float target = expanded_ ? 1.0f : 0.0f;
  if (expansion_fraction_ == target) {
    animations.cancel(*this, kExpansion);
    snap_when_presented_ = false;
    return;
  }

  if (presentationState() == PresentationState::kDetached) {
    animations.cancel(*this, kExpansion);
    snap_when_presented_ = true;
    return;
  }

  const float distance = target > expansion_fraction_
                             ? target - expansion_fraction_
                             : expansion_fraction_ - target;
  uint32_t travel_millis = static_cast<uint32_t>(
      static_cast<float>(animation_duration_millis_) * distance + 0.5f);
  if (travel_millis == 0) travel_millis = 1;

  AnimationStatus status;
  if (animations.contains(*this, kExpansion)) {
    status = animations.retarget(*this, kExpansion, target,
                                 roo_time::Millis(travel_millis));
  } else {
    status =
        animations.start(*this, kExpansion,
                         AnimationSpec::Value(expansion_fraction_, target,
                                              roo_time::Millis(travel_millis)));
  }
  if (status != AnimationStatus::kOk) {
    snapToRequestedState();
    return;
  }

  snap_when_presented_ = false;
  if (presentationState() == PresentationState::kHidden) {
    animations.pause(*this, kExpansion);
  }
}

void ExpandablePanel::snapToRequestedState() {
  context().animations().cancel(*this, kExpansion);
  expansion_fraction_ = expanded_ ? 1.0f : 0.0f;
  snap_when_presented_ = false;
}

int16_t ExpandablePanel::resolveVisibleHeight(int16_t full_height) const {
  if (full_height <= 0) return 0;
  return static_cast<int16_t>(static_cast<float>(full_height) *
                              expansion_fraction_);
}

Dimensions ExpandablePanel::onMeasure(WidthSpec width, HeightSpec height) {
  Widget* content = content_.get();
  if (content == nullptr || content->isGone()) {
    return Dimensions(width.resolveSize(0), height.resolveSize(0));
  }

  Dimensions measured = MeasureChildWithMargins(
      *content, width, HeightSpec::Unspecified(height.value()));
  int16_t visible_height = resolveVisibleHeight(measured.height());
  return Dimensions(width.resolveSize(measured.width()),
                    height.resolveSize(visible_height));
}

void ExpandablePanel::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  Widget* content = content_.get();
  if (content == nullptr || content->isGone()) return;
  if (rect.width() <= 0 || rect.height() <= 0) {
    LayoutChildWithMargins(*content, Rect(0, 0, -1, -1));
    return;
  }

  // Keep child layout clipped to the panel's current visible height so child
  // max-bounds cannot spill into sibling paint/invalidation regions during
  // expand/collapse animation.
  LayoutChildWithMargins(*content,
                         Rect(0, 0, rect.width() - 1, rect.height() - 1));
}

void ExpandablePanel::onAnimationFrame(AnimationTag tag,
                                       const AnimationSample& sample) {
  if (tag != kExpansion) {
    Container::onAnimationFrame(tag, sample);
    return;
  }
  if (expansion_fraction_ == sample.value) return;
  expansion_fraction_ = sample.value;
  requestLayout();
  invalidateInterior();
}

void ExpandablePanel::onPresentationChanged(const PresentationChange& change) {
  AnimationRegistry& animations = context().animations();
  if (change.detached_since_delivery ||
      change.state == PresentationState::kDetached) {
    animations.cancel(*this, kExpansion);
    snap_when_presented_ = expansion_fraction_ != (expanded_ ? 1.0f : 0.0f);
    return;
  }
  if (change.state == PresentationState::kHidden) {
    animations.pause(*this, kExpansion);
    return;
  }
  if (snap_when_presented_) {
    snapToRequestedState();
    requestLayout();
    invalidateInterior();
    return;
  }
  animations.resume(*this, kExpansion);
}

int ExpandablePanel::getChildrenCount() const {
  return (content_.get() != nullptr && !content_.get()->isGone()) ? 1 : 0;
}

const Widget& ExpandablePanel::getChild(int idx) const {
  CHECK_EQ(0, idx);
  CHECK(content_.get() != nullptr);
  return *content_.get();
}

Widget& ExpandablePanel::getChild(int idx) {
  return const_cast<Widget&>(
      static_cast<const ExpandablePanel&>(*this).getChild(idx));
}

ListEntry::ListEntry(ApplicationContext& context)
    : Material3Container(context),
      item_(nullptr),
      leading_child_(nullptr),
      overline_text_(nullptr),
      headline_text_(nullptr),
      supporting_text_(nullptr),
      trailing_child_(nullptr),
      body_child_(nullptr),
      overline_mode_(TextSlotMode::kNone),
      headline_mode_(TextSlotMode::kNone),
      supporting_mode_(TextSlotMode::kNone),
      visual_context_() {}

ListEntry::~ListEntry() { clearItem(); }

bool ListEntry::hasItem() const { return item_ != nullptr; }

ListItem* ListEntry::item() { return item_; }

const ListItem* ListEntry::item() const { return item_; }

void ListEntry::clearTextSlot(Widget*& slot, TextSlotMode& mode) {
  if (slot != nullptr) {
    detachChild(slot);
    slot = nullptr;
  }
  mode = TextSlotMode::kNone;
}

void ListEntry::clearTextSlots() {
  clearTextSlot(supporting_text_, supporting_mode_);
  clearTextSlot(headline_text_, headline_mode_);
  clearTextSlot(overline_text_, overline_mode_);
}

void ListEntry::detachBoundChildren() {
  clearTextSlots();
  if (body_child_ != nullptr) {
    detachChild(body_child_);
    body_child_ = nullptr;
  }
  if (trailing_child_ != nullptr) {
    detachChild(trailing_child_);
    trailing_child_ = nullptr;
  }
  if (leading_child_ != nullptr) {
    detachChild(leading_child_);
    leading_child_ = nullptr;
  }
  item_ = nullptr;
}

void ListEntry::releaseTextViews() {
  auto release = [](Widget* slot, TextSlotMode mode) {
    if (slot == nullptr) return;
    if (mode == TextSlotMode::kLabel) {
      static_cast<StringViewLabel*>(slot)->setText({});
    } else if (mode == TextSlotMode::kBlock) {
      static_cast<TextBlock*>(slot)->setText({});
    }
    slot->setVisibility(Visibility::kGone);
  };
  release(overline_text_, overline_mode_);
  release(headline_text_, headline_mode_);
  release(supporting_text_, supporting_mode_);
}

void ListEntry::syncTextSlotsFromItem() {
  if (item_ == nullptr) {
    clearTextSlots();
    return;
  }

  // Keeps each slot in one of two stable widget classes and only allows class
  // transitions during bind/clear/rebind/refresh, never during paint.
  auto sync_slot = [this](Widget*& slot, TextSlotMode& mode,
                          roo::string_view text, ListTextPolicy policy,
                          const TextStyle& text_style,
                          roo_display::Color color) {
    if (text.empty()) {
      if (!retainsTextSlots()) {
        clearTextSlot(slot, mode);
      } else if (slot != nullptr) {
        if (mode == TextSlotMode::kLabel) {
          static_cast<StringViewLabel*>(slot)->setText({});
        } else {
          static_cast<TextBlock*>(slot)->setText({});
        }
        slot->setVisibility(Visibility::kGone);
      }
      return;
    }
    if (slot != nullptr) slot->setVisibility(Visibility::kVisible);

    TextSlotMode desired_mode =
        UsesBlockSlot(policy) ? TextSlotMode::kBlock : TextSlotMode::kLabel;
    if (retainsTextSlots() && parent() != nullptr) {
      CHECK(slot != nullptr) << "prepare all text slots before binding";
      CHECK(mode == desired_mode) << "binding cannot change text slot classes";
    }
    if (slot == nullptr || mode != desired_mode) {
      clearTextSlot(slot, mode);
      if (desired_mode == TextSlotMode::kLabel) {
        auto owned = std::make_unique<StringViewLabel>(
            context(), text, text_style, color, kGravityLeft | kGravityMiddle);
        slot = owned.get();
        mode = TextSlotMode::kLabel;
        attachChild(WidgetRef(std::move(owned)));
        return;
      }
      auto owned = std::make_unique<TextBlock>(
          context(), ToString(text), text_style, color,
          roo_display::kLeft | roo_display::kTop);
      owned->setTextAlign(TextAlign::kStart);
      owned->setWrapMode(policy.overflow == TextOverflowPolicy::kWrap
                             ? TextWrapMode::kWordWrap
                             : TextWrapMode::kNoWrap);
      owned->setMaxLines(std::max<uint16_t>(1, policy.max_lines));
      owned->setEllipsize(policy.overflow == TextOverflowPolicy::kTruncate);
      slot = owned.get();
      mode = TextSlotMode::kBlock;
      attachChild(WidgetRef(std::move(owned)));
      return;
    }

    if (mode == TextSlotMode::kLabel) {
      StringViewLabel* label = static_cast<StringViewLabel*>(slot);
      label->setTextStyle(text_style);
      label->setText(text);
      label->setColor(color);
      return;
    }
    TextBlock* block = static_cast<TextBlock*>(slot);
    block->setTextStyle(text_style);
    block->setText(ToString(text));
    block->setColor(color);
    block->setWrapMode(policy.overflow == TextOverflowPolicy::kWrap
                           ? TextWrapMode::kWordWrap
                           : TextWrapMode::kNoWrap);
    block->setMaxLines(std::max<uint16_t>(1, policy.max_lines));
    block->setEllipsize(policy.overflow == TextOverflowPolicy::kTruncate);
  };

  sync_slot(overline_text_, overline_mode_, item_->overlineText(),
            item_->overlinePolicy(), FontForOverline(), supportingColor());
  sync_slot(headline_text_, headline_mode_, item_->headlineText(),
            item_->headlinePolicy(), headlineTextStyle(), headlineColor());
  sync_slot(supporting_text_, supporting_mode_, item_->supportingText(),
            item_->supportingPolicy(), FontForSupporting(), supportingColor());
}

const TextStyle& ListEntry::headlineTextStyle() const {
  return FontForHeadline();
}

Color ListEntry::headlineColor() const {
  const ColorScheme& colors = theme().material3Theme().color;
  return visual_context_.selected &&
                 visual_context_.variant == ListVariant::kExpressive
             ? colors.onSecondaryContainer
             : colors.onSurface;
}

Color ListEntry::supportingColor() const {
  const ColorScheme& colors = theme().material3Theme().color;
  return visual_context_.selected &&
                 visual_context_.variant == ListVariant::kExpressive
             ? colors.onSecondaryContainer
             : colors.onSurfaceVariant;
}

Color ListEntry::defaultColor() const { return supportingColor(); }

// Updates retained text widgets without reading a possibly unbound model.
void ListEntry::syncTextColors() {
  auto set_color = [](Widget* slot, TextSlotMode mode, Color color) {
    if (mode == TextSlotMode::kLabel) {
      static_cast<StringViewLabel*>(slot)->setColor(color);
    } else if (mode == TextSlotMode::kBlock) {
      static_cast<TextBlock*>(slot)->setColor(color);
    }
  };
  set_color(headline_text_, headline_mode_, headlineColor());
  set_color(overline_text_, overline_mode_, supportingColor());
  set_color(supporting_text_, supporting_mode_, supportingColor());
}

void ListEntry::setItem(ListItem& item) {
  if (item_ == &item) {
    refreshFromItem();
    return;
  }
  // A binding owns a stable borrowed slot set for its whole lifetime, so a
  // rebind always detaches the old children before attaching the new ones.
  detachBoundChildren();
  Widget* leading = item.leading();
  Widget* trailing = item.trailing();
  Widget* body = item.body();
  CHECK(leading == nullptr || leading != trailing);
  CHECK(leading == nullptr || leading != body);
  CHECK(trailing == nullptr || trailing != body);
  item_ = &item;
  if (leading != nullptr) {
    CHECK(leading->parent() == nullptr);
    attachChild(WidgetRef(*leading));
    leading_child_ = leading;
  }
  if (trailing != nullptr) {
    CHECK(trailing->parent() == nullptr);
    attachChild(WidgetRef(*trailing));
    trailing_child_ = trailing;
  }
  if (body != nullptr) {
    CHECK(body->parent() == nullptr);
    attachChild(WidgetRef(*body));
    body_child_ = body;
  }
  syncTextSlotsFromItem();
  invalidateInterior();
  requestLayout();
}

void ListEntry::clearItem() {
  if (item_ == nullptr && leading_child_ == nullptr &&
      overline_text_ == nullptr && headline_text_ == nullptr &&
      supporting_text_ == nullptr && trailing_child_ == nullptr &&
      body_child_ == nullptr) {
    return;
  }
  detachBoundChildren();
  invalidateInterior();
  requestLayout();
}

void ListEntry::refreshFromItem() {
  if (item_ == nullptr) return;
  CHECK(item_->leading() == leading_child_);
  CHECK(item_->trailing() == trailing_child_);
  CHECK(item_->body() == body_child_);
  syncTextSlotsFromItem();
  invalidateInterior();
  requestLayout();
}

void ListEntry::setVisualContext(const ListEntryVisualContext& context) {
  if (visual_context_.variant == context.variant &&
      visual_context_.style == context.style &&
      visual_context_.position == context.position &&
      visual_context_.selected == context.selected &&
      visual_context_.enabled == context.enabled &&
      visual_context_.pressed == context.pressed &&
      visual_context_.focused == context.focused &&
      visual_context_.hovered == context.hovered &&
      visual_context_.show_divider == context.show_divider &&
      visual_context_.divider_mode == context.divider_mode &&
      visual_context_.divider_start_inset == context.divider_start_inset &&
      visual_context_.divider_end_inset == context.divider_end_inset &&
      visual_context_.density == context.density) {
    return;
  }
  const bool colors_changed = visual_context_.selected != context.selected ||
                              visual_context_.variant != context.variant;
  const bool density_changed = visual_context_.density != context.density;
  visual_context_ = context;
  if (density_changed) {
    requestLayoutDescending();
    invalidateDescending();
  }
  if (colors_changed) syncTextColors();
  invalidateInterior();
}

const ListEntryVisualContext& ListEntry::visualContext() const {
  return visual_context_;
}

::roo_windows::material3::ColorToken ListEntry::containerRole() const {
  if (visual_context_.selected &&
      visual_context_.variant == ListVariant::kExpressive) {
    return ::roo_windows::material3::ColorToken::kSecondaryContainer;
  }
  const ListTheme& list = theme().material3Theme().components.list;
  return visual_context_.style == ListStyle::kSegmented
             ? list.segmentedContainer
             : list.standardContainer;
}

void ListEntry::setDensity(Density density) {
  ListEntryVisualContext context = visual_context_;
  context.density = DensityOverride::Explicit(density);
  setVisualContext(context);
}

void ListEntry::clearDensityOverride() {
  ListEntryVisualContext context = visual_context_;
  context.density = DensityOverride{};
  setVisualContext(context);
}

int8_t ListEntry::resolvedDensityLevel() const {
  return internal::ResolveDensityLevel(
      visual_context_.density.resolve(theme().material3Theme().density));
}

BorderStyle ListEntry::getBorderStyle() const {
  return BorderStyleFor(visual_context_);
}

Dimensions ListEntry::getSuggestedMinimumDimensions() const {
  int8_t level = resolvedDensityLevel();
  const RowTokens tokens =
      internal::ResolveListRowTokens(visual_context_.variant, level);
  Dimensions leading = SuggestedMinimumChild(leading_child_);
  Dimensions trailing = SuggestedMinimumChild(trailing_child_);
  Dimensions body = SuggestedMinimumChild(body_child_);
  TextSlotMetrics text =
      internal::ResolveListTextSlotMetrics(item_, headlineTextStyle());

  bool has_leading = leading.width() > 0 || leading.height() > 0;
  bool has_trailing = trailing.width() > 0 || trailing.height() > 0;
  bool has_text = text.width > 0 || text.height > 0;
  bool has_body = body.width() > 0 || body.height() > 0;

  int16_t horizontal_gaps = 0;
  if (has_leading && has_text) horizontal_gaps += tokens.slot_gap;
  if (has_trailing && (has_text || has_leading)) {
    horizontal_gaps += tokens.slot_gap;
  }

  int16_t desired_main_width = tokens.horizontal_padding * 2 + leading.width() +
                               trailing.width() + text.width + horizontal_gaps;
  int16_t desired_body_width =
      has_body ? tokens.horizontal_padding * 2 +
                     std::max<int16_t>(body.width(), text.width)
               : 0;
  internal::ListRowGeometryInput input{
      visual_context_.variant,
      text.line_count,
      item_ != nullptr && item_->preferTopTextAlignment(),
      VerticalVisualAlignment::kMiddle,
      VerticalVisualAlignment::kMiddle,
      text.height,
      leading,
      trailing,
      body};
  return Dimensions(std::max(desired_main_width, desired_body_width),
                    internal::ResolveListRowGeometry(input, level).height);
}

PreferredSize ListEntry::getPreferredSize() const {
  PreferredSize legacy = Widget::getPreferredSize();
  if (resolvedDensityLevel() == 0) return legacy;
  // Let measurement establish compact content floors from actual slots rather
  // than imposing a cheap descriptor budget as an exact parent constraint.
  return {legacy.width(), PreferredSize::WrapContentHeight()};
}

bool ListEntry::isClickable() const {
  return item_ != nullptr && item_->isInvokable();
}

void ListEntry::onClicked() {
  if (parent() != nullptr && parent()->invokeChild(*this)) return;
  if (item_ != nullptr) {
    item_->invoke();
  }
  Widget::onClicked();
}

void ListEntry::onFocusChanged(bool focused) {
  if (visual_context_.focused == focused) return;
  visual_context_.focused = focused;
  invalidateInterior();
}

int ListEntry::getChildrenCount() const {
  int count = 0;
  if (leading_child_ != nullptr) ++count;
  if (overline_text_ != nullptr) ++count;
  if (headline_text_ != nullptr) ++count;
  if (supporting_text_ != nullptr) ++count;
  if (trailing_child_ != nullptr) ++count;
  if (body_child_ != nullptr) ++count;
  return count;
}

const Widget& ListEntry::getChild(int idx) const {
  CHECK(idx >= 0);
  if (leading_child_ != nullptr) {
    if (idx == 0) return *leading_child_;
    --idx;
  }
  if (overline_text_ != nullptr) {
    if (idx == 0) return *overline_text_;
    --idx;
  }
  if (headline_text_ != nullptr) {
    if (idx == 0) return *headline_text_;
    --idx;
  }
  if (supporting_text_ != nullptr) {
    if (idx == 0) return *supporting_text_;
    --idx;
  }
  if (trailing_child_ != nullptr) {
    if (idx == 0) return *trailing_child_;
    --idx;
  }
  if (body_child_ != nullptr) {
    if (idx == 0) return *body_child_;
  }
  LOG(FATAL) << "Invalid material3::ListEntry child index";
  return *static_cast<const Widget*>(nullptr);
}

Widget& ListEntry::getChild(int idx) {
  return const_cast<Widget&>(
      static_cast<const ListEntry&>(*this).getChild(idx));
}

Dimensions ListEntry::onMeasure(WidthSpec width, HeightSpec height) {
  RowLayoutMetrics layout = internal::ResolveListRowLayout(
      *this, width, height, resolvedDensityLevel());
  return Dimensions(layout.width, layout.height);
}

void ListEntry::onLayout(bool changed, const Rect& rect) {
  const RowLayoutMetrics layout = internal::ResolveListRowLayout(
      *this, WidthSpec::Exactly(rect.width()),
      HeightSpec::Exactly(rect.height()), resolvedDensityLevel());
  internal::LayoutListRow(*this, layout);
}

StandardListItem::StandardListItem(const StandardListItemInit& init)
    : overline_(init.overline),
      headline_(init.headline),
      supporting_(init.supporting),
      overline_policy_(init.overline_policy),
      headline_policy_(init.headline_policy),
      supporting_policy_(init.supporting_policy),
      leading_(init.leading),
      trailing_(init.trailing),
      body_(init.body),
      divider_inset_hint_(init.divider_inset_hint),
      leading_alignment_(static_cast<uint8_t>(init.leading_alignment)),
      trailing_alignment_(static_cast<uint8_t>(init.trailing_alignment)),
      prefer_top_text_alignment_(init.prefer_top_text_alignment) {}

roo::string_view StandardListItem::overlineText() const { return overline_; }

roo::string_view StandardListItem::headlineText() const { return headline_; }

roo::string_view StandardListItem::supportingText() const {
  return supporting_;
}

ListTextPolicy StandardListItem::overlinePolicy() const {
  return overline_policy_;
}

ListTextPolicy StandardListItem::headlinePolicy() const {
  return headline_policy_;
}

ListTextPolicy StandardListItem::supportingPolicy() const {
  return supporting_policy_;
}

Widget* StandardListItem::leading() { return leading_; }

const Widget* StandardListItem::leading() const { return leading_; }

Widget* StandardListItem::trailing() { return trailing_; }

const Widget* StandardListItem::trailing() const { return trailing_; }

Widget* StandardListItem::body() { return body_; }

const Widget* StandardListItem::body() const { return body_; }

VerticalVisualAlignment StandardListItem::leadingAlignment() const {
  return static_cast<VerticalVisualAlignment>(leading_alignment_);
}

VerticalVisualAlignment StandardListItem::trailingAlignment() const {
  return static_cast<VerticalVisualAlignment>(trailing_alignment_);
}

DividerInsetHint StandardListItem::dividerInsetHint() const {
  return divider_inset_hint_;
}

bool StandardListItem::preferTopTextAlignment() const {
  return prefer_top_text_alignment_;
}

Widget* StandardListItem::leadingWidget() { return leading_; }

Widget* StandardListItem::trailingWidget() { return trailing_; }

Widget* StandardListItem::bodyWidget() { return body_; }

HeadlineListItem::HeadlineListItem(roo::string_view headline,
                                   ListTextPolicy headline_policy)
    : headline_(headline), headline_policy_(headline_policy) {}

roo::string_view HeadlineListItem::headlineText() const { return headline_; }

ListTextPolicy HeadlineListItem::headlinePolicy() const {
  return headline_policy_;
}

roo::string_view HeadlineListItem::headline() const { return headline_; }

void HeadlineListItem::setHeadline(roo::string_view headline) {
  headline_ = headline;
}

void HeadlineListItem::setHeadlinePolicy(ListTextPolicy policy) {
  headline_policy_ = policy;
}

HeadlineSupportingListItemBase::HeadlineSupportingListItemBase(
    roo::string_view headline, roo::string_view supporting,
    ListTextPolicy headline_policy, ListTextPolicy supporting_policy)
    : headline_(headline),
      supporting_(supporting),
      headline_policy_(headline_policy),
      supporting_policy_(supporting_policy) {}

roo::string_view HeadlineSupportingListItemBase::headlineText() const {
  return headline_;
}

roo::string_view HeadlineSupportingListItemBase::supportingText() const {
  return supporting_;
}

ListTextPolicy HeadlineSupportingListItemBase::headlinePolicy() const {
  return headline_policy_;
}

ListTextPolicy HeadlineSupportingListItemBase::supportingPolicy() const {
  return supporting_policy_;
}

roo::string_view HeadlineSupportingListItemBase::headline() const {
  return headline_;
}

roo::string_view HeadlineSupportingListItemBase::supporting() const {
  return supporting_;
}

void HeadlineSupportingListItemBase::setHeadline(roo::string_view headline) {
  headline_ = headline;
}

void HeadlineSupportingListItemBase::setSupportingText(
    roo::string_view supporting) {
  supporting_ = supporting;
}

void HeadlineSupportingListItemBase::setHeadlinePolicy(ListTextPolicy policy) {
  headline_policy_ = policy;
}

void HeadlineSupportingListItemBase::setSupportingPolicy(
    ListTextPolicy policy) {
  supporting_policy_ = policy;
}

SupportingTextListItem::SupportingTextListItem(roo::string_view headline,
                                               roo::string_view supporting,
                                               ListTextPolicy headline_policy,
                                               ListTextPolicy supporting_policy)
    : HeadlineSupportingListItemBase(headline, supporting, headline_policy,
                                     supporting_policy) {}

PictogramSupportingTextItem::PictogramSupportingTextItem(
    ApplicationContext& context, const roo_display::Pictogram& pictogram,
    roo::string_view headline, roo::string_view supporting,
    ListTextPolicy headline_policy, ListTextPolicy supporting_policy)
    : HeadlineSupportingListItemBase(headline, supporting, headline_policy,
                                     supporting_policy),
      leading_icon_(context, pictogram) {}

Widget* PictogramSupportingTextItem::leading() { return &leading_icon_; }

const Widget* PictogramSupportingTextItem::leading() const {
  return &leading_icon_;
}

Icon& PictogramSupportingTextItem::leadingIcon() { return leading_icon_; }

const Icon& PictogramSupportingTextItem::leadingIcon() const {
  return leading_icon_;
}

void PictogramSupportingTextItem::setPictogram(
    const roo_display::Pictogram& pictogram) {
  leading_icon_.setIcon(pictogram);
}

AvatarSupportingTextItem::AvatarSupportingTextItem(
    ApplicationContext& context, roo::string_view initials,
    roo::string_view headline, roo::string_view supporting,
    ListTextPolicy headline_policy, ListTextPolicy supporting_policy)
    : HeadlineSupportingListItemBase(headline, supporting, headline_policy,
                                     supporting_policy),
      leading_avatar_(std::make_unique<AvatarVisual>(context, initials)) {}

AvatarSupportingTextItem::~AvatarSupportingTextItem() = default;

Widget* AvatarSupportingTextItem::leading() { return leading_avatar_.get(); }

const Widget* AvatarSupportingTextItem::leading() const {
  return leading_avatar_.get();
}

roo::string_view AvatarSupportingTextItem::initials() const {
  return leading_avatar_ == nullptr ? roo::string_view{}
                                    : leading_avatar_->initials();
}

void AvatarSupportingTextItem::setInitials(roo::string_view initials) {
  if (leading_avatar_ != nullptr) {
    leading_avatar_->setInitials(initials);
  }
}

InvokableListItemBase::InvokableListItemBase(roo::string_view headline,
                                             roo::string_view supporting,
                                             ListTextPolicy headline_policy,
                                             ListTextPolicy supporting_policy,
                                             bool always_invokable)
    : HeadlineSupportingListItemBase(headline, supporting, headline_policy,
                                     supporting_policy),
      always_invokable_(always_invokable),
      action_only_(false),
      on_invoked_() {}

bool InvokableListItemBase::isInvokable() const {
  return always_invokable_ || static_cast<bool>(on_invoked_);
}

void InvokableListItemBase::invoke() {
  handleInvoke();
  notifyInvoked();
}

void InvokableListItemBase::setOnInvoked(std::function<void()> on_invoked) {
  on_invoked_ = std::move(on_invoked);
}

void InvokableListItemBase::notifyInvoked() {
  if (on_invoked_) {
    on_invoked_();
  }
}

void InvokableListItemBase::handleInvoke() {}

NavigationListItem::NavigationListItem(ApplicationContext& context,
                                       const roo_display::Pictogram& pictogram,
                                       roo::string_view headline,
                                       roo::string_view supporting,
                                       ListTextPolicy headline_policy,
                                       ListTextPolicy supporting_policy)
    : InvokableListItemBase(headline, supporting, headline_policy,
                            supporting_policy),
      leading_icon_(context, pictogram) {}

Widget* NavigationListItem::leading() { return &leading_icon_; }

const Widget* NavigationListItem::leading() const { return &leading_icon_; }

Icon& NavigationListItem::leadingIcon() { return leading_icon_; }

const Icon& NavigationListItem::leadingIcon() const { return leading_icon_; }

void NavigationListItem::setPictogram(const roo_display::Pictogram& pictogram) {
  leading_icon_.setIcon(pictogram);
}

AvatarNavigationListItem::AvatarNavigationListItem(
    ApplicationContext& context, roo::string_view initials,
    roo::string_view headline, roo::string_view supporting,
    ListTextPolicy headline_policy, ListTextPolicy supporting_policy)
    : InvokableListItemBase(headline, supporting, headline_policy,
                            supporting_policy),
      leading_avatar_(std::make_unique<AvatarVisual>(context, initials)),
      trailing_affordance_(context, ic_filled_24_navigation_chevron_right()) {}

AvatarNavigationListItem::~AvatarNavigationListItem() = default;

Widget* AvatarNavigationListItem::leading() { return leading_avatar_.get(); }

const Widget* AvatarNavigationListItem::leading() const {
  return leading_avatar_.get();
}

Widget* AvatarNavigationListItem::trailing() { return &trailing_affordance_; }

const Widget* AvatarNavigationListItem::trailing() const {
  return &trailing_affordance_;
}

roo::string_view AvatarNavigationListItem::initials() const {
  return leading_avatar_ == nullptr ? roo::string_view{}
                                    : leading_avatar_->initials();
}

Icon& AvatarNavigationListItem::trailingAffordance() {
  return trailing_affordance_;
}

const Icon& AvatarNavigationListItem::trailingAffordance() const {
  return trailing_affordance_;
}

void AvatarNavigationListItem::setInitials(roo::string_view initials) {
  if (leading_avatar_ != nullptr) {
    leading_avatar_->setInitials(initials);
  }
}

CheckboxListItem::CheckboxListItem(ApplicationContext& context,
                                   roo::string_view headline,
                                   roo::string_view supporting,
                                   Checkbox::OnOffState checked_state,
                                   AffordancePlacement placement,
                                   ListTextPolicy headline_policy,
                                   ListTextPolicy supporting_policy)
    : InvokableListItemBase(headline, supporting, headline_policy,
                            supporting_policy, true),
      checkbox_(context, checked_state),
      placement_(static_cast<uint8_t>(placement)) {
  checkbox_.setOnInteractiveChange([this]() { notifyInvoked(); });
}

CheckboxListItem::CheckboxListItem(ApplicationContext& context,
                                   roo::string_view headline,
                                   roo::string_view supporting, bool checked,
                                   AffordancePlacement placement,
                                   ListTextPolicy headline_policy,
                                   ListTextPolicy supporting_policy)
    : CheckboxListItem(
          context, headline, supporting,
          checked ? Checkbox::OnOffState::kOn : Checkbox::OnOffState::kOff,
          placement, headline_policy, supporting_policy) {}

Widget* CheckboxListItem::leading() {
  return placement_ == static_cast<uint8_t>(AffordancePlacement::kLeading)
             ? &checkbox_
             : nullptr;
}

const Widget* CheckboxListItem::leading() const {
  return placement_ == static_cast<uint8_t>(AffordancePlacement::kLeading)
             ? &checkbox_
             : nullptr;
}

Widget* CheckboxListItem::trailing() {
  return placement_ == static_cast<uint8_t>(AffordancePlacement::kTrailing)
             ? &checkbox_
             : nullptr;
}

const Widget* CheckboxListItem::trailing() const {
  return placement_ == static_cast<uint8_t>(AffordancePlacement::kTrailing)
             ? &checkbox_
             : nullptr;
}

Checkbox::OnOffState CheckboxListItem::checkedState() const {
  return checkbox_.onOffState();
}

bool CheckboxListItem::isChecked() const { return checkbox_.isOn(); }

void CheckboxListItem::setChecked(bool checked) {
  checked ? checkbox_.setOn() : checkbox_.setOff();
}

void CheckboxListItem::setCheckedState(Checkbox::OnOffState checked_state) {
  checkbox_.setOnOffState(checked_state);
}

void CheckboxListItem::setAffordancePlacement(AffordancePlacement placement) {
  placement_ = static_cast<uint8_t>(placement);
}

Checkbox& CheckboxListItem::checkbox() { return checkbox_; }

const Checkbox& CheckboxListItem::checkbox() const { return checkbox_; }

void CheckboxListItem::handleInvoke() { checkbox_.toggle(); }

RadioListItem::RadioListItem(ApplicationContext& context,
                             roo::string_view headline,
                             roo::string_view supporting, bool selected,
                             AffordancePlacement placement,
                             ListTextPolicy headline_policy,
                             ListTextPolicy supporting_policy)
    : InvokableListItemBase(headline, supporting, headline_policy,
                            supporting_policy, true),
      radio_button_(context, selected ? RadioButton::OnOffState::kOn
                                      : RadioButton::OnOffState::kOff),
      placement_(static_cast<uint8_t>(placement)) {
  radio_button_.setOnInteractiveChange([this]() { notifyInvoked(); });
}

Widget* RadioListItem::leading() {
  return placement_ == static_cast<uint8_t>(AffordancePlacement::kLeading)
             ? &radio_button_
             : nullptr;
}

const Widget* RadioListItem::leading() const {
  return placement_ == static_cast<uint8_t>(AffordancePlacement::kLeading)
             ? &radio_button_
             : nullptr;
}

Widget* RadioListItem::trailing() {
  return placement_ == static_cast<uint8_t>(AffordancePlacement::kTrailing)
             ? &radio_button_
             : nullptr;
}

const Widget* RadioListItem::trailing() const {
  return placement_ == static_cast<uint8_t>(AffordancePlacement::kTrailing)
             ? &radio_button_
             : nullptr;
}

bool RadioListItem::isSelected() const { return radio_button_.isOn(); }

void RadioListItem::setSelected(bool selected) {
  selected ? radio_button_.setOn() : radio_button_.setOff();
}

void RadioListItem::setAffordancePlacement(AffordancePlacement placement) {
  placement_ = static_cast<uint8_t>(placement);
}

RadioButton& RadioListItem::radioButton() { return radio_button_; }

const RadioButton& RadioListItem::radioButton() const { return radio_button_; }

void RadioListItem::handleInvoke() { radio_button_.setOn(); }

SwitchListItem::SwitchListItem(ApplicationContext& context,
                               roo::string_view headline,
                               roo::string_view supporting, bool on,
                               AffordancePlacement placement,
                               ListTextPolicy headline_policy,
                               ListTextPolicy supporting_policy)
    : InvokableListItemBase(headline, supporting, headline_policy,
                            supporting_policy, true),
      switch_(context, on ? Switch::OnOffState::kOn : Switch::OnOffState::kOff),
      placement_(static_cast<uint8_t>(placement)) {
  switch_.setOnInteractiveChange([this]() { notifyInvoked(); });
}

Widget* SwitchListItem::leading() {
  return placement_ == static_cast<uint8_t>(AffordancePlacement::kLeading)
             ? &switch_
             : nullptr;
}

const Widget* SwitchListItem::leading() const {
  return placement_ == static_cast<uint8_t>(AffordancePlacement::kLeading)
             ? &switch_
             : nullptr;
}

Widget* SwitchListItem::trailing() {
  return placement_ == static_cast<uint8_t>(AffordancePlacement::kTrailing)
             ? &switch_
             : nullptr;
}

const Widget* SwitchListItem::trailing() const {
  return placement_ == static_cast<uint8_t>(AffordancePlacement::kTrailing)
             ? &switch_
             : nullptr;
}

bool SwitchListItem::isOn() const { return switch_.isOn(); }

void SwitchListItem::setOn(bool on) { switch_.setOn(on); }

void SwitchListItem::toggle() { switch_.toggle(); }

void SwitchListItem::setAffordancePlacement(AffordancePlacement placement) {
  placement_ = static_cast<uint8_t>(placement);
}

Switch& SwitchListItem::switchControl() { return switch_; }

const Switch& SwitchListItem::switchControl() const { return switch_; }

void SwitchListItem::handleInvoke() { switch_.toggle(); }

List::List(ApplicationContext& context) : Container(context) {}

List::~List() {
  destroying_ = true;
  clear();
  for (Invocation* call = invocation_; call != nullptr; call = call->previous) {
    call->valid = false;
    call->owner = nullptr;
    call->row = nullptr;
  }
}

void List::checkNotCleaningBindings() const {
  for (const Section& section : sections_) {
    if (section.dynamic) {
      CHECK(!static_cast<DynamicListBase*>(section.widget)->cleaning());
    }
  }
}

void List::invalidateInvocations(Widget* section) {
  for (Invocation* call = invocation_; call != nullptr; call = call->previous) {
    if (section == nullptr || call->section == section) {
      call->valid = false;
      call->row = nullptr;
    }
  }
}

int List::findSection(const Widget* widget) const {
  for (int i = 0; i < static_cast<int>(sections_.size()); ++i) {
    if (sections_[i].widget == widget) return i;
  }
  return -1;
}

int List::sectionCount(const Section& section) const {
  if (section.widget->isGone()) return 0;
  if (!section.dynamic) return 1;
  const auto& dynamic = static_cast<const DynamicListBase&>(*section.widget);
  return dynamic.resetting() ? 0 : dynamic.elementCount();
}

bool List::selected(const Section& section, int index) const {
  if (!section.dynamic) {
    const ListItem* item =
        static_cast<const ListEntry*>(section.widget)->item();
    if (item != nullptr &&
        item->selectionParticipation() == SelectionParticipation::kAction)
      return false;
  }
  if (section.dynamic) {
    const auto& model =
        static_cast<const DynamicListBase*>(section.widget)->model();
    if (model.rowState(index).participation ==
        SelectionParticipation::kAction) {
      return false;
    }
    if (model.ownsSelection()) return model.rowState(index).selected;
  }
  if (selection_policy_.mode == SelectionMode::kNone) return false;
  if (selection_policy_.mode == SelectionMode::kSingle) {
    return selection_.section == section.widget && selection_.index == index;
  }
  return section.dynamic ? static_cast<const DynamicListBase*>(section.widget)
                               ->model()
                               .rowState(index)
                               .selected
                         : section.selected;
}

ListEntryVisualContext List::rowContext(int section_index, int index) const {
  const Section& section = sections_[section_index];
  ListEntryVisualContext result;
  int logical = 0;
  if (section.dynamic) {
    const auto& dynamic = static_cast<const DynamicListBase&>(*section.widget);
    logical = dynamic.logical_start_ + index;
    result.enabled = dynamic.model().sectionState().enabled;
  } else {
    result = static_cast<const ListEntry*>(section.widget)->visualContext();
    return result;
  }
  result.density = density_;
  result.variant = variant_;
  result.style = style_;
  result.position = PositionForIndex(logical, logical_count_);
  result.selected = selected(section, index);
  result.divider_mode = divider_policy_.mode;
  result.divider_start_inset = divider_policy_.start_inset;
  result.divider_end_inset = divider_policy_.end_inset;
  bool next_selected = false;
  if (index + 1 < sectionCount(section)) {
    next_selected = selected(section, index + 1);
  } else {
    int next = section_index + 1;
    while (next < static_cast<int>(sections_.size()) &&
           sectionCount(sections_[next]) == 0) {
      ++next;
    }
    if (next < static_cast<int>(sections_.size())) {
      next_selected = selected(sections_[next], 0);
    }
  }
  result.show_divider = ShouldShowDivider(
      divider_policy_, logical, logical_count_, result.selected, next_selected);
  return result;
}

DividerInsetHint List::rowHint(int section, int index) const {
  if (sections_[section].dynamic) {
    return static_cast<const DynamicListBase*>(sections_[section].widget)
        ->model()
        .rowState(index)
        .divider_inset_hint;
  }
  const ListItem* item =
      static_cast<const ListEntry*>(sections_[section].widget)->item();
  return item == nullptr ? DividerInsetHint{} : item->dividerInsetHint();
}

Rect List::rowBounds(int section, int index) const {
  const Widget& widget = *sections_[section].widget;
  return sections_[section].dynamic
             ? static_cast<const DynamicListBase&>(widget)
                   .rowBounds(index)
                   .translate(widget.offsetLeft(), widget.offsetTop())
             : widget.parent_bounds();
}

int List::uniformGap() const {
  ListEntryVisualContext context;
  context.variant = variant_;
  context.style = style_;
  context.show_divider = divider_policy_.mode != DividerMode::kNone;
  return internal::ResolveGap(style_, divider_policy_.mode, context, context);
}

YDim List::interSectionGap(int previous, int next) const {
  if (sections_[previous].dynamic || sections_[next].dynamic) {
    return uniformGap();
  }
  return internal::ResolveGap(
      style_, divider_policy_.mode,
      static_cast<const ListEntry*>(sections_[previous].widget)
          ->visualContext(),
      static_cast<const ListEntry*>(sections_[next].widget)->visualContext());
}

void List::resolveContexts() {
  int64_t total = 0;
  for (int i = 0; i < static_cast<int>(sections_.size()); ++i) {
    Section& section = sections_[i];
    if (section.dynamic) {
      auto& dynamic = static_cast<DynamicListBase&>(*section.widget);
      CHECK(!dynamic.model().ownsSelection() ||
            selection_policy_.mode == SelectionMode::kNone);
      dynamic.logical_start_ = total;
      dynamic.section_index_ = i;
    }
    total += sectionCount(section);
    CHECK_LE(total, std::numeric_limits<int>::max());
  }
  logical_count_ = total;
  int logical = 0;
  for (int i = 0; i < static_cast<int>(sections_.size()); ++i) {
    Section& section = sections_[i];
    int count = sectionCount(section);
    if (section.dynamic) {
      static_cast<DynamicListBase*>(section.widget)->refreshContexts();
    } else {
      auto& row = static_cast<ListEntry&>(*section.widget);
      // Use the already known ordinal rather than rescanning static prefixes.
      ListEntryVisualContext context = row.visualContext();
      context.density = density_;
      context.variant = variant_;
      context.style = style_;
      context.position = count == 0 ? ListItemPosition::kSingle
                                    : PositionForIndex(logical, logical_count_);
      context.selected = selected(section, 0);
      context.focused = row.isFocused();
      context.divider_mode = divider_policy_.mode;
      context.divider_start_inset = divider_policy_.start_inset;
      context.divider_end_inset = divider_policy_.end_inset;
      int next = i + 1;
      while (count != 0 && next < static_cast<int>(sections_.size()) &&
             sectionCount(sections_[next]) == 0) {
        ++next;
      }
      bool next_selected = count != 0 &&
                           next < static_cast<int>(sections_.size()) &&
                           selected(sections_[next], 0);
      context.show_divider =
          count != 0 &&
          ShouldShowDivider(divider_policy_, logical, logical_count_,
                            context.selected, next_selected);
      if (row.visualContext().show_divider != context.show_divider) {
        invalidateDividerAfter(i, 0);
      }
      row.setVisualContext(context);
      if (row.item() != nullptr && row.item()->selectionControl() != nullptr) {
        if (selection_policy_.mode != SelectionMode::kNone &&
            row.item()->selectionParticipation() ==
                SelectionParticipation::kSelectable) {
          row.item()->applySelection(context.selected
                                         ? SelectionState::kSelected
                                         : SelectionState::kDeselected);
          row.item()->setSelectionHandler([this, &row]() { invokeChild(row); });
        } else {
          row.item()->setSelectionHandler({});
        }
      }
    }
    logical += count;
  }
}

void List::onStructureOrPolicyChanged() {
  resolveContexts();
  invalidateInterior();
  requestLayout();
}

void List::addSection(WidgetRef ref, bool dynamic) {
  checkNotCleaningBindings();
  CHECK(!clearing_);
  Widget* widget = ref.get();
  CHECK(widget != nullptr);
  CHECK(widget->parent() == nullptr);
  bool initial =
      !dynamic && static_cast<ListEntry*>(widget)->visualContext().selected;
  sections_.push_back({widget, dynamic, initial});
  if (dynamic) static_cast<DynamicListBase*>(widget)->owner_ = this;
  attachChild(std::move(ref));
  onStructureOrPolicyChanged();
}

void List::add(ListEntry& entry) { addSection(WidgetRef(entry), false); }
void List::add(std::unique_ptr<ListEntry> entry) {
  addSection(WidgetRef(std::move(entry)), false);
}
void List::add(DynamicListBase& section) {
  addSection(WidgetRef(section), true);
}
void List::add(std::unique_ptr<DynamicListBase> section) {
  addSection(WidgetRef(std::move(section)), true);
}

void List::clear() {
  checkNotCleaningBindings();
  if (clearing_) return;
  clearing_ = true;
  invalidateInvocations(nullptr);
  clearSelection();
  while (!sections_.empty()) {
    Section section = sections_.back();
    sections_.pop_back();
    if (section.dynamic) {
      static_cast<DynamicListBase*>(section.widget)->owner_ = nullptr;
    }
    if (!section.dynamic) {
      ListItem* item = static_cast<ListEntry*>(section.widget)->item();
      if (item != nullptr) item->setSelectionHandler({});
    }
    detachChild(section.widget);
  }
  logical_count_ = 0;
  invalidateInterior();
  requestLayout();
  clearing_ = false;
}

bool List::replaceSelection(ListRowLocation location) {
  if (selection_.section == location.section &&
      selection_.index == location.index) {
    return true;
  }
  // A nested transition supersedes outstanding model notifications.
  for (Invocation* call = invocation_; call != nullptr; call = call->previous) {
    if (call->row == nullptr) call->valid = false;
  }
  ListRowLocation previous_selection = selection_;
  selection_ = location;
  resolveContexts();
  int previous = -1;
  for (int i = 0; i < static_cast<int>(sections_.size()); ++i) {
    if (sectionCount(sections_[i]) == 0) continue;
    if (previous >= 0 && !sections_[previous].dynamic &&
        !sections_[i].dynamic) {
      Widget* before = sections_[previous].widget;
      Widget* after = sections_[i].widget;
      if (previous_selection.section == before ||
          previous_selection.section == after || location.section == before ||
          location.section == after) {
        requestLayout();
      }
    }
    previous = i;
  }
  if (destroying_) return true;
  Invocation new_call(*this, nullptr, location.section);
  int old_section = findSection(previous_selection.section);
  if (old_section >= 0 && sections_[old_section].dynamic) {
    static_cast<DynamicListBase*>(previous_selection.section)
        ->model()
        .setSelected(previous_selection.index, SelectionState::kDeselected);
  }
  if (!new_call.valid || new_call.owner == nullptr) return true;
  int new_section = findSection(location.section);
  if (new_section >= 0 && sections_[new_section].dynamic) {
    static_cast<DynamicListBase*>(location.section)
        ->model()
        .setSelected(location.index, SelectionState::kSelected);
  }
  if (!new_call.valid || new_call.owner == nullptr) return true;
  onSingleSelectionChanged(location);
  return true;
}

bool List::select(ListEntry& entry) {
  checkNotCleaningBindings();
  if (clearing_ || selection_policy_.mode != SelectionMode::kSingle ||
      findSection(&entry) < 0 || entry.isGone() ||
      (entry.item() != nullptr && entry.item()->selectionParticipation() ==
                                      SelectionParticipation::kAction)) {
    return false;
  }
  return replaceSelection({&entry, 0});
}

bool List::select(DynamicListBase& section, int index) {
  checkNotCleaningBindings();
  if (clearing_ || selection_policy_.mode != SelectionMode::kSingle ||
      section.owner_ != this || section.resetting() || section.isGone() ||
      index < 0 || index >= section.elementCount() ||
      section.model().rowState(index).participation ==
          SelectionParticipation::kAction) {
    return false;
  }
  return replaceSelection({&section, index});
}

void List::clearSelection() {
  checkNotCleaningBindings();
  if (selection_.section != nullptr) replaceSelection({});
}

bool List::setSelected(ListEntry& entry, bool selected) {
  checkNotCleaningBindings();
  int index = findSection(&entry);
  if (clearing_ || index < 0 ||
      selection_policy_.mode != SelectionMode::kMultiple) {
    return false;
  }
  if (selected && entry.item() != nullptr &&
      entry.item()->selectionParticipation() ==
          SelectionParticipation::kAction) {
    return false;
  }
  if (sections_[index].selected == selected) return true;
  sections_[index].selected = selected;
  onStructureOrPolicyChanged();
  return true;
}

bool List::setSelected(DynamicListBase& section, int index,
                       SelectionState state) {
  checkNotCleaningBindings();
  if (clearing_ || selection_policy_.mode != SelectionMode::kMultiple ||
      section.owner_ != this || section.resetting() || section.isGone() ||
      index < 0 || index >= section.elementCount()) {
    return false;
  }
  DynamicListRowState current = section.model().rowState(index);
  if (current.participation == SelectionParticipation::kAction) return false;
  if (current.selected == (state == SelectionState::kSelected)) return true;
  Invocation call(*this, nullptr, &section);
  section.model().setSelected(index, state);
  if (call.valid && call.owner != nullptr) resolveContexts();
  return true;
}

void List::setVariant(ListVariant variant) {
  checkNotCleaningBindings();
  if (variant_ == variant) return;
  variant_ = variant;
  onStructureOrPolicyChanged();
}

void List::setStyle(ListStyle style) {
  checkNotCleaningBindings();
  if (style_ == style) return;
  style_ = style;
  onStructureOrPolicyChanged();
}

void List::setDensity(Density density) {
  setDensityOverride(DensityOverride::Explicit(density));
}

void List::clearDensityOverride() { setDensityOverride(DensityOverride{}); }

void List::setDensityOverride(DensityOverride density) {
  checkNotCleaningBindings();
  if (density_ == density) return;
  density_ = density;
  resolveContexts();
  requestLayoutDescending();
  invalidateDescending();
}

void List::setSelectionPolicy(const ListSelectionPolicy& policy) {
  checkNotCleaningBindings();
  bool clear =
      selection_policy_.mode != policy.mode && selection_.section != nullptr;
  selection_policy_ = policy;
  if (clear) {
    replaceSelection({});  // Terminal: callbacks may destroy this list.
    return;
  }
  onStructureOrPolicyChanged();
}

void List::setDividerPolicy(const ListDividerPolicy& policy) {
  checkNotCleaningBindings();
  divider_policy_ = policy;
  onStructureOrPolicyChanged();
}

void List::sectionChanged(DynamicListBase& section, bool layout) {
  resolveContexts();
  invalidateInterior();
  if (layout) requestLayout();
  if (selection_.section == &section &&
      (section.resetting() || section.isGone() ||
       selection_.index >= section.elementCount())) {
    clearSelection();  // Callback is terminal: it may destroy this
                       // list/section.
  }
}

bool List::fillTouchTargetPath(XDim x, YDim y, std::vector<Widget*>& path) {
  if (!Widget::fillTouchTargetPath(x, y, path)) return false;
  for (const Section& section : sections_) {
    Widget& child = *section.widget;
    if (sectionCount(section) == 0 || !child.parent_bounds().contains(x, y)) {
      continue;
    }
    child.fillTouchTargetPath(x - child.offsetLeft(), y - child.offsetTop(),
                              path);
    break;
  }
  return true;
}

bool List::fillSloppyTouchTargetPath(XDim x, YDim y,
                                     std::vector<Widget*>& path) {
  return fillTouchTargetPath(x, y, path);
}

bool List::invokeChild(Widget& child) {
  int index = findSection(&child);
  if (index < 0 || sections_[index].dynamic) return false;
  return invokeRow({&child, 0}, static_cast<ListEntry&>(child));
}

List::Invocation::Invocation(List& list, ListEntry* entry, Widget* section)
    : owner(&list), row(entry), section(section), previous(list.invocation_) {
  list.invocation_ = this;
}

List::Invocation::~Invocation() {
  if (owner != nullptr) owner->invocation_ = previous;
}

bool List::invokeRow(ListRowLocation location, ListEntry& row) {
  if (!row.isClickable() || !row.isEnabled()) return true;
  Invocation call(*this, &row, location.section);
  bool selectable =
      row.item() == nullptr || row.item()->selectionParticipation() ==
                                   SelectionParticipation::kSelectable;
  bool managed = selection_policy_.mode != SelectionMode::kNone;
  int section_index = findSection(location.section);
  if (section_index >= 0 && sections_[section_index].dynamic) {
    auto& model = static_cast<DynamicListBase*>(location.section)->model();
    selectable = selectable && model.rowState(location.index).participation ==
                                   SelectionParticipation::kSelectable;
    managed = managed || model.ownsSelection();
    if (selectable && model.ownsSelection()) {
      model.setSelected(location.index, SelectionState::kSelected);
      // Model notification may reset/clear the section or destroy the list.
      if (call.row == nullptr) return true;
    }
  }
  if (selectable && selection_policy_.mode == SelectionMode::kSingle &&
      selection_policy_.selection_follows_press) {
    replaceSelection(location);
  } else if (selectable && selection_policy_.mode == SelectionMode::kMultiple &&
             selection_policy_.selection_follows_press && section_index >= 0) {
    if (sections_[section_index].dynamic) {
      auto& section = *static_cast<DynamicListBase*>(location.section);
      setSelected(section, location.index,
                  section.model().rowState(location.index).selected
                      ? SelectionState::kDeselected
                      : SelectionState::kSelected);
    } else {
      setSelected(row, !sections_[section_index].selected);
    }
  }
  if (call.row != nullptr && selectable && managed) {
    // A control can change itself before routing here, even with follows-press
    // disabled. Always restore authoritative state before its action runs.
    resolveContexts();
  }
  if (call.row != nullptr && call.row->item() != nullptr) {
    ListItem* item = call.row->item();
    if (selectable && managed && item->selectionControl() != nullptr) {
      item->invokeSelection();
    } else {
      item->invoke();
    }
  }
  if (call.row != nullptr) call.row->Widget::onClicked();
  return true;
}

int List::getChildrenCount() const { return sections_.size(); }
const Widget& List::getChild(int index) const {
  return *sections_[index].widget;
}
Widget& List::getChild(int index) { return *sections_[index].widget; }

// Dividers occupy container-owned gaps outside row bounds, so a row repaint
// alone cannot hide or restore them when adjacent-selection suppression
// changes.
void List::invalidateDividerAfter(int section, int index) {
  const Section& current = sections_[section];
  if (current.dynamic && index + 1 < sectionCount(current)) {
    auto& dynamic = static_cast<DynamicListBase&>(*current.widget);
    YDim top = dynamic.rowBounds(index).yMax() + 1;
    YDim bottom = dynamic.rowBounds(index + 1).yMin() - 1;
    if (top <= bottom) {
      dynamic.invalidateInterior(Rect(0, top, dynamic.width() - 1, bottom));
    }
    return;
  }
  int next = section + 1;
  while (next < static_cast<int>(sections_.size()) &&
         sectionCount(sections_[next]) == 0) {
    ++next;
  }
  if (next == static_cast<int>(sections_.size())) return;
  Rect row = rowBounds(section, index);
  YDim top = row.yMax() + 1;
  YDim bottom = rowBounds(next, 0).yMin() - 1;
  if (top <= bottom) {
    invalidateInterior(Rect(row.xMin(), top, row.xMax(), bottom));
  }
}

void List::paintBand(PaintContext& context, int section, int index,
                     YDim gap) const {
  if (gap <= 0) return;
  Rect bounds = rowBounds(section, index);
  YDim y = bounds.yMax() + 1 + (gap - DividerThicknessPx()) / 2;
  ListEntryVisualContext visual =
      sections_[section].dynamic
          ? rowContext(section, index)
          : static_cast<const ListEntry*>(sections_[section].widget)
                ->visualContext();
  DividerMetrics divider = ResolveDividerMetrics(
      visual, rowHint(section, index), bounds.width(), bounds.xMin(), y);
  if (!divider.visible) return;
  Rect band(divider.start_x, y, divider.end_x, y + DividerThicknessPx() - 1);
  context.fillRect(band, theme().material3Theme().color.outlineVariant);
  context.addExclusion(band);
}

void List::paint(PaintContext& context) const {
  int previous = -1;
  for (int i = 0; i < static_cast<int>(sections_.size()); ++i) {
    if (sectionCount(sections_[i]) == 0) continue;
    if (previous >= 0) {
      paintBand(context, previous, sectionCount(sections_[previous]) - 1,
                interSectionGap(previous, i));
    }
    previous = i;
  }
  Container::paint(context);
}

Dimensions List::onMeasure(WidthSpec width, HeightSpec height) {
  resolveContexts();
  XDim resolved_width = width.kind() == EXACTLY ? width.value() : 0;
  if (width.kind() != EXACTLY) {
    for (Section& section : sections_) {
      if (sectionCount(section) == 0) continue;
      Dimensions measured = MeasureChildWithMargins(*section.widget, width,
                                                    HeightSpec::Unspecified(0));
      resolved_width = std::max(resolved_width, measured.width());
    }
    resolved_width = width.resolveSize(resolved_width);
  }
  int64_t total = 0;
  int previous = -1;
  for (int i = 0; i < static_cast<int>(sections_.size()); ++i) {
    if (sectionCount(sections_[i]) == 0) continue;
    if (previous >= 0) total += interSectionGap(previous, i);
    total += MeasureChildWithMargins(*sections_[i].widget,
                                     WidthSpec::Exactly(resolved_width),
                                     HeightSpec::Unspecified(0))
                 .height();
    CHECK_LE(total, Rect::MaximumRect().yMax());
    previous = i;
  }
  Dimensions measured(resolved_width, height.resolveSize(total));
  if (selection_.section != nullptr && selection_.section->isGone()) {
    clearSelection();
  }
  return measured;
}

void List::onLayout(bool changed, const Rect& rect) {
  resolveContexts();
  YDim y = 0;
  int previous = -1;
  for (int i = 0; i < static_cast<int>(sections_.size()); ++i) {
    Widget& widget = *sections_[i].widget;
    if (sectionCount(sections_[i]) == 0) {
      if (!widget.isGone()) {
        MeasureChildWithMargins(widget, WidthSpec::Exactly(rect.width()),
                                HeightSpec::Exactly(0));
        LayoutChildWithMargins(widget, Rect(0, y, rect.width() - 1, y - 1));
      }
      continue;
    }
    if (previous >= 0) y += interSectionGap(previous, i);
    Dimensions measured = MeasureChildWithMargins(
        widget, WidthSpec::Exactly(rect.width()), HeightSpec::Unspecified(0));
    CHECK_LE(static_cast<int64_t>(y) + measured.height(),
             Rect::MaximumRect().yMax());
    LayoutChildWithMargins(
        widget, Rect(0, y, rect.width() - 1, y + measured.height() - 1));
    y += measured.height();
    previous = i;
  }
}

namespace {
// Finds a real eligible target; descendants are visited only within one row.
Widget* RowFocusTarget(Widget& row, bool backwards, bool include_row) {
  if (!row.isVisible() || !row.isEnabled()) return nullptr;
  if (!backwards && include_row && row.isFocusable() && !row.bounds().empty()) {
    return &row;
  }
  for (int n = 0; n < row.focusChildCount(); ++n) {
    int index = backwards ? row.focusChildCount() - n - 1 : n;
    Widget* child = row.focusChildAt(index);
    if (child != nullptr) {
      Widget* target = RowFocusTarget(*child, backwards, true);
      if (target != nullptr) return target;
    }
  }
  return backwards && include_row && row.isFocusable() && !row.bounds().empty()
             ? &row
             : nullptr;
}
}  // namespace

bool List::onKeyEvent(const KeyEvent& event) {
  if ((event.phase != KeyPhase::kDown && event.phase != KeyPhase::kRepeat) ||
      (event.code != KeyCode::kUp && event.code != KeyCode::kDown)) {
    return false;
  }
  for (Widget* ancestor = this; ancestor != nullptr;
       ancestor = ancestor->parent()) {
    if (!ancestor->isVisible() || !ancestor->isEnabled()) return false;
  }
  Widget* focused = focusManager().focused();
  if (focused == nullptr) return false;
  Widget* direct = focused;
  while (direct->parent() != nullptr && direct->parent() != this) {
    direct = direct->parent();
  }
  int section = findSection(direct);
  if (section < 0) return false;
  bool backwards = event.code == KeyCode::kUp;
  int step = backwards ? -1 : 1;
  int index = 0;
  if (sections_[section].dynamic) {
    Widget* row = focused;
    while (row->parent() != direct) row = row->parent();
    index = static_cast<DynamicListBase*>(direct)->indexOf(*row);
  }
  index += step;
  while (section >= 0 && section < static_cast<int>(sections_.size())) {
    Section record = sections_[section];
    int count = sectionCount(record);
    if (index >= 0 && index < count && record.widget->isVisible() &&
        record.widget->isEnabled()) {
      if (!record.dynamic) {
        Widget* target = RowFocusTarget(*record.widget, backwards, true);
        if (target != nullptr) return focusManager().requestFocus(*target);
      } else {
        auto& dynamic = static_cast<DynamicListBase&>(*record.widget);
        DynamicListSectionState state = dynamic.model().sectionState();
        if (state.enabled &&
            state.focus_target != DynamicListFocusTarget::kNone) {
          ListEntry* row = dynamic.focusRow(index);
          Widget* target =
              state.focus_target == DynamicListFocusTarget::kRowSurface
                  ? row
                  : RowFocusTarget(*row, backwards, false);
          CHECK(target != nullptr);
          return focusManager().requestFocus(*target);
        }
      }
    }
    section += step;
    if (section >= 0 && section < static_cast<int>(sections_.size())) {
      index = backwards ? sectionCount(sections_[section]) - 1 : 0;
    }
  }
  return false;
}

}  // namespace material3
}  // namespace roo_windows
