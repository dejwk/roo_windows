#include "roo_windows/material3/app_bar/app_bar.h"

#include <algorithm>
#include <cstdlib>
#include <utility>

#include "roo_display/shape/smooth.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_logging.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/child_layout.h"
#include "roo_windows/material3/app_bar/app_bar_tokens.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3 {
namespace {

void CheckTrailingIndex(uint8_t index) {
  CHECK(index < 2) << "Material 3 app-bar trailing index must be 0 or 1";
}

// Counts only populated fixed child slots without allocating storage.
template <size_t N>
int ChildCount(Widget* const (&slots)[N]) {
  int result = 0;
  for (Widget* slot : slots) result += slot != nullptr;
  return result;
}

// Returns a populated fixed child slot or terminates on an invalid index.
template <size_t N>
const Widget& ChildAt(Widget* const (&slots)[N], int idx, const char* owner) {
  for (Widget* slot : slots) {
    if (slot != nullptr && idx-- == 0) return *slot;
  }
  LOG(FATAL) << owner << " child index out of bounds";
  return *slots[0];
}

// Presentation children (the passive search glyph and display text) must not
// swallow a search-surface tap. A supplied descendant still gets first refusal
// when it is itself interactive, so callers can attach independent actions.
bool AppendInteractiveChildTouchTarget(Widget& child, XDim x, YDim y,
                                       bool sloppy,
                                       std::vector<Widget*>& path) {
  const size_t old_size = path.size();
  const bool hit =
      sloppy ? child.fillSloppyTouchTargetPath(x - child.offsetLeft(),
                                               y - child.offsetTop(), path)
             : child.fillTouchTargetPath(x - child.offsetLeft(),
                                         y - child.offsetTop(), path);
  if (hit && !path.empty() && path.back()->isClickable()) return true;
  path.resize(old_size);
  return false;
}

}  // namespace

void internal::AppBarText::setText(roo::string_view text) {
  if (text_ == text) return;
  Rect previous = getParentContentBounds();
  text_ = text;
  invalidateInterior();
  notifyParentInvalidatedRegion(previous);
}

void internal::AppBarText::setTextStyle(const TextStyle& style) {
  if (text_style_ == &style) return;
  Rect previous = getParentContentBounds();
  text_style_ = &style;
  invalidateInterior();
  notifyParentInvalidatedRegion(previous);
}

void internal::AppBarText::setAlignment(roo_display::Alignment alignment) {
  if (alignment_ == alignment) return;
  Rect previous = getParentContentBounds();
  alignment_ = alignment;
  invalidateInterior();
  notifyParentInvalidatedRegion(previous);
}

Dimensions internal::AppBarText::getSuggestedMinimumDimensions() const {
  if (text_.empty()) return Dimensions(0, 0);
  const TextStyle& style =
      text_style_ == nullptr ? text_style_body_medium() : *text_style_;
  return Dimensions(style.font()
                        .getHorizontalStringMetrics(text_, fontOptions(style))
                        .advance(),
                    style.lineHeight());
}

roo_display::Color internal::AppBarTitle::textColor(
    roo_display::Color background) const {
  roo_display::Color foreground = AppBarText::textColor(background);
  if (parent() == nullptr) return foreground;
  auto connection = internal::FindAppBarConnection(*parent());
  if (connection == nullptr || connection->limit() == 0 ||
      static_cast<const AppBar&>(*parent()).variant() ==
          AppBarVariant::kSmall) {
    return foreground;
  }
  // Switch typography and anchors while invisible at half collapse.
  const int visibility =
      std::abs(connection->limit() - 2 * connection->collapse());
  const uint8_t alpha =
      (255 * visibility + connection->limit() / 2) / connection->limit();
  return roo_display::AlphaBlend(background, foreground.withA(alpha));
}

roo_display::Color internal::AppBarText::textColor(
    roo_display::Color background) const {
  return use_on_surface_variant_
             ? theme().material3Theme().color.onSurfaceVariant
             : theme().material3Theme().color.onSurface;
}

roo_display::Color internal::AppBarSubtitle::textColor(
    roo_display::Color background) const {
  roo_display::Color foreground =
      theme().material3Theme().color.onSurfaceVariant;
  if (parent() == nullptr) return foreground;
  auto connection = internal::FindAppBarConnection(*parent());
  if (connection == nullptr || connection->limit() == 0 ||
      connection->collapse() == 0) {
    return foreground;
  }
  // Fade to the painted surface before the subtitle leaves the title stack
  // at half collapse. Use the retained scroll sample, without opacity state.
  YDim remaining =
      std::max<YDim>(0, connection->limit() - 2 * connection->collapse());
  uint8_t alpha =
      (255 * remaining + connection->limit() / 2) / connection->limit();
  return roo_display::AlphaBlend(background, foreground.withA(alpha));
}

