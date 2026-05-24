#include "roo_windows/material3/list/list.h"

#include <algorithm>

#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_logging.h"
#include "roo_windows/core/theme.h"

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

const roo_display::Font& FontForOverline() { return font_overline(); }

const roo_display::Font& FontForHeadline() { return font_body1(); }

const roo_display::Font& FontForSupporting() { return font_body2(); }

int16_t TextLineHeight(const roo_display::Font& font) {
  return font.metrics().maxHeight();
}

int16_t TextWidth(const roo_display::Font& font, roo_display::StringView text) {
  if (text.empty()) return 0;
  return font.getHorizontalStringMetrics(text).advance();
}

uint8_t SlotLineCount(roo_display::StringView text, ListTextPolicy policy) {
  if (text.empty()) return 0;
  return std::max<uint8_t>(1, policy.max_lines);
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
  width = std::max(width,
                   TextWidth(FontForSupporting(), item->supportingText()));

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

// Resolves one shared row geometry so measurement, paint, and child layout all
// agree on the same slot positions and text bounds.
RowLayoutMetrics ResolveRowLayout(const ListEntry& entry, WidthSpec width_spec,
                                  HeightSpec height_spec) {
  const ListItem* item = entry.item();
  const RowTokens tokens = TokensFor(entry.visualContext().variant);

  Dimensions leading = MeasureChild(item == nullptr ? nullptr : item->leading(),
                                    WidthSpec::Unspecified(0),
                                    HeightSpec::Unspecified(0));
  Dimensions trailing = MeasureChild(
      item == nullptr ? nullptr : item->trailing(), WidthSpec::Unspecified(0),
      HeightSpec::Unspecified(0));
  TextSlotMetrics text = MeasureTextSlots(item);

  bool has_leading = leading.width() > 0 || leading.height() > 0;
  bool has_trailing = trailing.width() > 0 || trailing.height() > 0;
  bool has_text = text.width > 0 || text.height > 0;

  int16_t horizontal_gaps = 0;
  if (has_leading && has_text) horizontal_gaps += tokens.slot_gap;
  if (has_trailing && (has_text || has_leading)) horizontal_gaps += tokens.slot_gap;

  int16_t desired_main_width = tokens.horizontal_padding * 2 + leading.width() +
                               trailing.width() + text.width + horizontal_gaps;

  Dimensions body = MeasureChild(item == nullptr ? nullptr : item->body(),
                                 WidthSpec::Unspecified(0),
                                 HeightSpec::Unspecified(0));
  bool has_body = body.width() > 0 || body.height() > 0;
  int16_t desired_body_width = has_body ? tokens.horizontal_padding * 2 +
                                              std::max<int16_t>(body.width(),
                                                                text.width)
                                        : 0;
  int16_t desired_width = std::max(desired_main_width, desired_body_width);
  int16_t resolved_width = ConstrainWidth(desired_width, width_spec);

  // The text column expands into whatever horizontal space remains between the
  // fixed leading and trailing slots.
  int16_t content_width =
      std::max<int16_t>(0, resolved_width - 2 * tokens.horizontal_padding);
  int16_t text_x = tokens.horizontal_padding;
  if (has_leading) text_x += leading.width() + (has_text ? tokens.slot_gap : 0);
  int16_t trailing_x = resolved_width - tokens.horizontal_padding - trailing.width();
  int16_t text_right = has_trailing ? trailing_x - tokens.slot_gap - 1
                                    : resolved_width - tokens.horizontal_padding - 1;
  int16_t text_width = has_text ? std::max<int16_t>(0, text_right - text_x + 1) : 0;

  int16_t main_content_height = std::max<int16_t>(text.height, leading.height());
  main_content_height = std::max<int16_t>(main_content_height, trailing.height());
  int16_t row_band_height = std::max<int16_t>(
      MinRowBandHeight(item, text), main_content_height + 2 * tokens.vertical_padding);
  int16_t body_y = row_band_height;
  if (has_body) body_y += tokens.body_gap;
  int16_t desired_height = row_band_height +
                           (has_body ? tokens.body_gap + body.height() +
                                           tokens.vertical_padding
                                     : 0);
  int16_t resolved_height = ConstrainHeight(desired_height, height_spec);

  int16_t text_y = tokens.vertical_padding;
  if (item == nullptr || !item->preferTopTextAlignment()) {
    text_y = MiddleOffset(row_band_height, text.height);
  }
  int16_t leading_y = item == nullptr
                          ? MiddleOffset(row_band_height, leading.height())
                          : SlotY(item, item->leadingAlignment(), row_band_height,
                                  leading.height(), tokens);
  int16_t trailing_y = item == nullptr
                           ? MiddleOffset(row_band_height, trailing.height())
                           : SlotY(item, item->trailingAlignment(), row_band_height,
                                   trailing.height(), tokens);

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

void DrawTextLine(const Canvas& canvas, roo_display::StringView text,
                  const roo_display::Font& font, Color color, Rect bounds) {
  if (text.empty() || bounds.empty()) return;
  canvas.drawTiled(roo_display::ClippedStringViewLabel(text, font, color),
                   bounds, roo_display::kLeft | roo_display::kMiddle);
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
                           const ListSelectionPolicy& selection_policy,
                           int idx, int first_selected_idx) {
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

StandardListItemInit StandardListItemInit::OneLine(
    roo_display::StringView headline, Widget* leading, Widget* trailing) {
  StandardListItemInit init;
  init.headline = headline;
  init.leading = leading;
  init.trailing = trailing;
  return init;
}

StandardListItemInit StandardListItemInit::TwoLine(
    roo_display::StringView headline, roo_display::StringView supporting,
    Widget* leading, Widget* trailing) {
  StandardListItemInit init = OneLine(headline, leading, trailing);
  init.supporting = supporting;
  return init;
}

StandardListItemInit StandardListItemInit::ThreeLine(
    roo_display::StringView headline, roo_display::StringView supporting,
    roo_display::StringView overline, Widget* leading, Widget* trailing,
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

ExpandablePanel::ExpandablePanel(const Environment& env)
    : Container(env),
      content_(),
      animation_duration_millis_(0),
      animation_progress_millis_(0),
      expanded_(false) {}

void ExpandablePanel::setContent(WidgetRef content) {
  (void)content;
  LOG(FATAL) << "Unimplemented: material3::ExpandablePanel::setContent";
}

void ExpandablePanel::clearContent() {
  if (content_.get() == nullptr) return;
  LOG(FATAL) << "Unimplemented: material3::ExpandablePanel::clearContent";
}

void ExpandablePanel::setExpanded(bool expanded, bool animate) {
  (void)animate;
  if (!expanded && !expanded_) return;
  LOG(FATAL) << "Unimplemented: material3::ExpandablePanel::setExpanded";
}

bool ExpandablePanel::isExpanded() const { return expanded_; }

bool ExpandablePanel::isAnimating() const {
  (void)animation_progress_millis_;
  return false;
}

void ExpandablePanel::setAnimationDuration(uint16_t millis) {
  animation_duration_millis_ = millis;
}

int ExpandablePanel::getChildrenCount() const { return 0; }

const Widget& ExpandablePanel::getChild(int idx) const {
  (void)idx;
  LOG(FATAL) << "Unimplemented: material3::ExpandablePanel::getChild";
  return *static_cast<const Widget*>(nullptr);
}

Widget& ExpandablePanel::getChild(int idx) {
  return const_cast<Widget&>(
      static_cast<const ExpandablePanel&>(*this).getChild(idx));
}

ListEntry::ListEntry(const Environment& env)
    : Container(env),
      item_(nullptr),
      leading_child_(nullptr),
      trailing_child_(nullptr),
      body_child_(nullptr),
      visual_context_() {}

ListEntry::~ListEntry() { clearItem(); }

bool ListEntry::hasItem() const { return item_ != nullptr; }

ListItem* ListEntry::item() { return item_; }

const ListItem* ListEntry::item() const { return item_; }

void ListEntry::setItem(ListItem& item) {
  if (item_ == &item) {
    refreshFromItem();
    return;
  }
  // A binding owns a stable borrowed slot set for its whole lifetime, so a
  // rebind always detaches the old children before attaching the new ones.
  clearItem();
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
  invalidateInterior();
  requestLayout();
}

void ListEntry::clearItem() {
  if (item_ == nullptr) return;
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
  invalidateInterior();
  requestLayout();
}

void ListEntry::refreshFromItem() {
  if (item_ == nullptr) return;
  CHECK(item_->leading() == leading_child_);
  CHECK(item_->trailing() == trailing_child_);
  CHECK(item_->body() == body_child_);
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

void ListEntry::paint(const Canvas& canvas) const {
  // Borrowed slot widgets paint through normal child traversal; this pass only
  // settles the row background and descriptor text.
  canvas.clear();
  if (item_ == nullptr) return;
  RowLayoutMetrics layout = ResolveRowLayout(
      *this, WidthSpec::Exactly(width()), HeightSpec::Exactly(height()));
  Color headline_color = theme().color.onSurface;
  Color supporting_color = theme().color.onSurfaceVariant;

  int16_t y = layout.text_y;
  if (!item_->overlineText().empty()) {
    Rect line_bounds(layout.text_x, y, layout.text_x + layout.text_width - 1,
                     y + TextLineHeight(FontForOverline()) - 1);
    DrawTextLine(canvas, item_->overlineText(), FontForOverline(),
                 supporting_color, line_bounds);
    y += TextLineHeight(FontForOverline());
  }
  if (!item_->headlineText().empty()) {
    Rect line_bounds(layout.text_x, y, layout.text_x + layout.text_width - 1,
                     y + TextLineHeight(FontForHeadline()) - 1);
    DrawTextLine(canvas, item_->headlineText(), FontForHeadline(),
                 headline_color, line_bounds);
    y += TextLineHeight(FontForHeadline());
  }
  if (!item_->supportingText().empty()) {
    Rect line_bounds(layout.text_x, y, layout.text_x + layout.text_width - 1,
                     y + TextLineHeight(FontForSupporting()) - 1);
    DrawTextLine(canvas, item_->supportingText(), FontForSupporting(),
                 supporting_color, line_bounds);
  }
}

Dimensions ListEntry::getSuggestedMinimumDimensions() const {
  RowLayoutMetrics layout = ResolveRowLayout(
      *this, WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  return Dimensions(layout.width, layout.height);
}

int ListEntry::getChildrenCount() const {
  int count = 0;
  if (leading_child_ != nullptr) ++count;
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
  RowLayoutMetrics layout = ResolveRowLayout(
      *this, WidthSpec::Exactly(rect.width()), HeightSpec::Exactly(rect.height()));
  if (leading_child_ != nullptr && !leading_child_->isGone()) {
    leading_child_->layout(Rect(layout.leading_x, layout.leading_y,
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

roo_display::StringView StandardListItem::overlineText() const {
  return overline_;
}

roo_display::StringView StandardListItem::headlineText() const {
  return headline_;
}

roo_display::StringView StandardListItem::supportingText() const {
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

Widget* StandardListItem::leading() const { return leading_; }

Widget* StandardListItem::trailing() const { return trailing_; }

Widget* StandardListItem::body() const { return body_; }

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

List::List(const Environment& env)
    : Container(env),
      entries_(),
      selected_entries_(),
      variant_(ListVariant::kExpressive),
      style_(ListStyle::kStandard),
      selection_policy_(),
      divider_policy_(),
      contexts_dirty_(false) {}

List::~List() { clear(); }

void List::markEntryContextsDirty() {
  contexts_dirty_ = true;
  invalidateInterior();
  requestLayout();
}

int16_t List::interRowGap() const {
  if (style_ == ListStyle::kSegmented && divider_policy_.mode == DividerMode::kNone) {
    return Scaled(kSegmentedListGapDp);
  }
  return 0;
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
    context.selected = ResolvedSelectedState(selected_entries_, selection_policy_,
                                            i, first_selected_idx);
    bool next_selected = ResolvedSelectedState(selected_entries_, selection_policy_,
                                               i + 1, first_selected_idx);
    context.show_divider =
        ShouldShowDivider(divider_policy_, i, count, context.selected, next_selected);
    entry.setVisualContext(context);
  }

  contexts_dirty_ = false;
}

void List::setVariant(ListVariant variant) {
  if (variant_ == variant) return;
  variant_ = variant;
  markEntryContextsDirty();
  refreshEntryVisualContexts();
}

void List::setStyle(ListStyle style) {
  if (style_ == style) return;
  style_ = style;
  markEntryContextsDirty();
  refreshEntryVisualContexts();
}

void List::setSelectionPolicy(const ListSelectionPolicy& policy) {
  selection_policy_ = policy;
  markEntryContextsDirty();
  refreshEntryVisualContexts();
}

void List::setDividerPolicy(const ListDividerPolicy& policy) {
  divider_policy_ = policy;
  markEntryContextsDirty();
  refreshEntryVisualContexts();
}

void List::add(ListEntry& entry) {
  CHECK(entry.parent() == nullptr);
  selected_entries_.push_back(entry.visualContext().selected ? 1 : 0);
  entries_.push_back(&entry);
  attachChild(WidgetRef(entry));
  markEntryContextsDirty();
  refreshEntryVisualContexts();
}

void List::add(std::unique_ptr<ListEntry> entry) {
  CHECK(entry != nullptr);
  ListEntry* raw_entry = entry.get();
  CHECK(raw_entry->parent() == nullptr);
  selected_entries_.push_back(raw_entry->visualContext().selected ? 1 : 0);
  entries_.push_back(raw_entry);
  attachChild(WidgetRef(std::move(entry)));
  markEntryContextsDirty();
  refreshEntryVisualContexts();
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
    WidthSpec row_width = width.kind() == AT_MOST ? WidthSpec::AtMost(width.value())
                                                  : WidthSpec::Unspecified(0);
    for (ListEntry* entry : entries_) {
      if (entry->isGone()) continue;
      Dimensions measured = entry->measure(row_width, HeightSpec::Unspecified(0));
      resolved_width = std::max<int16_t>(resolved_width, measured.width());
    }
    resolved_width = ConstrainWidth(resolved_width, width);
  }

  const int16_t gap = interRowGap();
  int16_t total_height = 0;
  bool has_visible_entry = false;
  for (ListEntry* entry : entries_) {
    if (entry->isGone()) continue;
    Dimensions measured =
        entry->measure(WidthSpec::Exactly(resolved_width), HeightSpec::Unspecified(0));
    if (has_visible_entry) total_height += gap;
    total_height += measured.height();
    has_visible_entry = true;
  }

  return Dimensions(resolved_width, ConstrainHeight(total_height, height));
}

void List::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  refreshEntryVisualContexts();

  const int16_t gap = interRowGap();
  int16_t y = 0;
  for (ListEntry* entry : entries_) {
    if (entry->isGone()) continue;
    Dimensions measured =
        entry->measure(WidthSpec::Exactly(rect.width()), HeightSpec::Unspecified(0));
    entry->layout(Rect(0, y, rect.width() - 1, y + measured.height() - 1));
    y += measured.height() + gap;
  }
}

}  // namespace material3
}  // namespace roo_windows