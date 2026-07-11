#include "roo_windows/material3/list/list.h"

#include <algorithm>
#include <new>

#include "roo_display/shape/smooth.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_icons/filled/24/navigation.h"
#include "roo_logging.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/widgets/text_block.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows {
namespace material3 {

namespace {

constexpr int16_t kExpressiveHorizontalPaddingDp = 16;
constexpr int16_t kExpressiveVerticalPaddingDp = 10;
constexpr int16_t kExpressiveSlotGapDp = 12;
constexpr int16_t kBaselineHorizontalPaddingDp = 16;
constexpr int16_t kBaselineVerticalPaddingDp = 8;
constexpr int16_t kBaselineSlotGapDp = 16;
constexpr int16_t kBodyGapDp = 8;
constexpr int16_t kOneLineMinHeightDp = 56;
constexpr int16_t kTwoLineMinHeightDp = 72;
constexpr int16_t kThreeLineMinHeightDp = 88;
constexpr int16_t kSegmentedListGapDp = 8;
constexpr int16_t kExpressiveOuterCornerRadiusDp = 16;
constexpr int16_t kExpressiveInnerCornerRadiusDp = 4;
constexpr int16_t kExpressiveStandardSeparatorDp = 2;
constexpr int16_t kDividerThicknessDp = 1;
constexpr int16_t kAvatarSizeDp = 40;
constexpr uint16_t kExpandableAnimationFrameMillis = 20;

struct RowTokens {
  int16_t horizontal_padding;
  int16_t vertical_padding;
  int16_t slot_gap;
  int16_t body_gap;
};

struct TextSlotMetrics {
  int16_t width;
  int16_t height;
  uint8_t line_count;
};

struct RowLayoutMetrics {
  Dimensions leading;
  Dimensions trailing;
  Dimensions body;
  TextSlotMetrics text;
  int16_t width;
  int16_t height;
  int16_t main_height;
  int16_t row_band_height;
  int16_t text_x;
  int16_t text_y;
  int16_t text_width;
  int16_t leading_x;
  int16_t leading_y;
  int16_t trailing_x;
  int16_t trailing_y;
  int16_t body_x;
  int16_t body_y;
  int16_t body_width;
};

struct DividerMetrics {
  int16_t start_x;
  int16_t end_x;
  int16_t y;
  bool visible;
};

RowTokens TokensFor(ListVariant variant) {
  if (variant == ListVariant::kBaseline) {
    return RowTokens{Scaled(kBaselineHorizontalPaddingDp),
                     Scaled(kBaselineVerticalPaddingDp),
                     Scaled(kBaselineSlotGapDp), Scaled(kBodyGapDp)};
  }
  return RowTokens{Scaled(kExpressiveHorizontalPaddingDp),
                   Scaled(kExpressiveVerticalPaddingDp),
                   Scaled(kExpressiveSlotGapDp), Scaled(kBodyGapDp)};
}

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

const roo_display::Font& FontForOverline() { return font_overline(); }

const roo_display::Font& FontForHeadline() { return font_body1(); }

const roo_display::Font& FontForSupporting() { return font_body2(); }

int16_t TextLineHeight(const roo_display::Font& font) {
  return font.metrics().maxHeight();
}

int16_t TextWidth(const roo_display::Font& font, roo::string_view text) {
  if (text.empty()) return 0;
  return font.getHorizontalStringMetrics(text).advance();
}

uint8_t SlotLineCount(roo::string_view text, ListTextPolicy policy) {
  if (text.empty()) return 0;
  return std::max<uint8_t>(1, policy.max_lines);
}

// Converts descriptor text to an owning std::string for TextBlock.
std::string ToString(roo::string_view text) {
  return std::string(text.data(), text.size());
}

// Phase 7 policy split: single-line truncate stays on the cheap label path;
// any wrap or multi-line mode opts into the heavier block-text widget.
bool UsesBlockSlot(ListTextPolicy policy) {
  return policy.max_lines > 1 || policy.overflow == TextOverflowPolicy::kWrap;
}

// Measures the descriptor-driven text stack without depending on any row-owned
// child widget state.
TextSlotMetrics MeasureTextSlots(const ListItem* item) {
  if (item == nullptr) return TextSlotMetrics{0, 0, 0};

  uint8_t overline_lines =
      SlotLineCount(item->overlineText(), item->overlinePolicy());
  uint8_t headline_lines =
      SlotLineCount(item->headlineText(), item->headlinePolicy());
  uint8_t supporting_lines =
      SlotLineCount(item->supportingText(), item->supportingPolicy());

  int16_t width = 0;
  width = std::max(width, TextWidth(FontForOverline(), item->overlineText()));
  width = std::max(width, TextWidth(FontForHeadline(), item->headlineText()));
  width =
      std::max(width, TextWidth(FontForSupporting(), item->supportingText()));

  int16_t height = overline_lines * TextLineHeight(FontForOverline()) +
                   headline_lines * TextLineHeight(FontForHeadline()) +
                   supporting_lines * TextLineHeight(FontForSupporting());
  return TextSlotMetrics{
      width, height,
      static_cast<uint8_t>(overline_lines + headline_lines + supporting_lines)};
}

Dimensions MeasureChild(Widget* child, WidthSpec width, HeightSpec height) {
  if (child == nullptr || child->isGone()) return Dimensions(0, 0);
  return child->measure(width, height);
}

Dimensions SuggestedMinimumChild(const Widget* child) {
  if (child == nullptr || child->isGone()) return Dimensions(0, 0);
  return child->getSuggestedMinimumDimensions();
}

int16_t ConstrainWidth(int16_t desired, WidthSpec spec) {
  switch (spec.kind()) {
    case UNSPECIFIED:
      return desired;
    case AT_MOST:
      return std::min<int16_t>(desired, spec.value());
    case EXACTLY:
      return spec.value();
  }
  return desired;
}

int16_t ConstrainHeight(int16_t desired, HeightSpec spec) {
  switch (spec.kind()) {
    case UNSPECIFIED:
      return desired;
    case AT_MOST:
      return std::min<int16_t>(desired, spec.value());
    case EXACTLY:
      return spec.value();
  }
  return desired;
}

int16_t MinRowBandHeight(const ListItem* item, const TextSlotMetrics& text) {
  if (item != nullptr && item->preferTopTextAlignment()) {
    return Scaled(kThreeLineMinHeightDp);
  }
  if (text.line_count >= 3) return Scaled(kThreeLineMinHeightDp);
  if (text.line_count == 2) return Scaled(kTwoLineMinHeightDp);
  return Scaled(kOneLineMinHeightDp);
}

int16_t MiddleOffset(int16_t outer, int16_t inner) {
  if (outer <= inner) return 0;
  return (outer - inner) / 2;
}

int16_t SlotY(const ListItem* item, VerticalVisualAlignment alignment,
              int16_t row_band_height, int16_t slot_height,
              const RowTokens& tokens) {
  if (alignment == VerticalVisualAlignment::kTop ||
      (item != nullptr && item->preferTopTextAlignment())) {
    return tokens.vertical_padding;
  }
  return MiddleOffset(row_band_height, slot_height);
}

// Resolves one shared row geometry so measurement and child layout agree on
// the same slot positions and text bounds.
RowLayoutMetrics ResolveRowLayout(ListEntry& entry, WidthSpec width_spec,
                                  HeightSpec height_spec) {
  ListItem* item = entry.item();
  const RowTokens tokens = TokensFor(entry.visualContext().variant);

  Dimensions leading =
      MeasureChild(item == nullptr ? nullptr : item->leading(),
                   WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  Dimensions trailing =
      MeasureChild(item == nullptr ? nullptr : item->trailing(),
                   WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  TextSlotMetrics text = MeasureTextSlots(item);

  bool has_leading = leading.width() > 0 || leading.height() > 0;
  bool has_trailing = trailing.width() > 0 || trailing.height() > 0;
  bool has_text = text.width > 0 || text.height > 0;

  int16_t horizontal_gaps = 0;
  if (has_leading && has_text) horizontal_gaps += tokens.slot_gap;
  if (has_trailing && (has_text || has_leading))
    horizontal_gaps += tokens.slot_gap;

  int16_t desired_main_width = tokens.horizontal_padding * 2 + leading.width() +
                               trailing.width() + text.width + horizontal_gaps;

  Dimensions body =
      MeasureChild(item == nullptr ? nullptr : item->body(),
                   WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  bool has_body = body.width() > 0 || body.height() > 0;
  int16_t desired_body_width =
      has_body ? tokens.horizontal_padding * 2 +
                     std::max<int16_t>(body.width(), text.width)
               : 0;
  int16_t desired_width = std::max(desired_main_width, desired_body_width);
  int16_t resolved_width = ConstrainWidth(desired_width, width_spec);

  // The text column expands into whatever horizontal space remains between the
  // fixed leading and trailing slots.
  int16_t content_width =
      std::max<int16_t>(0, resolved_width - 2 * tokens.horizontal_padding);
  int16_t text_x = tokens.horizontal_padding;
  if (has_leading) text_x += leading.width() + (has_text ? tokens.slot_gap : 0);
  int16_t trailing_x =
      resolved_width - tokens.horizontal_padding - trailing.width();
  int16_t text_right = has_trailing
                           ? trailing_x - tokens.slot_gap - 1
                           : resolved_width - tokens.horizontal_padding - 1;
  int16_t text_width =
      has_text ? std::max<int16_t>(0, text_right - text_x + 1) : 0;

  int16_t main_content_height =
      std::max<int16_t>(text.height, leading.height());
  main_content_height =
      std::max<int16_t>(main_content_height, trailing.height());
  int16_t row_band_height =
      std::max<int16_t>(MinRowBandHeight(item, text),
                        main_content_height + 2 * tokens.vertical_padding);
  int16_t body_y = row_band_height;
  if (has_body) body_y += tokens.body_gap;
  int16_t desired_height =
      row_band_height +
      (has_body ? tokens.body_gap + body.height() + tokens.vertical_padding
                : 0);
  int16_t resolved_height = ConstrainHeight(desired_height, height_spec);

  int16_t text_y = tokens.vertical_padding;
  if (item == nullptr || !item->preferTopTextAlignment()) {
    text_y = MiddleOffset(row_band_height, text.height);
  }
  int16_t leading_y = item == nullptr
                          ? MiddleOffset(row_band_height, leading.height())
                          : SlotY(item, item->leadingAlignment(),
                                  row_band_height, leading.height(), tokens);
  int16_t trailing_y = item == nullptr
                           ? MiddleOffset(row_band_height, trailing.height())
                           : SlotY(item, item->trailingAlignment(),
                                   row_band_height, trailing.height(), tokens);

  int16_t body_width = has_body ? std::max<int16_t>(0, content_width) : 0;
  if (has_body && body_width != body.width()) {
    // Re-measure the optional body at the resolved content width so stacked
    // body content and row height stay consistent with the final row width.
    body = MeasureChild(item->body(), WidthSpec::Exactly(body_width),
                        HeightSpec::Unspecified(0));
    body_y = row_band_height + tokens.body_gap;
    desired_height = row_band_height + tokens.body_gap + body.height() +
                     tokens.vertical_padding;
    resolved_height = ConstrainHeight(desired_height, height_spec);
  }

  return RowLayoutMetrics{leading,
                          trailing,
                          body,
                          text,
                          resolved_width,
                          resolved_height,
                          main_content_height,
                          row_band_height,
                          text_x,
                          text_y,
                          text_width,
                          tokens.horizontal_padding,
                          leading_y,
                          trailing_x,
                          trailing_y,
                          tokens.horizontal_padding,
                          body_y,
                          body_width};
}

DividerMetrics ResolveDividerMetrics(const ListEntry& entry, int16_t x_offset,
                                     int16_t y) {
  const ListEntryVisualContext& context = entry.visualContext();
  if (!context.show_divider || context.divider_mode == DividerMode::kNone ||
      entry.width() <= 0) {
    return DividerMetrics{0, -1, 0, false};
  }

  int16_t start_inset = 0;
  int16_t end_inset = 0;
  if (context.divider_mode == DividerMode::kInset) {
    start_inset = context.divider_start_inset;
    end_inset = context.divider_end_inset;
    const ListItem* item = entry.item();
    if (item != nullptr) {
      DividerInsetHint hint = item->dividerInsetHint();
      start_inset = std::max<int16_t>(start_inset, hint.start_inset);
      end_inset = std::max<int16_t>(end_inset, hint.end_inset);
    }
  }

  int16_t start_x = x_offset + start_inset;
  int16_t end_x = x_offset + entry.width() - 1 - end_inset;
  if (start_x > end_x) return DividerMetrics{0, -1, y, false};
  return DividerMetrics{start_x, end_x, y, true};
}

ListItemPosition PositionForIndex(int idx, int count) {
  if (count <= 1) return ListItemPosition::kSingle;
  if (idx == 0) return ListItemPosition::kFirst;
  if (idx == count - 1) return ListItemPosition::kLast;
  return ListItemPosition::kMiddle;
}

int FirstSelectedIndex(const std::vector<uint8_t>& selected_entries) {
  for (int i = 0; i < static_cast<int>(selected_entries.size()); ++i) {
    if (selected_entries[i] != 0) return i;
  }
  return -1;
}

bool ResolvedSelectedState(const std::vector<uint8_t>& selected_entries,
                           const ListSelectionPolicy& selection_policy, int idx,
                           int first_selected_idx) {
  if (selection_policy.mode == SelectionMode::kNone) return false;
  if (idx < 0 || idx >= static_cast<int>(selected_entries.size())) return false;
  if (selected_entries[idx] == 0) return false;
  if (selection_policy.mode == SelectionMode::kSingle) {
    return idx == first_selected_idx;
  }
  return true;
}

bool ShouldShowDivider(const ListDividerPolicy& divider_policy, int idx,
                       int count, bool selected, bool next_selected) {
  if (divider_policy.mode == DividerMode::kNone || idx >= count - 1) {
    return false;
  }
  if (divider_policy.suppress_between_selected && selected && next_selected) {
    return false;
  }
  return true;
}

}  // namespace

// Keeps the avatar-specific paint logic private to the convenience item layer
// instead of introducing a broader public widget before the API needs one.
class AvatarVisual : public BasicWidget {
 public:
  AvatarVisual(ApplicationContext& context, roo::string_view initials)
      : BasicWidget(context), initials_(initials) {}

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
        0.5f * static_cast<float>(diameter), theme().color.primaryContainer);

    if (!initials_.empty()) {
      roo_display::ClippedStringViewLabel initials_label(
          initials_, FontForHeadline(), theme().color.onPrimaryContainer);
      roo_display::Offset text_offset =
          (roo_display::kCenter | roo_display::kMiddle)
              .resolveOffset(roo_display::Box(0, 0, diameter - 1, diameter - 1),
                             initials_label.anchorExtents());
      Rect text_bounds = Rect(initials_label.extents())
                             .translate(text_offset.dx, text_offset.dy)
                             .translate(rect.xMin(), rect.yMin());
      ctx.setBgcolor(theme().color.primaryContainer);
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
      animation_duration_millis_(0),
      animation_progress_millis_(0),
      expanded_(false) {}

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
  if (expanded_ == expanded && !isAnimating()) return;

  expanded_ = expanded;
  if (!animate || animation_duration_millis_ == 0) {
    animation_progress_millis_ = expanded_ ? animation_duration_millis_ : 0;
  }

  requestLayout();
  invalidateInterior();
}

bool ExpandablePanel::isExpanded() const { return expanded_; }

bool ExpandablePanel::isAnimating() const {
  if (animation_duration_millis_ == 0) return false;
  if (expanded_) {
    return animation_progress_millis_ < animation_duration_millis_;
  }
  return animation_progress_millis_ > 0;
}

void ExpandablePanel::setAnimationDuration(uint16_t millis) {
  uint16_t old_duration = animation_duration_millis_;
  animation_duration_millis_ = millis;
  if (animation_duration_millis_ == 0) {
    animation_progress_millis_ = 0;
  } else if (old_duration == 0) {
    animation_progress_millis_ = expanded_ ? animation_duration_millis_ : 0;
  } else if (animation_progress_millis_ > old_duration) {
    animation_progress_millis_ = old_duration;
  }
  requestLayout();
  invalidateInterior();
}

Dimensions ExpandablePanel::getSuggestedMinimumDimensions() const {
  const Widget* content = content_.get();
  if (content == nullptr || content->isGone()) {
    return Dimensions(0, 0);
  }
  Dimensions suggested = content->getSuggestedMinimumDimensions();
  return Dimensions(suggested.width(),
                    resolveVisibleHeight(suggested.height()));
}

void ExpandablePanel::paintWidgetContents(PaintContext& ctx) {
  Container::paintWidgetContents(ctx);
  if (isAnimating()) {
    // Keep relayout and repaint active while the clipped height is changing.
    requestLayout();
    invalidateInterior();
  }
}

void ExpandablePanel::stepAnimation() {
  if (!isAnimating()) return;

  uint16_t delta = std::min<uint16_t>(kExpandableAnimationFrameMillis,
                                      animation_duration_millis_);
  if (delta == 0) delta = 1;

  if (expanded_) {
    animation_progress_millis_ = std::min<uint16_t>(
        animation_duration_millis_, animation_progress_millis_ + delta);
  } else {
    animation_progress_millis_ =
        (animation_progress_millis_ > delta)
            ? static_cast<uint16_t>(animation_progress_millis_ - delta)
            : 0;
  }
}

int16_t ExpandablePanel::resolveVisibleHeight(int16_t full_height) const {
  if (full_height <= 0) return 0;
  if (animation_duration_millis_ == 0) {
    return expanded_ ? full_height : 0;
  }

  uint16_t clamped_progress = std::min<uint16_t>(animation_progress_millis_,
                                                 animation_duration_millis_);
  return static_cast<int16_t>(
      (static_cast<int32_t>(full_height) * clamped_progress) /
      animation_duration_millis_);
}

Dimensions ExpandablePanel::onMeasure(WidthSpec width, HeightSpec height) {
  if (isAnimating()) {
    stepAnimation();
  }

  Widget* content = content_.get();
  if (content == nullptr || content->isGone()) {
    return Dimensions(width.resolveSize(0), height.resolveSize(0));
  }

  Dimensions measured =
      content->measure(width, HeightSpec::Unspecified(height.value()));
  int16_t visible_height = resolveVisibleHeight(measured.height());
  return Dimensions(width.resolveSize(measured.width()),
                    height.resolveSize(visible_height));
}

void ExpandablePanel::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  Widget* content = content_.get();
  if (content == nullptr || content->isGone()) return;
  if (rect.width() <= 0 || rect.height() <= 0) {
    content->layout(Rect(0, 0, -1, -1));
    return;
  }

  // Keep child layout clipped to the panel's current visible height so child
  // max-bounds cannot spill into sibling paint/invalidation regions during
  // expand/collapse animation.
  content->layout(Rect(0, 0, rect.width() - 1, rect.height() - 1));
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
    : Container(context),
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
      suppress_next_click_invoke_(false),
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

void ListEntry::syncTextSlotsFromItem() {
  if (item_ == nullptr) {
    clearTextSlots();
    return;
  }

  // Keeps each slot in one of two stable widget classes and only allows class
  // transitions during bind/clear/rebind/refresh, never during paint.
  auto sync_slot = [this](Widget*& slot, TextSlotMode& mode,
                          roo::string_view text, ListTextPolicy policy,
                          const roo_display::Font& font,
                          roo_display::Color color) {
    if (text.empty()) {
      clearTextSlot(slot, mode);
      return;
    }

    TextSlotMode desired_mode =
        UsesBlockSlot(policy) ? TextSlotMode::kBlock : TextSlotMode::kLabel;
    if (slot == nullptr || mode != desired_mode) {
      clearTextSlot(slot, mode);
      if (desired_mode == TextSlotMode::kLabel) {
        auto owned = std::make_unique<StringViewLabel>(
            context(), text, font, color, kGravityLeft | kGravityMiddle);
        slot = owned.get();
        mode = TextSlotMode::kLabel;
        attachChild(WidgetRef(std::move(owned)));
        return;
      }
      auto owned =
          std::make_unique<TextBlock>(context(), ToString(text), font, color,
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
      label->setText(text);
      return;
    }
    TextBlock* block = static_cast<TextBlock*>(slot);
    block->setText(ToString(text));
    block->setColor(color);
    block->setWrapMode(policy.overflow == TextOverflowPolicy::kWrap
                           ? TextWrapMode::kWordWrap
                           : TextWrapMode::kNoWrap);
    block->setMaxLines(std::max<uint16_t>(1, policy.max_lines));
    block->setEllipsize(policy.overflow == TextOverflowPolicy::kTruncate);
  };

  const Theme& theme = context().theme();
  sync_slot(overline_text_, overline_mode_, item_->overlineText(),
            item_->overlinePolicy(), FontForOverline(),
            theme.color.onSurfaceVariant);
  sync_slot(headline_text_, headline_mode_, item_->headlineText(),
            item_->headlinePolicy(), FontForHeadline(), theme.color.onSurface);
  sync_slot(supporting_text_, supporting_mode_, item_->supportingText(),
            item_->supportingPolicy(), FontForSupporting(),
            theme.color.onSurfaceVariant);
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
  visual_context_ = context;
  invalidateInterior();
}

const ListEntryVisualContext& ListEntry::visualContext() const {
  return visual_context_;
}

ColorRole ListEntry::containerRole() const {
  if (visual_context_.selected &&
      visual_context_.variant == ListVariant::kExpressive) {
    return ColorRole::kSecondaryContainer;
  }
  return ColorRole::kSurface;
}

BorderStyle ListEntry::getBorderStyle() const {
  return BorderStyleFor(visual_context_);
}

Dimensions ListEntry::getSuggestedMinimumDimensions() const {
  const RowTokens tokens = TokensFor(visual_context_.variant);
  Dimensions leading = SuggestedMinimumChild(leading_child_);
  Dimensions trailing = SuggestedMinimumChild(trailing_child_);
  Dimensions body = SuggestedMinimumChild(body_child_);
  TextSlotMetrics text = MeasureTextSlots(item_);

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
  int16_t main_content_height =
      std::max<int16_t>(text.height, leading.height());
  main_content_height =
      std::max<int16_t>(main_content_height, trailing.height());
  int16_t row_band_height =
      std::max<int16_t>(MinRowBandHeight(item_, text),
                        main_content_height + 2 * tokens.vertical_padding);
  int16_t desired_height =
      row_band_height +
      (has_body ? tokens.body_gap + body.height() + tokens.vertical_padding
                : 0);

  return Dimensions(std::max(desired_main_width, desired_body_width),
                    desired_height);
}

bool ListEntry::isClickable() const {
  return item_ != nullptr && item_->isInvokable();
}

void ListEntry::onClicked() {
  if (item_ != nullptr && !suppress_next_click_invoke_) {
    item_->invoke();
  }
  suppress_next_click_invoke_ = false;
  Widget::onClicked();
}

bool ListEntry::onSingleTapUp(XDim x, YDim y) {
  bool handled = false;
  if (getMainWindow() == nullptr) {
    // Allows direct unit-test invocation without a live gesture detector.
    handled = true;
  } else {
    handled = Widget::onSingleTapUp(x, y);
  }

  if (item_ != nullptr && item_->isInvokable()) {
    item_->invoke();
    suppress_next_click_invoke_ = true;
  }
  return handled;
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
  RowLayoutMetrics layout = ResolveRowLayout(*this, width, height);
  return Dimensions(layout.width, layout.height);
}

void ListEntry::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  // Reuse the same geometry resolver used by measure() and paint() so child
  // slot placement stays consistent with the resolved text layout.
  RowLayoutMetrics layout =
      ResolveRowLayout(*this, WidthSpec::Exactly(rect.width()),
                       HeightSpec::Exactly(rect.height()));

  auto layout_text_slot = [](Widget* slot, int16_t x, int16_t y,
                             int16_t max_width) -> int16_t {
    if (slot == nullptr || slot->isGone()) return 0;
    if (max_width <= 0) {
      slot->layout(Rect(0, 0, -1, -1));
      return 0;
    }
    Dimensions measured =
        slot->measure(WidthSpec::AtMost(max_width), HeightSpec::Unspecified(0));
    int16_t slot_width = std::min<int16_t>(measured.width(), max_width);
    if (slot_width <= 0 || measured.height() <= 0) {
      slot->layout(Rect(0, 0, -1, -1));
      return 0;
    }
    slot->layout(Rect(x, y, x + slot_width - 1, y + measured.height() - 1));
    return measured.height();
  };

  int16_t text_y = layout.text_y;
  text_y += layout_text_slot(overline_text_, layout.text_x, text_y,
                             layout.text_width);
  text_y += layout_text_slot(headline_text_, layout.text_x, text_y,
                             layout.text_width);
  layout_text_slot(supporting_text_, layout.text_x, text_y, layout.text_width);

  if (leading_child_ != nullptr && !leading_child_->isGone()) {
    leading_child_->layout(
        Rect(layout.leading_x, layout.leading_y,
             layout.leading_x + layout.leading.width() - 1,
             layout.leading_y + layout.leading.height() - 1));
  }
  if (trailing_child_ != nullptr && !trailing_child_->isGone()) {
    trailing_child_->layout(
        Rect(layout.trailing_x, layout.trailing_y,
             layout.trailing_x + layout.trailing.width() - 1,
             layout.trailing_y + layout.trailing.height() - 1));
  }
  if (body_child_ != nullptr && !body_child_->isGone()) {
    body_child_->layout(Rect(layout.body_x, layout.body_y,
                             layout.body_x + layout.body.width() - 1,
                             layout.body_y + layout.body.height() - 1));
  }
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
      leading_icon_(context, pictogram),
      trailing_affordance_(context, ic_filled_24_navigation_chevron_right()) {}

Widget* NavigationListItem::leading() { return &leading_icon_; }

const Widget* NavigationListItem::leading() const { return &leading_icon_; }

Widget* NavigationListItem::trailing() { return &trailing_affordance_; }

const Widget* NavigationListItem::trailing() const {
  return &trailing_affordance_;
}

Icon& NavigationListItem::leadingIcon() { return leading_icon_; }

const Icon& NavigationListItem::leadingIcon() const { return leading_icon_; }

Icon& NavigationListItem::trailingAffordance() { return trailing_affordance_; }

const Icon& NavigationListItem::trailingAffordance() const {
  return trailing_affordance_;
}

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

List::List(ApplicationContext& context)
    : Container(context),
      entries_(),
      selected_entries_(),
      variant_(ListVariant::kExpressive),
      style_(ListStyle::kStandard),
      selection_policy_(),
      divider_policy_(),
      contexts_dirty_(false) {}

List::~List() { clear(); }

void List::onStructureOrPolicyChanged() {
  markEntryContextsDirty();
  refreshEntryVisualContexts();
}

void List::addEntryInternal(ListEntry* entry, WidgetRef ref) {
  selected_entries_.push_back(entry->visualContext().selected ? 1 : 0);
  entries_.push_back(entry);
  attachChild(std::move(ref));
  onStructureOrPolicyChanged();
}

void List::markEntryContextsDirty() {
  contexts_dirty_ = true;
  invalidateInterior();
  requestLayout();
}

int16_t List::interRowGap(int previous_idx, int next_idx) const {
  if (previous_idx < 0 || next_idx < 0) return 0;

  const ListEntryVisualContext& previous =
      entries_[previous_idx]->visualContext();
  const ListEntryVisualContext& next = entries_[next_idx]->visualContext();

  int16_t gap = 0;
  if (style_ == ListStyle::kSegmented &&
      divider_policy_.mode == DividerMode::kNone) {
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

void List::paint(PaintContext& ctx) const {
  if (!ctx.isDeadlineExceeded()) {
    const roo_display::Color divider_color = theme().color.outlineVariant;
    const int16_t divider_thickness = DividerThicknessPx();

    int previous_visible_idx = -1;
    for (int i = 0; i < static_cast<int>(entries_.size()); ++i) {
      const ListEntry& entry = *entries_[i];
      if (entry.isGone()) continue;
      if (previous_visible_idx >= 0) {
        const ListEntry& previous = *entries_[previous_visible_idx];
        int16_t gap = interRowGap(previous_visible_idx, i);
        if (gap >= divider_thickness) {
          int16_t divider_top = previous.offsetTop() + previous.height() +
                                (gap - divider_thickness) / 2;
          int16_t divider_bottom = divider_top + divider_thickness - 1;
          DividerMetrics divider = ResolveDividerMetrics(
              previous, previous.offsetLeft(), divider_top);
          if (divider.visible) {
            Rect divider_bounds(divider.start_x, divider_top, divider.end_x,
                                divider_bottom);
            ctx.fillRect(divider_bounds, divider_color);
            ctx.addExclusion(divider_bounds);
          }
        }
      }
      previous_visible_idx = i;
    }
  }

  Container::paint(ctx);
}

// Resolves the list-owned row context so each entry sees stable variant,
// style, position, selection, and divider decisions.
void List::refreshEntryVisualContexts() {
  if (!contexts_dirty_) return;

  const int count = static_cast<int>(entries_.size());
  const int first_selected_idx =
      selection_policy_.mode == SelectionMode::kSingle
          ? FirstSelectedIndex(selected_entries_)
          : -1;

  for (int i = 0; i < count; ++i) {
    ListEntry& entry = *entries_[i];
    ListEntryVisualContext context = entry.visualContext();
    context.variant = variant_;
    context.style = style_;
    context.position = PositionForIndex(i, count);
    context.selected = ResolvedSelectedState(
        selected_entries_, selection_policy_, i, first_selected_idx);
    bool next_selected = ResolvedSelectedState(
        selected_entries_, selection_policy_, i + 1, first_selected_idx);
    context.show_divider = ShouldShowDivider(divider_policy_, i, count,
                                             context.selected, next_selected);
    context.divider_mode = divider_policy_.mode;
    context.divider_start_inset = divider_policy_.start_inset;
    context.divider_end_inset = divider_policy_.end_inset;
    entry.setVisualContext(context);
  }

  contexts_dirty_ = false;
}

void List::setVariant(ListVariant variant) {
  if (variant_ == variant) return;
  variant_ = variant;
  onStructureOrPolicyChanged();
}

void List::setStyle(ListStyle style) {
  if (style_ == style) return;
  style_ = style;
  onStructureOrPolicyChanged();
}

void List::setSelectionPolicy(const ListSelectionPolicy& policy) {
  selection_policy_ = policy;
  onStructureOrPolicyChanged();
}

void List::setDividerPolicy(const ListDividerPolicy& policy) {
  divider_policy_ = policy;
  onStructureOrPolicyChanged();
}

void List::add(ListEntry& entry) {
  CHECK(entry.parent() == nullptr);
  addEntryInternal(&entry, WidgetRef(entry));
}

void List::add(std::unique_ptr<ListEntry> entry) {
  CHECK(entry != nullptr);
  ListEntry* raw_entry = entry.get();
  CHECK(raw_entry->parent() == nullptr);
  addEntryInternal(raw_entry, WidgetRef(std::move(entry)));
}

void List::clear() {
  if (entries_.empty()) return;
  for (int i = static_cast<int>(entries_.size()) - 1; i >= 0; --i) {
    detachChild(entries_[i]);
  }
  entries_.clear();
  selected_entries_.clear();
  contexts_dirty_ = false;
  invalidateInterior();
  requestLayout();
}

int List::getChildrenCount() const { return static_cast<int>(entries_.size()); }

const Widget& List::getChild(int idx) const {
  CHECK(idx >= 0);
  CHECK_LT(idx, static_cast<int>(entries_.size()));
  return *entries_[idx];
}

Widget& List::getChild(int idx) {
  return const_cast<Widget&>(static_cast<const List&>(*this).getChild(idx));
}

Dimensions List::onMeasure(WidthSpec width, HeightSpec height) {
  refreshEntryVisualContexts();

  int16_t resolved_width = 0;
  if (width.kind() == EXACTLY) {
    resolved_width = width.value();
  } else {
    WidthSpec row_width = width.kind() == AT_MOST
                              ? WidthSpec::AtMost(width.value())
                              : WidthSpec::Unspecified(0);
    for (ListEntry* entry : entries_) {
      if (entry->isGone()) continue;
      Dimensions measured =
          entry->measure(row_width, HeightSpec::Unspecified(0));
      resolved_width = std::max<int16_t>(resolved_width, measured.width());
    }
    resolved_width = ConstrainWidth(resolved_width, width);
  }

  int16_t total_height = 0;
  int previous_visible_idx = -1;
  for (int i = 0; i < static_cast<int>(entries_.size()); ++i) {
    ListEntry* entry = entries_[i];
    if (entry->isGone()) continue;
    Dimensions measured = entry->measure(WidthSpec::Exactly(resolved_width),
                                         HeightSpec::Unspecified(0));
    if (previous_visible_idx >= 0) {
      total_height += interRowGap(previous_visible_idx, i);
    }
    total_height += measured.height();
    previous_visible_idx = i;
  }

  return Dimensions(resolved_width, ConstrainHeight(total_height, height));
}

void List::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  refreshEntryVisualContexts();

  int16_t y = 0;
  int previous_visible_idx = -1;
  for (int i = 0; i < static_cast<int>(entries_.size()); ++i) {
    ListEntry* entry = entries_[i];
    if (entry->isGone()) continue;
    if (previous_visible_idx >= 0) {
      y += interRowGap(previous_visible_idx, i);
    }
    Dimensions measured = entry->measure(WidthSpec::Exactly(rect.width()),
                                         HeightSpec::Unspecified(0));
    entry->layout(Rect(0, y, rect.width() - 1, y + measured.height() - 1));
    y += measured.height();
    previous_visible_idx = i;
  }
}

}  // namespace material3
}  // namespace roo_windows