Insets internal::AppBarText::getInkInsets() const {
  if (text_.empty()) return Insets(0, 0, bounds().width(), bounds().height());
  const TextStyle& style =
      text_style_ == nullptr ? text_style_body_medium() : *text_style_;
  roo_display::StringViewLabel label(
      text_, style.font(), roo_display::color::Transparent, fontOptions(style));
  auto offset =
      ResolveAlignmentOffset(bounds(), Rect(label.anchorExtents()), alignment_);
  Rect ink = Rect(label.extents()).translate(offset.first, offset.second);
  return Insets(ink.xMin() - bounds().xMin(), ink.yMin() - bounds().yMin(),
                bounds().xMax() - ink.xMax(), bounds().yMax() - ink.yMax());
}

void internal::AppBarText::paint(PaintContext& ctx) const {
  if (text_.empty()) return;
  const TextStyle& style =
      text_style_ == nullptr ? text_style_body_medium() : *text_style_;
  roo_display::StringViewLabel label(text_, style.font(),
                                     textColor(ctx.canvas().bgcolor()),
                                     fontOptions(style));
  auto offset =
      ResolveAlignmentOffset(bounds(), Rect(label.anchorExtents()), alignment_);
  ctx.drawObject(
      roo_display::Tile(&label, label.extents(), roo_display::kNoAlign),
      offset.first, offset.second);
}

AppBar::AppBar(ApplicationContext& context, AppBarVariant variant)
    : Material3Container(context),
      title_widget_(context),
      subtitle_widget_(context),
      leading_(nullptr),
      trailing_{nullptr, nullptr},
      variant_(variant),
      title_alignment_(AppBarTitleAlignment::kLeading),
      surface_state_(AppBarSurfaceState::kFlat) {
  // The title children are by-value, so a title-only bar has no dynamic
  // allocations and text updates follow the regular child invalidation path.
  title_widget_.setTextStyle(titleTextStyle());
  subtitle_widget_.setTextStyle(text_style_body_medium());
  title_widget_.setVisibility(Visibility::kGone);
  subtitle_widget_.setVisibility(Visibility::kGone);
  attachChild(title_widget_);
  attachChild(subtitle_widget_);
}

AppBar::~AppBar() {
  roo_windows::internal::ScrollConnectionRegistry::Disconnect(*this);
  for (Widget* slot : trailing_) {
    if (slot) detachChild(slot);
  }
  if (leading_) detachChild(leading_);
  detachChild(&subtitle_widget_);
  detachChild(&title_widget_);
}

const internal::AppBarVariantTokens& AppBar::tokens() const {
  switch (variant_) {
    case AppBarVariant::kSmall:
      return internal::kSmallAppBarTokens;
    case AppBarVariant::kMediumFlexible:
      return internal::kMediumFlexibleAppBarTokens;
    case AppBarVariant::kLargeFlexible:
      return internal::kLargeFlexibleAppBarTokens;
  }
  return internal::kSmallAppBarTokens;
}

const TextStyle& AppBar::titleTextStyle() const {
  auto connection = internal::FindAppBarConnection(*this);
  if (connection != nullptr && connection->limit() > 0 &&
      connection->collapse() * 2 >= connection->limit()) {
    return text_style_title_large();
  }
  return expandedTitleTextStyle();
}

const TextStyle& AppBar::expandedTitleTextStyle() const {
  switch (variant_) {
    case AppBarVariant::kSmall:
      return text_style_title_large();
    case AppBarVariant::kMediumFlexible:
      return text_style_headline_small();
    case AppBarVariant::kLargeFlexible:
      return text_style_headline_medium();
  }
  return text_style_title_large();
}

int16_t AppBar::containerHeightDp() const {
  const internal::AppBarVariantTokens& variant_tokens = tokens();
  return variant_tokens.supports_subtitle && !subtitle_widget_.text().empty()
             ? variant_tokens.subtitle_container_height_dp
             : variant_tokens.container_height_dp;
}

void AppBar::setVariant(AppBarVariant variant) {
  if (variant_ == variant) return;
  variant_ = variant;
  title_widget_.setTextStyle(titleTextStyle());
  if (!tokens().supports_subtitle) {
    subtitle_widget_.setVisibility(Visibility::kGone);
  } else if (!subtitle_widget_.text().empty()) {
    subtitle_widget_.setVisibility(Visibility::kVisible);
  }
  invalidateInterior();
  requestLayout();
}

void AppBar::setTitleAlignment(AppBarTitleAlignment alignment) {
  if (title_alignment_ == alignment) return;
  title_alignment_ = alignment;
  const roo_display::Alignment text_alignment =
      (alignment == AppBarTitleAlignment::kCentered ? roo_display::kCenter
                                                    : roo_display::kLeft) |
      roo_display::kMiddle;
  title_widget_.setAlignment(text_alignment);
  subtitle_widget_.setAlignment(text_alignment);
  invalidateInterior();
  requestLayout();
}

AppBarSurfaceState AppBar::surfaceState() const {
  auto connection = internal::FindAppBarConnection(*this);
  return connection == nullptr
             ? surface_state_
             : (connection->scrolled() ? AppBarSurfaceState::kScrolled
                                       : AppBarSurfaceState::kFlat);
}

ScrollConnectionStatus AppBar::setScrollBehavior(
    SimpleScrollablePanel& panel, AppBarScrollBehavior behavior) {
  auto previous = internal::FindAppBarConnection(*this);
  ScrollConnectionStatus status =
      internal::ConnectAppBar(context(), *this, panel, behavior, false);
  if (status == ScrollConnectionStatus::kSuccess) {
    auto connection = internal::FindAppBarConnection(*this);
    if (connection == previous) return status;
    // Resolve the compact font at attachment, not on a drag frame.
    (void)text_style_title_large();
    connection->measureHeight(
        Scaled(containerHeightDp()),
        variant_ == AppBarVariant::kSmall &&
                connection->behavior() == AppBarScrollBehavior::kEnterAlways
            ? 0
            : Scaled(64),
        HeightSpec::Unspecified(0));
    connection->onPositionChanged({}, panel.getScrollPosition(),
                                  ScrollSource::kProgrammatic);
    requestLayout();
  }
  return status;
}

ScrollConnectionStatus AppBar::clearScrollBehavior() {
  return internal::ClearAppBarConnection(context(), *this);
}

bool AppBar::hasScrollBehavior() const {
  return internal::FindAppBarConnection(*this) != nullptr;
}

void AppBar::onAnimationFrame(AnimationTag tag, const AnimationSample& sample) {
  auto connection = internal::FindAppBarConnection(*this);
  if (tag == internal::AppBarScrollConnection::kSettle && connection != nullptr)
    connection->animate(sample);
  else
    Material3Container::onAnimationFrame(tag, sample);
}

void AppBar::onPresentationChanged(const PresentationChange& change) {
  auto connection = internal::FindAppBarConnection(*this);
  if (connection != nullptr && (change.state != PresentationState::kPresented ||
                                change.detached_since_delivery))
    connection->suspend();
}

void AppBar::setSurfaceState(AppBarSurfaceState state) {
  if (surface_state_ == state) return;
  surface_state_ = state;
  invalidateInterior();
}

void AppBar::setTitle(roo::string_view title) {
  if (title_widget_.text() == title) return;
  title_widget_.setText(title);
  title_widget_.setVisibility(title.empty() ? Visibility::kGone
                                            : Visibility::kVisible);
  invalidateInterior();
  requestLayout();
}

void AppBar::setSubtitle(roo::string_view subtitle) {
  if (subtitle_widget_.text() == subtitle) return;
  subtitle_widget_.setText(subtitle);
  subtitle_widget_.setVisibility(!subtitle.empty() && tokens().supports_subtitle
                                     ? Visibility::kVisible
                                     : Visibility::kGone);
  invalidateInterior();
  requestLayout();
}

void AppBar::replaceSlot(Widget*& slot, WidgetRef widget) {
  Widget* incoming = widget.get();
  if (incoming == slot) return;
  if (slot) detachChild(slot);
  slot = incoming;
  if (slot) {
    CHECK(slot->parent() == nullptr);
    attachChild(std::move(widget));
  }
  invalidateInterior();
  requestLayout();
}

void AppBar::setLeading(WidgetRef widget) {
  replaceSlot(leading_, std::move(widget));
}

void AppBar::setTrailing(uint8_t index, WidgetRef widget) {
  CheckTrailingIndex(index);
  replaceSlot(trailing_[index], std::move(widget));
}

int AppBar::getChildrenCount() const {
  return (!title_widget_.isGone()) + (!subtitle_widget_.isGone()) +
         (leading_ != nullptr) + ChildCount(trailing_);
}

const Widget& AppBar::getChild(int idx) const {
  if (!title_widget_.isGone() && idx-- == 0) return title_widget_;
  if (!subtitle_widget_.isGone() && idx-- == 0) return subtitle_widget_;
  if (leading_ && idx-- == 0) return *leading_;
  return ChildAt(trailing_, idx, "AppBar");
}

Widget& AppBar::getChild(int idx) {
  if (!title_widget_.isGone() && idx-- == 0) return title_widget_;
  if (!subtitle_widget_.isGone() && idx-- == 0) return subtitle_widget_;
  if (leading_ && idx-- == 0) return *leading_;
  for (Widget* slot : trailing_) {
    if (slot && idx-- == 0) return *slot;
  }
  LOG(FATAL) << "AppBar child index out of bounds";
  return *leading_;
}

ColorToken AppBar::containerRole() const {
  const AppBarTheme& app_bar = theme().material3Theme().components.appBar;
  return surfaceState() == AppBarSurfaceState::kFlat
             ? app_bar.flatContainer
             : app_bar.scrolledContainer;
}

Dimensions AppBar::onMeasure(WidthSpec width, HeightSpec height) {
  const int16_t row_height = Scaled(internal::kActionTapTargetDp);
  const int16_t container_height = Scaled(containerHeightDp());
  auto connection = internal::FindAppBarConnection(*this);
  YDim resolved_height =
      connection == nullptr ? height.resolveSize(container_height)
                            : connection->measureHeight(
                                  container_height,
                                  variant_ == AppBarVariant::kSmall &&
                                          connection->behavior() ==
                                              AppBarScrollBehavior::kEnterAlways
                                      ? 0
                                      : Scaled(64),
                                  height);
  title_widget_.setTextStyle(titleTextStyle());
  bool subtitle_visible = tokens().supports_subtitle &&
                          !subtitle_widget_.text().empty() &&
                          (connection == nullptr || connection->limit() == 0 ||
                           connection->collapse() * 2 < connection->limit());
  subtitle_widget_.setVisibility(subtitle_visible ? Visibility::kVisible
                                                  : Visibility::kGone);
  const int16_t available_width = width.value();

  // Measure all children even when an exact app-bar width leaves them no room;
  // this preserves the normal child layout-request lifecycle.
  if (leading_ != nullptr) {
    MeasureChildWithMargins(*leading_, WidthSpec::AtMost(row_height),
                            HeightSpec::AtMost(row_height));
  }
  for (Widget* slot : trailing_) {
    if (slot != nullptr) {
      MeasureChildWithMargins(*slot, WidthSpec::AtMost(row_height),
                              HeightSpec::AtMost(row_height));
    }
  }
  title_widget_.measure(
      WidthSpec::AtMost(std::max<int16_t>(0, available_width)),
      HeightSpec::Unspecified(container_height));
  subtitle_widget_.measure(
      WidthSpec::AtMost(std::max<int16_t>(0, available_width)),
      HeightSpec::Unspecified(container_height));
  return Dimensions(width.resolveSize(available_width), resolved_height);
}

void AppBar::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  const int16_t edge = Scaled(internal::kAppBarEdgeInsetDp);
  const int16_t title_inset = Scaled(internal::kAppBarTitleInsetDp);
  const int16_t action_size = Scaled(internal::kActionTapTargetDp);
  const int16_t width = std::max<int16_t>(0, rect.width());
  auto connection = internal::FindAppBarConnection(*this);
  title_widget_.setAlignment(
      (title_alignment_ == AppBarTitleAlignment::kCentered
           ? roo_display::kCenter
           : roo_display::kLeft) |
      roo_display::kMiddle);
  const YDim collapse = connection == nullptr ? 0 : connection->collapse();
  const int16_t height = std::max<int16_t>(0, rect.height() + collapse);
  const bool single_row = variant_ == AppBarVariant::kSmall;
  const bool compact_title = !single_row && connection != nullptr &&
                             connection->limit() > 0 &&
                             collapse * 2 >= connection->limit();
  int16_t left = std::min<int16_t>(edge, width);
  int16_t right = std::max<int16_t>(left, width - edge);

  // Flexible bars retain the compact bar's action row throughout collapse:
  // a 48dp slot with 8dp above and below. The title stack changes position.
  const int16_t action_row_height =
      single_row ? height
                 : Scaled(internal::kSmallAppBarTokens.container_height_dp);
  const int16_t action_y =
      std::max<int16_t>(0, (action_row_height - action_size) / 2);
  auto layout_action = [action_size, action_y](Widget* child, int16_t x) {
    if (child == nullptr) return;
    // Keep the action slot fixed while allowing a smaller visual surface.
    Dimensions measured =
        MeasureChildWithMargins(*child, WidthSpec::AtMost(action_size),
                                HeightSpec::AtMost(action_size));
    const int16_t left = x + (action_size - measured.width()) / 2;
    const int16_t top = action_y + (action_size - measured.height()) / 2;
    LayoutChildWithMargins(*child, Rect(left, top, left + measured.width() - 1,
                                        top + measured.height() - 1));
  };
  if (leading_ != nullptr) {
    const int16_t slot = std::min<int16_t>(action_size, right - left);
    layout_action(leading_, left);
    left += slot;
  }
  for (int index = 1; index >= 0; --index) {
    if (trailing_[index] == nullptr) continue;
    const int16_t slot =
        std::min<int16_t>(action_size, std::max<int16_t>(0, right - left));
    right -= slot;
    layout_action(trailing_[index], right);
  }

  // A single-row title starts at the 16dp title inset without navigation, or
  // 4dp after its 48dp navigation slot. Expanded flexible titles use the
  // second row and therefore do not reserve navigation/action width.
  if (!single_row && !compact_title) {
    left = std::min<int16_t>(title_inset, width);
    right = std::max<int16_t>(left, width - title_inset);
  } else {
    const int16_t gap = Scaled(internal::kAppBarTitleActionGapDp);
    left = std::min<int16_t>(leading_ == nullptr ? title_inset : left + gap,
                             right);
    if (ChildCount(trailing_) != 0) right -= gap;
    right = std::max<int16_t>(left, right);
  }

  const int16_t title_row_height = compact_title ? action_row_height : height;
  const int16_t lane_top =
      single_row || compact_title ? 0 : action_size + 2 * edge;
  const int16_t lane_bottom =
      single_row || compact_title
          ? title_row_height
          : std::max<int16_t>(lane_top,
                              height - Scaled(tokens().title_bottom_inset_dp));
  const int16_t lane_height = std::max<int16_t>(0, lane_bottom - lane_top);
  const int16_t lane_width = std::max<int16_t>(0, right - left);
  const bool show_subtitle = !subtitle_widget_.isGone();
  const int16_t title_height =
      std::min<int16_t>(title_widget_
                            .measure(WidthSpec::AtMost(lane_width),
                                     HeightSpec::AtMost(lane_height))
                            .height(),
                        lane_height);
  const int16_t subtitle_height =
      show_subtitle
          ? std::min<int16_t>(subtitle_widget_
                                  .measure(WidthSpec::AtMost(lane_width),
                                           HeightSpec::AtMost(lane_height))
                                  .height(),
                              lane_height - title_height)
          : 0;
  const int16_t stack_height = title_height + subtitle_height;
  const int16_t stack_top = single_row || compact_title
                                ? (title_row_height - stack_height) / 2
                                : lane_bottom - stack_height - collapse;
  title_widget_.layout(
      Rect(left, stack_top, right - 1, stack_top + title_height - 1));
  if (show_subtitle) {
    subtitle_widget_.layout(Rect(left, stack_top + title_height, right - 1,
                                 stack_top + stack_height - 1));
  }
  if (collapse > 0 && single_row) {
    for (int i = 0; i < getChildrenCount(); ++i) {
      Widget& child = getChild(i);
      child.layout(child.parent_bounds().translate(0, -collapse));
    }
  }
}

SearchBar::SearchBar(ApplicationContext& context)
    : Material3Container(context),
      display_text_widget_(context),
      passive_search_icon_(context, ic_outlined_24_action_search()),
      leading_(nullptr),
      trailing_{nullptr, nullptr} {
  // These presentation children are stored by value and borrowed by the
  // container, avoiding heap ownership for the default search treatment.
  display_text_widget_.setUseOnSurfaceVariant(true);
  attachChild(display_text_widget_);
  attachChild(passive_search_icon_);
}

SearchBar::~SearchBar() {
  for (Widget* slot : trailing_) {
    if (slot) detachChild(slot);
  }
  if (leading_) detachChild(leading_);
  detachChild(&passive_search_icon_);
  detachChild(&display_text_widget_);
}

void SearchBar::setDisplayText(roo::string_view text) {
  if (display_text_ != text) {
    display_text_ = text;
    display_text_widget_.setText(text);
    invalidateInterior();
    requestLayout();
  }
}

::roo_windows::material3::ColorToken SearchBar::containerRole() const {
  return theme().material3Theme().components.searchBar.container;
}

const internal::SearchEntryTokens& SearchBar::entryTokens() const {
  return internal::kStandaloneSearchEntryTokens;
}

BorderStyle SearchBar::getBorderStyle() const {
  return BorderStyle(static_cast<uint8_t>(std::min<int16_t>(Scaled(28), 0xff)),
                     0);
}

bool SearchBar::fillTouchTargetPath(XDim x, YDim y,
                                    std::vector<Widget*>& path) {
  if (!isVisible() || !isEnabled() || !bounds().contains(x, y)) return false;
  path.push_back(this);
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible() || !child.isEnabled() ||
        !child.parent_bounds().contains(x, y)) {
      continue;
    }
    if (AppendInteractiveChildTouchTarget(child, x, y, false, path)) {
      return true;
    }
  }
  return true;
}

bool SearchBar::fillSloppyTouchTargetPath(XDim x, YDim y,
                                          std::vector<Widget*>& path) {
  if (!isVisible() || !isEnabled() || !getSloppyTouchBounds().contains(x, y)) {
    return false;
  }
  path.push_back(this);
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible() || !child.isEnabled() ||
        !child.getMaxSloppyTouchParentBounds().contains(x, y)) {
      continue;
    }
    const bool within_bounds = child.parent_bounds().contains(x, y);
    if (AppendInteractiveChildTouchTarget(child, x, y, !within_bounds, path)) {
      return true;
    }
  }
  return true;
}

void SearchBar::replaceSlot(Widget*& slot, WidgetRef widget) {
  Widget* incoming = widget.get();
  if (incoming == slot) return;
  if (slot) detachChild(slot);
  slot = incoming;
  if (slot) {
    CHECK(slot->parent() == nullptr);
    attachChild(std::move(widget));
  }
  invalidateInterior();
  requestLayout();
}

void SearchBar::setLeading(WidgetRef widget) {
  passive_search_icon_.setVisibility(
      widget.get() == nullptr ? Visibility::kVisible : Visibility::kGone);
  replaceSlot(leading_, std::move(widget));
}

void SearchBar::setTrailing(uint8_t index, WidgetRef widget) {
  CheckTrailingIndex(index);
  replaceSlot(trailing_[index], std::move(widget));
}

int SearchBar::getChildrenCount() const {
  return 2 + (leading_ != nullptr) + ChildCount(trailing_);
}

const Widget& SearchBar::getChild(int idx) const {
  if (idx-- == 0) return display_text_widget_;
  if (idx-- == 0) return passive_search_icon_;
  if (leading_ && idx-- == 0) return *leading_;
  return ChildAt(trailing_, idx, "SearchBar");
}

Widget& SearchBar::getChild(int idx) {
  if (idx-- == 0) return display_text_widget_;
  if (idx-- == 0) return passive_search_icon_;
  if (leading_ && idx-- == 0) return *leading_;
  for (Widget* slot : trailing_) {
    if (slot && idx-- == 0) return *slot;
  }
  LOG(FATAL) << "SearchBar child index out of bounds";
  return *leading_;
}

Dimensions SearchBar::onMeasure(WidthSpec width, HeightSpec height) {
  const internal::SearchEntryTokens& tokens = entryTokens();
  const int16_t edge = Scaled(tokens.edge_padding_dp);
  const int16_t gap = Scaled(tokens.slot_gap_dp);
  const int16_t row_height = Scaled(tokens.container_height_dp);
  Widget* leading = leading_ == nullptr ? &passive_search_icon_ : leading_;
  int16_t occupied = edge * 2;
  occupied += MeasureChildWithMargins(*leading, WidthSpec::AtMost(row_height),
                                      HeightSpec::Exactly(row_height))
                  .width();
  if (!display_text_.empty()) occupied += gap;
  for (Widget* slot : trailing_) {
    if (slot == nullptr) continue;
    occupied +=
        gap + MeasureChildWithMargins(*slot, WidthSpec::AtMost(row_height),
                                      HeightSpec::Exactly(row_height))
                  .width();
  }
  Dimensions text = display_text_widget_.measure(
      WidthSpec::Unspecified(0), HeightSpec::AtMost(row_height));
  const int16_t natural = occupied + text.width();
  const int16_t preferred =
      std::max<int16_t>(Scaled(240), std::min<int16_t>(Scaled(720), natural));
  return Dimensions(width.resolveSize(preferred),
                    height.resolveSize(row_height));
}

void SearchBar::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  const internal::SearchEntryTokens& tokens = entryTokens();
  const int16_t edge = Scaled(tokens.edge_padding_dp);
  const int16_t gap = Scaled(tokens.slot_gap_dp);
  const int16_t row_height = rect.height();
  int16_t left = edge;
  int16_t right = rect.width() - edge;
  auto place = [row_height](Widget* child, int16_t x, int16_t max_width) {
    if (child == nullptr) return int16_t{0};
    Dimensions measured = MeasureChildWithMargins(
        *child, WidthSpec::AtMost(max_width), HeightSpec::AtMost(row_height));
    int16_t w = std::min<int16_t>(measured.width(), max_width);
    int16_t h = std::min<int16_t>(measured.height(), row_height);
    LayoutChildWithMargins(*child, Rect(x, (row_height - h) / 2, x + w - 1,
                                        (row_height - h) / 2 + h - 1));
    return w;
  };
  Widget* leading = leading_ == nullptr ? &passive_search_icon_ : leading_;
  left += place(leading, left, std::max<int16_t>(0, right - left)) + gap;
  for (int i = 1; i >= 0; --i) {
    Widget* slot = trailing_[i];
    if (slot == nullptr) continue;
    Dimensions measured = MeasureChildWithMargins(
        *slot, WidthSpec::AtMost(std::max<int16_t>(0, right - left)),
        HeightSpec::AtMost(row_height));
    int16_t w =
        std::min<int16_t>(measured.width(), std::max<int16_t>(0, right - left));
    right -= w;
    place(slot, right, w);
    right -= gap;
  }
  int16_t text_width = std::max<int16_t>(0, right - left);
  display_text_widget_.layout(
      Rect(left, 0, left + text_width - 1, row_height - 1));
}

SearchAppBar::SearchAppBar(ApplicationContext& context)
    : Material3Container(context),
      search_entry_(context),
      leading_(nullptr),
      trailing_{nullptr, nullptr},
      surface_state_(AppBarSurfaceState::kFlat) {
  attachChild(search_entry_);
  search_entry_.setOnInteractiveChange(
      [this]() { this->triggerInteractiveChange(); });
}

SearchAppBar::~SearchAppBar() {
  roo_windows::internal::ScrollConnectionRegistry::Disconnect(*this);
  for (Widget* slot : trailing_) {
    if (slot) detachChild(slot);
  }
  if (leading_) detachChild(leading_);
  detachChild(&search_entry_);
}

AppBarSurfaceState SearchAppBar::surfaceState() const {
  auto connection = internal::FindAppBarConnection(*this);
  return connection == nullptr
             ? surface_state_
             : (connection->scrolled() ? AppBarSurfaceState::kScrolled
                                       : AppBarSurfaceState::kFlat);
}

ScrollConnectionStatus SearchAppBar::setScrollBehavior(
    SimpleScrollablePanel& panel, AppBarScrollBehavior behavior) {
  auto previous = internal::FindAppBarConnection(*this);
  ScrollConnectionStatus status =
      internal::ConnectAppBar(context(), *this, panel, behavior, true);
  if (status == ScrollConnectionStatus::kSuccess) {
    auto connection = internal::FindAppBarConnection(*this);
    if (connection == previous) return status;
    // Resolve the compact font at attachment, not on a drag frame.
    (void)text_style_title_large();
    connection->measureHeight(Scaled(64), 0, HeightSpec::Unspecified(0));
    connection->onPositionChanged({}, panel.getScrollPosition(),
                                  ScrollSource::kProgrammatic);
    requestLayout();
  }
  return status;
}

ScrollConnectionStatus SearchAppBar::clearScrollBehavior() {
  return internal::ClearAppBarConnection(context(), *this);
}

bool SearchAppBar::hasScrollBehavior() const {
  return internal::FindAppBarConnection(*this) != nullptr;
}

void SearchAppBar::onAnimationFrame(AnimationTag tag,
                                    const AnimationSample& sample) {
  auto connection = internal::FindAppBarConnection(*this);
  if (tag == internal::AppBarScrollConnection::kSettle && connection != nullptr)
    connection->animate(sample);
  else
    Material3Container::onAnimationFrame(tag, sample);
}

void SearchAppBar::onPresentationChanged(const PresentationChange& change) {
  auto connection = internal::FindAppBarConnection(*this);
  if (connection != nullptr && (change.state != PresentationState::kPresented ||
                                change.detached_since_delivery))
    connection->suspend();
}

void SearchAppBar::setSurfaceState(AppBarSurfaceState state) {
  if (surface_state_ != state) {
    surface_state_ = state;
    search_entry_.setSurfaceState(state);
    invalidateInterior();
  }
}

void SearchAppBar::setDisplayText(roo::string_view text) {
  search_entry_.setDisplayText(text);
}

void SearchAppBar::replaceSlot(Widget*& slot, WidgetRef widget) {
  Widget* incoming = widget.get();
  if (incoming == slot) return;
  if (slot) detachChild(slot);
  slot = incoming;
  if (slot) {
    CHECK(slot->parent() == nullptr);
    attachChild(std::move(widget));
  }
  invalidateInterior();
  requestLayout();
}

void SearchAppBar::setLeading(WidgetRef widget) {
  replaceSlot(leading_, std::move(widget));
}

void SearchAppBar::setInnerTrailing(uint8_t index, WidgetRef widget) {
  search_entry_.setTrailing(index, std::move(widget));
}

void SearchAppBar::setTrailing(uint8_t index, WidgetRef widget) {
  CheckTrailingIndex(index);
  replaceSlot(trailing_[index], std::move(widget));
}

int SearchAppBar::getChildrenCount() const {
  return 1 + (leading_ != nullptr) + ChildCount(trailing_);
}

const Widget& SearchAppBar::getChild(int idx) const {
  if (idx-- == 0) return search_entry_;
  if (leading_ != nullptr && idx-- == 0) return *leading_;
  int n = ChildCount(trailing_);
  if (idx < n) return ChildAt(trailing_, idx, "SearchAppBar");
  LOG(FATAL) << "SearchAppBar child index out of bounds";
  return *leading_;
}

Widget& SearchAppBar::getChild(int idx) {
  if (idx-- == 0) return search_entry_;
  if (leading_ != nullptr && idx-- == 0) return *leading_;
  for (Widget* slot : trailing_) {
    if (slot && idx-- == 0) return *slot;
  }
  LOG(FATAL) << "SearchAppBar child index out of bounds";
  return *leading_;
}

ColorToken SearchAppBar::containerRole() const {
  const SearchAppBarTheme& app_bar =
      theme().material3Theme().components.searchAppBar;
  return surfaceState() == AppBarSurfaceState::kFlat
             ? app_bar.flatContainer
             : app_bar.scrolledContainer;
}

void SearchAppBar::EmbeddedSearchBar::setSurfaceState(
    AppBarSurfaceState state) {
  if (surface_state_ == state) return;
  surface_state_ = state;
  invalidateInterior();
}

::roo_windows::material3::ColorToken
SearchAppBar::EmbeddedSearchBar::containerRole() const {
  AppBarSurfaceState state =
      parent() == nullptr
          ? surface_state_
          : static_cast<const SearchAppBar*>(parent())->surfaceState();
  const SearchAppBarTheme& app_bar =
      theme().material3Theme().components.searchAppBar;
  return state == AppBarSurfaceState::kFlat ? app_bar.flatSearchContainer
                                            : app_bar.scrolledSearchContainer;
}

const internal::SearchEntryTokens&
SearchAppBar::EmbeddedSearchBar::entryTokens() const {
  return internal::kEmbeddedSearchEntryTokens;
}

Dimensions SearchAppBar::onMeasure(WidthSpec width, HeightSpec height) {
  const int16_t action_size = Scaled(internal::kActionTapTargetDp);
  const int16_t outer_height = Scaled(64);
  auto connection = internal::FindAppBarConnection(*this);
  const int16_t entry_height =
      Scaled(internal::kEmbeddedSearchEntryTokens.container_height_dp);
  const int16_t available_width = std::max<int16_t>(0, width.value());
  if (leading_)
    MeasureChildWithMargins(*leading_, WidthSpec::Exactly(action_size),
                            HeightSpec::Exactly(action_size));
  for (Widget* slot : trailing_) {
    if (slot)
      MeasureChildWithMargins(*slot, WidthSpec::Exactly(action_size),
                              HeightSpec::Exactly(action_size));
  }
  const int16_t outer_slots =
      (leading_ != nullptr) * action_size + ChildCount(trailing_) * action_size;
  const int16_t entry_width = std::min<int16_t>(
      std::max<int16_t>(0, available_width -
                               2 * Scaled(internal::kAppBarEdgeInsetDp) -
                               outer_slots),
      Scaled(internal::kEmbeddedSearchEntryTokens.max_width_dp));
  search_entry_.measure(WidthSpec::Exactly(entry_width),
                        HeightSpec::Exactly(entry_height));
  return Dimensions(width.resolveSize(available_width),
                    connection == nullptr
                        ? height.resolveSize(outer_height)
                        : connection->measureHeight(outer_height, 0, height));
}

void SearchAppBar::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  const int16_t edge = Scaled(internal::kAppBarEdgeInsetDp);
  const int16_t action_size = Scaled(internal::kActionTapTargetDp);
  const int16_t entry_height =
      Scaled(internal::kEmbeddedSearchEntryTokens.container_height_dp);
  const int16_t width = std::max<int16_t>(0, rect.width());
  auto connection = internal::FindAppBarConnection(*this);
  const int16_t height = std::max<int16_t>(
      0, rect.height() + (connection == nullptr ? 0 : connection->collapse()));
  const int16_t action_y = std::max<int16_t>(0, (height - action_size) / 2);
  int16_t left = std::min<int16_t>(edge, width);
  int16_t right = std::max<int16_t>(left, width - edge);
  auto layout_outer = [action_size, action_y](Widget* child, int16_t x) {
    if (child)
      LayoutChildWithMargins(*child, Rect(x, action_y, x + action_size - 1,
                                          action_y + action_size - 1));
  };
  if (leading_) {
    layout_outer(leading_, left);
    left = std::min<int16_t>(right, left + action_size);
  }
  for (int i = 1; i >= 0; --i) {
    if (!trailing_[i]) continue;
    right = std::max<int16_t>(left, right - action_size);
    layout_outer(trailing_[i], right);
  }
  const int16_t available = std::max<int16_t>(0, right - left);
  const int16_t lane_width = std::min<int16_t>(
      available, Scaled(internal::kEmbeddedSearchEntryTokens.max_width_dp));
  // When the cap leaves slack, retain the central strip's reading edge rather
  // than centering the entry and making it drift away from page content.
  const int16_t lane_left = left;
  const int16_t lane_top = std::max<int16_t>(0, (height - entry_height) / 2);
  search_entry_.layout(Rect(lane_left, lane_top, lane_left + lane_width - 1,
                            lane_top + entry_height - 1));
  auto scroll_connection = internal::FindAppBarConnection(*this);
  if (scroll_connection != nullptr && scroll_connection->collapse() > 0) {
    for (int i = 0; i < getChildrenCount(); ++i) {
      Widget& child = getChild(i);
      child.layout(
          child.parent_bounds().translate(0, -scroll_connection->collapse()));
    }
  }
}

bool SearchAppBar::fillTouchTargetPath(XDim x, YDim y,
                                       std::vector<Widget*>& path) {
  if (!isVisible() || !isEnabled() || !bounds().contains(x, y)) return false;
  path.push_back(this);
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible() || !child.isEnabled() ||
        !child.parent_bounds().contains(x, y)) {
      continue;
    }
    if (AppendInteractiveChildTouchTarget(child, x, y, false, path)) {
      return true;
    }
  }
  path.pop_back();
  return false;
}

bool SearchAppBar::fillSloppyTouchTargetPath(XDim x, YDim y,
                                             std::vector<Widget*>& path) {
  if (!isVisible() || !isEnabled() || !getSloppyTouchBounds().contains(x, y)) {
    return false;
  }
  path.push_back(this);
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible() || !child.isEnabled() ||
        !child.getMaxSloppyTouchParentBounds().contains(x, y)) {
      continue;
    }
    const bool within_bounds = child.parent_bounds().contains(x, y);
    if (AppendInteractiveChildTouchTarget(child, x, y, !within_bounds, path)) {
      return true;
    }
  }
  path.pop_back();
  return false;
}

}  // namespace roo_windows::material3
