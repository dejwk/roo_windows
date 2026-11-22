#include <algorithm>
#include <cmath>
#include <cstdio>
#include <new>

#include "roo_display.h"
#include "roo_display/shape/smooth.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_icons.h"
#include "roo_icons/filled/24/action.h"
#include "roo_icons/filled/24/editor.h"
#include "roo_icons/filled/24/navigation.h"
#include "roo_windows/core/child_layout.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/button/internal/button_geometry.h"
#include "roo_windows/material3/date_picker/date_picker_internal.h"
#include "roo_windows/material3/internal/density.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3::internal {
using roo_time::CivilDay;
namespace {
constexpr int kHeader = Scaled(112);
constexpr int kFooter = Scaled(64);

/// Resolves nominal calendar cells from the live calendar density policy.
int CellSize(const DatePickerPanel& panel) {
  return Scaled(48 + 4 * ResolveDensityLevel(panel.session().density()));
}

int InsetSize(const DatePickerPanel& panel) {
  const int inset =
      Scaled(16 + 2 * ResolveDensityLevel(panel.session().density()));
  // Floating chrome must stay inside the opaque interior of the 28 dp corners.
  return panel.getBorderStyle().hasRoundedCorners()
             ? std::max(inset, Scaled(16))
             : inset;
}

/// Resolves foreground plus background in one draw, then excludes settled
/// pixels.
void PaintText(PaintContext& ctx, roo::string_view text, const TextStyle& style,
               Color color, Color background, const Rect& bounds,
               roo_display::Alignment alignment = roo_display::kCenter |
                                                  roo_display::kMiddle) {
  if (!ctx.localClip().intersects(bounds)) return;
  roo_display::StringViewLabel label(text, style.font(), color,
                                     style.fontOptions());
  PaintContext local = ctx.clipped(bounds);
  local.setBgcolor(background);
  local.drawTiled(label, bounds, alignment);
  ctx.addExclusion(bounds);
}

/// Composes a rounded marker with its inherited surface in bounded scratch.
class CellBackground final : public roo_display::Rasterizable {
 public:
  CellBackground(const Rect& cell, const Rect& marker, Color surface,
                 Color fill, Color outline, int outline_width,
                 const PressOverlay* press = nullptr)
      : cell_(cell.asBox()),
        surface_(surface),
        press_(press),
        marker_(roo_display::SmoothFilledRoundRect(
            marker.xMin(), marker.yMin(), marker.xMax(), marker.yMax(),
            marker.height() * 0.5f, fill)),
        outline_(roo_display::SmoothThickRoundRect(
            marker.xMin(), marker.yMin(), marker.xMax(), marker.yMax(),
            marker.height() * 0.5f, outline_width, outline)) {}

  roo_display::Box extents() const override { return cell_; }

  void readColors(const int16_t* x, const int16_t* y, uint32_t count,
                  Color* result) const override {
    Color fill[16];
    Color outline[16];
    while (count != 0) {
      const uint32_t batch = std::min<uint32_t>(count, 16);
      marker_.readColorsMaybeOutOfBounds(x, y, batch, fill);
      outline_.readColorsMaybeOutOfBounds(x, y, batch, outline);
      for (uint32_t i = 0; i < batch; ++i) {
        result[i] = AlphaBlend(AlphaBlend(surface_, fill[i]), outline[i]);
        if (press_ != nullptr) {
          Color ink = press_->get(x[i], y[i]);
          ink.set_a(static_cast<uint8_t>(ink.a() * fill[i].a() / 255));
          result[i] = AlphaBlend(result[i], ink);
        }
      }
      x += batch;
      y += batch;
      result += batch;
      count -= batch;
    }
  }

 private:
  roo_display::Box cell_;
  Color surface_;
  const PressOverlay* press_;
  roo_display::SmoothShape marker_;
  roo_display::SmoothShape outline_;
};

/// Chooses legible compact text without retaining per-cell font state.
const TextStyle& CellTextStyle(const Rect& cell, int nominal_cell) {
  if (cell.height() >= nominal_cell) return text_style_body_medium();
  return cell.height() < Scaled(18)   ? text_style_label_small()
         : cell.height() < Scaled(24) ? text_style_body_small()
                                      : text_style_body_medium();
}

/// Blends selection colors using the shared click's retained sample.
Color SelectionColor(Color from, Color to, float amount) {
  return AlphaBlend(from, to.withA(static_cast<uint8_t>(255 * amount)));
}

/// Keeps borrowed foreground and background alive through a synchronous draw.
void PaintComposedControl(PaintContext& ctx,
                          const roo_display::Drawable& foreground,
                          const Rect& bounds, const Rect& marker,
                          Color background, Color fill, Color outline,
                          int outline_width, const PressOverlay* press) {
  const CellBackground composed(bounds, marker, background, fill, outline,
                                outline_width, press);
  roo_display::Tile tile(&foreground, bounds.asBox(),
                         roo_display::kCenter | roo_display::kMiddle);
  tile.setBackground(&composed);
  // Short bands keep Tile's background filter from allocating raster streams.
  // The borrowed sources stay alive through this synchronous draw only.
  for (int y = bounds.yMin(); y <= bounds.yMax(); y += 8) {
    PaintContext band = ctx.clipped(Rect(bounds.xMin(), y, bounds.xMax(),
                                         std::min<int>(y + 7, bounds.yMax())));
    band.drawObject(tile);
  }
  ctx.addExclusion(bounds);
}

/// Samples bounded ripple/fade feedback without reading time from paint.
PressOverlay MakePressFeedback(const Widget& widget, const Rect& bounds,
                               Color ink) {
  const ClickAnimation* animation = widget.getClickAnimation();
  const float progress = animation == nullptr ? 1.0f : animation->progress();
  const int x = animation == nullptr ? bounds.xMin() + bounds.width() / 2
                                     : animation->xCenter();
  const int y = animation == nullptr ? bounds.yMin() + bounds.height() / 2
                                     : animation->yCenter();
  const int dx = std::max(x - bounds.xMin(), bounds.xMax() - x);
  const int dy = std::max(y - bounds.yMin(), bounds.yMax() - y);
  const bool fade = animation != nullptr &&
                    animation->reveal() == ClickOverlayAnimation::kFade;
  const float reveal =
      fade ? 1.0f : 1.0f - (1.0f - progress) * (1.0f - progress);
  const float opacity =
      widget.isPressed() ? (fade ? progress : 1.0f) : 1.0f - progress;
  ink.set_a(static_cast<uint8_t>(
      widget.interactionOpacity(InteractionState::kPressed) * opacity));
  return PressOverlay(
      x, y, static_cast<int>(std::sqrt(dx * dx + dy * dy) * reveal) + 1, ink);
}

/// Resolves glyphs, rounded markers, and sampled feedback in one tile.
void PaintCell(PaintContext& ctx, roo::string_view text, const Rect& cell,
               Color foreground, Color background, Color accent,
               float selection, bool today, bool focused, int nominal_cell,
               const PressOverlay* press) {
  const TextStyle& style = CellTextStyle(cell, nominal_cell);
  if (selection == 0.0f && !today && !focused && press == nullptr) {
    PaintText(ctx, text, style, foreground, background, cell);
    return;
  }
  roo_display::StringViewLabel label(text, style.font(), foreground,
                                     style.fontOptions());
  const int marker_height =
      std::min(nominal_cell - Scaled(8), cell.height() - Scaled(2));
  const int marker_width =
      std::min(cell.width() - Scaled(2),
               std::max(marker_height, label.extents().width() + Scaled(6)));
  const int left = cell.xMin() + (cell.width() - marker_width) / 2;
  const int top = cell.yMin() + (cell.height() - marker_height) / 2;
  const Rect marker(left, top, left + marker_width - 1,
                    top + marker_height - 1);
  PaintComposedControl(
      ctx, label, cell, marker, background,
      SelectionColor(background, accent, selection),
      today || focused ? accent : roo_display::color::Transparent,
      focused ? Scaled(2) : Scaled(1), press);
}

/// Keeps header click feedback inside its own circular or pill-shaped control.
void PaintHeaderControl(PaintContext& ctx, const Widget& widget,
                        const roo_display::Drawable& content,
                        const Rect& bounds, Color foreground, Color background,
                        bool wide) {
  const int h = std::min(Scaled(40), bounds.height() - Scaled(4));
  const int w = wide ? bounds.width() - Scaled(4) : h;
  const int x = bounds.xMin() + (bounds.width() - w) / 2;
  const int y = bounds.yMin() + (bounds.height() - h) / 2;
  const PressOverlay press = MakePressFeedback(widget, bounds, foreground);
  PaintComposedControl(ctx, content, bounds, Rect(x, y, x + w - 1, y + h - 1),
                       background, background, roo_display::color::Transparent,
                       0, &press);
}

/// Makes room for seven compact calendar rows on short displays.
int HeaderHeight(const DatePickerPanel& panel, bool shrink, int height) {
  const int level = ResolveDensityLevel(panel.session().density());
  return shrink && height < Scaled(320) ? Scaled(80 + 2 * level)
                                        : kHeader + Scaled(4 * level);
}

int FooterHeight(const DatePickerPanel& panel, bool shrink, int height) {
  const int level = ResolveDensityLevel(panel.session().density());
  return (shrink && height < Scaled(320) ? Scaled(48) : kFooter) +
         Scaled(4 * level);
}

int BottomInset(const DatePickerPanel& panel, bool shrink, int height) {
  return shrink && height < Scaled(320) ? InsetSize(panel) / 2
                                        : InsetSize(panel);
}

}  // namespace

DatePickerHeader::DatePickerHeader(ApplicationContext& context,
                                   DatePickerPanel& panel)
    : Widget(context),
      panel_(panel),
      focused_control_(0),
      navigation_style_(0),
      abbreviated_months_(0) {}

Rect DatePickerHeader::controlBounds(int control) const {
  const int cell_size = CellSize(panel_);
  const int navigation_height =
      std::min(cell_size, std::max(Scaled(24), height() - Scaled(40)));
  if (control == 4)
    return Rect(width() - cell_size, 0, width() - 1,
                std::min(cell_size, height() - navigation_height) - 1);
  int y = height() - navigation_height;
  if (control == 0) return Rect(0, y, cell_size - 1, height() - 1);
  if (control == 3)
    return Rect(width() - cell_size, y, width() - 1, height() - 1);
  return control == 1
             ? Rect(cell_size, y, cell_size + month_width_ - 1, height() - 1)
             : Rect(cell_size + month_width_, y, width() - cell_size - 1,
                    height() - 1);
}

const TextStyle& DatePickerHeader::navigationTextStyle() const {
  return navigation_style_ == 0   ? text_style_label_large()
         : navigation_style_ == 1 ? text_style_label_medium()
                                  : text_style_label_small();
}

Dimensions DatePickerHeader::getSuggestedMinimumDimensions() const {
  return Dimensions(2 * CellSize(panel_) + Scaled(112),
                    HeaderHeight(panel_, false, 0));
}

roo::string_view DatePickerHeader::monthLabel(int month) const {
  const roo_locale::CalendarLocale& locale = panel_.session().locale();
  return abbreviated_months_ != 0 ? locale.month_standalone_abbreviated[month]
                                  : locale.month_standalone_wide[month];
}

Dimensions DatePickerHeader::onMeasure(WidthSpec width, HeightSpec height) {
  const int cell_size = CellSize(panel_);
  const int w = width.resolveSize(getSuggestedMinimumDimensions().width());
  const int available = w - 2 * cell_size;
  int year_width = 0;
  // Prefer abbreviated locale forms to reducing the navigation typography.
  // Measure all months so the hit targets and format stay stable on navigation.
  for (navigation_style_ = 0; navigation_style_ < 3; ++navigation_style_) {
    const TextStyle& style = navigationTextStyle();
    roo_display::StringViewLabel year("9999", style.font(), Color(),
                                      style.fontOptions());
    year_width = std::max(Scaled(32), year.anchorExtents().width() + Scaled(8));
    for (int abbreviated = 0; abbreviated < 2; ++abbreviated) {
      abbreviated_months_ = abbreviated;
      int month_width = 0;
      for (int month = 0; month < 12; ++month) {
        roo_display::StringViewLabel label(monthLabel(month), style.font(),
                                           Color(), style.fontOptions());
        month_width = std::max<int>(month_width, label.anchorExtents().width());
      }
      if (month_width + year_width + Scaled(4) <= available) {
        month_width_ = std::max(1, available - year_width);
        return Dimensions(
            w, height.resolveSize(getSuggestedMinimumDimensions().height()));
      }
    }
  }
  navigation_style_ = 2;
  abbreviated_months_ = 1;
  month_width_ = std::max(1, available - year_width);
  return Dimensions(
      w, height.resolveSize(getSuggestedMinimumDimensions().height()));
}

void DatePickerHeader::dirtyPart(int part) {
  dirty_parts_ |= 1 << part;
  setDirty(part == 5 ? Rect(0, 0, width() - CellSize(panel_) - 1,
                            controlBounds(0).yMin() - 1)
                     : controlBounds(part));
}

void DatePickerHeader::paint(PaintContext& ctx) const {
  const int cell_size = CellSize(panel_);
  const DatePickerSession& session = panel_.session();
  const auto& colors = theme().material3Theme().color;
  Color bg = panel_.background();
  bool input = panel_.mode() == DatePickerMode::kInput;
  const int navigation_top = controlBounds(0).yMin();
  const int title_height =
      navigation_top < Scaled(56) ? Scaled(16) : Scaled(24);
  if (isInvalidated())
    PaintText(ctx,
              input ? session.strings().headline_input_date
                    : session.strings().headline_select_date,
              text_style_label_large(), colors.onSurfaceVariant, bg,
              Rect(0, 0, width() - cell_size - 1, title_height - 1),
              roo_display::kLeft | roo_display::kMiddle);
  if (isInvalidated() || (dirty_parts_ & (1 << 5))) {
    char text[64] = {};
    if (session.draft.isValid())
      session.codec().format(session.draft, text, sizeof(text));
    PaintText(
        ctx,
        session.draft.isValid() ? roo::string_view(text)
                                : roo::string_view("—"),
        navigation_top < Scaled(56) ? text_style_title_large()
                                    : text_style_headline_small(),
        colors.onSurface, bg,
        Rect(0, title_height, width() - cell_size - 1, navigation_top - 1),
        roo_display::kLeft | roo_display::kMiddle);
  }
  for (int control = 0; control < 5; ++control) {
    if (!isInvalidated() && !(dirty_parts_ & (1 << control))) continue;
    Rect bounds = controlBounds(control);
    Color foreground = isFocused() && focused_control_ == control
                           ? colors.primary
                           : colors.onSurface;
    const bool feedback = control == focused_control_ &&
                          (getClickAnimation() != nullptr || isPressed());
    if (control == 0 || control == 3 || control == 4) {
      MonoIcon icon = control == 0 ? ic_filled_24_navigation_chevron_left()
                      : control == 3
                          ? ic_filled_24_navigation_chevron_right()
                          : (input ? ic_filled_24_action_calendar_today()
                                   : ic_filled_24_editor_mode_edit());
      icon.color_mode().setColor(foreground);
      if (feedback) {
        PaintHeaderControl(ctx, *this, icon, bounds, foreground, bg, false);
      } else {
        PaintContext local = ctx.clipped(bounds);
        local.setBgcolor(bg);
        local.drawTiled(icon, bounds,
                        roo_display::kCenter | roo_display::kMiddle);
        ctx.addExclusion(bounds);
      }
    } else {
      roo::string_view label;
      char year[16];
      if (control == 1) {
        label = monthLabel(session.month.month() - 1);
      } else {
        std::snprintf(year, sizeof(year), "%d", session.month.year());
        label = year;
      }
      if (feedback) {
        roo_display::StringViewLabel text(label, navigationTextStyle().font(),
                                          foreground,
                                          navigationTextStyle().fontOptions());
        PaintHeaderControl(ctx, *this, text, bounds, foreground, bg, true);
      } else {
        PaintText(ctx, label, navigationTextStyle(), foreground, bg, bounds);
      }
    }
  }
  dirty_parts_ = 0;
}

int DatePickerHeader::controlAt(XDim x, YDim y) const {
  for (int control = 0; control < 5; ++control) {
    if (controlBounds(control).contains(x, y)) return control;
  }
  return -1;
}

Rect DatePickerHeader::getClickFeedbackBounds() const {
  return controlBounds(focused_control_);
}

void DatePickerHeader::onShowPress(XDim x, YDim y) {
  const int control = controlAt(x, y);
  if (control < 0) return;
  panel_.prepareInteraction(*this);
  if (isClicking() && !isPressed()) onCancel();
  dirtyPart(focused_control_);
  focused_control_ = control;
  dirtyPart(focused_control_);
  requestFocus();
  Widget::onShowPress(x, y);
}

void DatePickerHeader::onSingleTapUp(XDim x, YDim y) {
  const int control = controlAt(x, y);
  if (control < 0 || (isPressed() && control != focused_control_)) {
    onCancel();
    return;
  }
  panel_.prepareInteraction(*this);
  if (isClicking() && !isPressed()) onCancel();
  dirtyPart(focused_control_);
  focused_control_ = control;
  dirtyPart(focused_control_);
  requestFocus();
  Widget::onSingleTapUp(x, y);
}

void DatePickerHeader::onClicked() { panel_.activateHeader(focused_control_); }

bool DatePickerHeader::onKeyEvent(const KeyEvent& event) {
  if (event.phase != KeyPhase::kDown && event.phase != KeyPhase::kRepeat)
    return false;
  if (event.code == KeyCode::kLeft || event.code == KeyCode::kRight) {
    onCancel();
    dirtyPart(focused_control_);
    focused_control_ =
        (focused_control_ + (event.code == KeyCode::kLeft ? 4 : 1)) % 5;
    dirtyPart(focused_control_);
    return true;
  }
  if (event.code == KeyCode::kEnter || event.code == KeyCode::kSpace) {
    const Rect control = controlBounds(focused_control_);
    onSingleTapUp(control.xMin() + control.width() / 2,
                  control.yMin() + control.height() / 2);
    return true;
  }
  return false;
}

DatePickerBody::DatePickerBody(ApplicationContext& context,
                               DatePickerPanel& panel)
    : SurfaceWidget(context),
      panel_(panel),
      painted_focused_(false),
      abbreviated_months_(false) {}

Color DatePickerBody::background() const { return panel_.background(); }

int DatePickerBody::cellCount() const {
  return panel_.mode() == DatePickerMode::kDays ? 42
         : panel_.mode() == DatePickerMode::kMonths
             ? 12
             : std::min(120, 10000 - panel_.firstYear());
}

Dimensions DatePickerBody::getSuggestedMinimumDimensions() const {
  const int cell_size = CellSize(panel_);
  return Dimensions(7 * cell_size, panel_.mode() == DatePickerMode::kDays
                                       ? 7 * cell_size
                                       : ((cellCount() + 2) / 3) * cell_size);
}

Dimensions DatePickerBody::onMeasure(WidthSpec width, HeightSpec height) {
  const Dimensions desired = getSuggestedMinimumDimensions();
  int w = desired.width();
  int h = desired.height();
  if (panel_.session().shrinkToFit()) {
    if (width.value() > 0) w = width.value();
    if (height.value() > 0 && panel_.mode() != DatePickerMode::kYears)
      h = height.value();
  }
  abbreviated_months_ = false;
  if (panel_.mode() == DatePickerMode::kMonths) {
    const TextStyle& style =
        CellTextStyle(Rect(0, 0, w / 3 - 1, h / 4 - 1), CellSize(panel_));
    for (int month = 0; month < 12; ++month) {
      roo_display::StringViewLabel label(
          panel_.session().locale().month_standalone_wide[month], style.font(),
          Color(), style.fontOptions());
      if (label.anchorExtents().width() + Scaled(8) > w / 3)
        abbreviated_months_ = true;
    }
  }
  return Dimensions(width.resolveSize(w), height.resolveSize(h));
}

Rect DatePickerBody::cellBounds(int index) const {
  const bool days = panel_.mode() == DatePickerMode::kDays;
  const int columns = days ? 7 : 3;
  const int column = index % columns;
  const int row = index / columns + (days ? 1 : 0);
  const int rows = days ? 7 : (cellCount() + 2) / 3;
  return Rect(column * width() / columns, row * height() / rows,
              (column + 1) * width() / columns - 1,
              (row + 1) * height() / rows - 1);
}

void DatePickerBody::paint(PaintContext& ctx) const {
  const DatePickerSession& session = panel_.session();
  const auto& colors = theme().material3Theme().color;
  bool days = panel_.mode() == DatePickerMode::kDays;
  const ClickAnimation* animation = getClickAnimation();
  const float raw_progress =
      animation == nullptr ? 1.0f : animation->progress();
  const float progress =
      raw_progress * raw_progress * (3.0f - 2.0f * raw_progress);
  const bool transition =
      days && animation != nullptr && transition_from_ != -2;

  if (days && isInvalidated()) {
    for (int col = 0; col < 7; ++col) {
      const Rect first = cellBounds(col);
      PaintText(
          ctx,
          session.locale().weekday_narrow[(col + session.firstWeekday()) % 7],
          CellTextStyle(first, CellSize(panel_)), colors.onSurfaceVariant,
          background(), Rect(first.xMin(), 0, first.xMax(), first.yMin() - 1));
    }
  }
  for (int i = 0; i < cellCount(); ++i) {
    Rect cell = cellBounds(i);
    if (!ctx.localClip().intersects(cell)) continue;
    // External invalidation repaints all exposed pixels. A state-only repaint
    // needs just cells whose selection or keyboard marker changed since paint.
    bool old_selected = false;
    bool new_selected = false;
    if (days) {
      CivilDay day = GridDay(session.month, session.firstWeekday(), i);
      old_selected = day.isValid() && day == painted_draft_;
      new_selected = day.isValid() && day == session.draft;
    }
    if (!isInvalidated() && old_selected == new_selected &&
        (painted_focused_ && painted_cursor_ == i) ==
            (isFocused() && cursor_ == i))
      continue;
    char number[12];
    roo::string_view text;
    bool enabled = true;
    bool selected = false;
    bool today = false;
    if (days) {
      CivilDay day = GridDay(session.month, session.firstWeekday(), i);
      if (!day.isValid()) continue;
      enabled = day.month() == session.month.month() && session.enabled(day);
      selected = day == session.draft;
      today = day == session.today();
      std::snprintf(number, sizeof(number), "%d", day.day());
      text = number;
    } else if (panel_.mode() == DatePickerMode::kMonths) {
      text = abbreviated_months_
                 ? session.locale().month_standalone_abbreviated[i]
                 : session.locale().month_standalone_wide[i];
      selected = i + 1 == session.month.month();
    } else {
      std::snprintf(number, sizeof(number), "%d", panel_.firstYear() + i);
      text = number;
      selected = panel_.firstYear() + i == session.month.year();
    }
    float selection = selected ? 1.0f : 0.0f;
    if (transition) {
      if (selected)
        selection = progress;
      else if (i == transition_from_)
        selection = 1.0f - progress;
    }
    Color color = today ? colors.primary : colors.onSurface;
    if (!enabled)
      color = AlphaBlend(background(), colors.onSurfaceVariant.withA(97));
    color = SelectionColor(color, colors.onPrimary, selection);
    const bool feedback = i == cursor_ && (animation != nullptr || isPressed());
    const PressOverlay press =
        feedback ? MakePressFeedback(*this, cell, color) : PressOverlay();
    PaintContext cell_ctx = ctx.clipped(cell);
    PaintCell(cell_ctx, text, cell, color, background(), colors.primary,
              selection, today, isFocused() && cursor_ == i, CellSize(panel_),
              feedback ? &press : nullptr);
    cell_ctx.clear();
    ctx.addExclusion(cell);
  }
  if (isInvalidated()) ctx.clear();
  painted_draft_ = session.draft;
  painted_cursor_ = cursor_;
  painted_focused_ = isFocused();
}

void DatePickerBody::activate(int index) {
  if (index < 0 || index >= cellCount()) return;
  DatePickerSession& session = panel_.session();
  if (panel_.mode() == DatePickerMode::kDays) {
    CivilDay day = GridDay(session.month, session.firstWeekday(), index);
    if (day.isValid() && day.month() == session.month.month())
      panel_.selectDate(day);
  } else if (panel_.mode() == DatePickerMode::kMonths) {
    panel_.selectMonth(session.month.year(), index + 1);
  } else {
    panel_.selectMonth(panel_.firstYear() + index, session.month.month());
  }
}

int DatePickerBody::cellAt(XDim x, YDim y) const {
  const bool days = panel_.mode() == DatePickerMode::kDays;
  const int columns = days ? 7 : 3;
  const int rows = days ? 7 : (cellCount() + 2) / 3;
  if (x < 0 || y < 0 || x >= width() || y >= height()) return -1;
  const int row = ((y + 1) * rows - 1) / height() - (days ? 1 : 0);
  if (row < 0) return -1;
  const int index = row * columns + ((x + 1) * columns - 1) / width();
  return index < cellCount() ? index : -1;
}

bool DatePickerBody::cellEnabled(int index) const {
  if (index < 0 || index >= cellCount()) return false;
  if (panel_.mode() != DatePickerMode::kDays) return true;
  const DatePickerSession& session = panel_.session();
  const CivilDay day = GridDay(session.month, session.firstWeekday(), index);
  return day.isValid() && day.month() == session.month.month() &&
         session.enabled(day);
}

void DatePickerBody::dirtyCell(int index) {
  if (index >= 0 && index < cellCount()) setDirty(cellBounds(index));
}

int DatePickerBody::selectedCell(CivilDay day) const {
  const DatePickerSession& session = panel_.session();
  if (!day.isValid() || MonthStart(day) != session.month) return -1;
  return (7 + session.month.dayOfWeek() - session.firstWeekday()) % 7 +
         day.day() - 1;
}

Rect DatePickerBody::getClickFeedbackBounds() const {
  return cursor_ < 0 ? Rect(0, 0, -1, -1) : cellBounds(cursor_);
}

void DatePickerBody::onShowPress(XDim x, YDim y) {
  const int index = cellAt(x, y);
  if (!cellEnabled(index)) return;
  panel_.prepareInteraction(*this);
  if (isClicking() && !isPressed()) onCancel();
  transition_from_ = -2;
  dirtyCell(cursor_);
  cursor_ = index;
  dirtyCell(cursor_);
  requestFocus();
  Widget::onShowPress(x, y);
}

void DatePickerBody::onSingleTapUp(XDim x, YDim y) {
  const int index = cellAt(x, y);
  if (!cellEnabled(index) || (isPressed() && index != cursor_)) {
    onCancel();
    return;
  }
  panel_.prepareInteraction(*this);
  if (isClicking() && !isPressed()) onCancel();
  dirtyCell(cursor_);
  cursor_ = index;
  if (panel_.mode() == DatePickerMode::kDays) {
    const CivilDay day =
        GridDay(panel_.session().month, panel_.session().firstWeekday(), index);
    transition_from_ = day == panel_.session().draft
                           ? -2
                           : selectedCell(panel_.session().draft);
  }
  dirtyCell(cursor_);
  requestFocus();
  Widget::onSingleTapUp(x, y);
  if (getClickAnimation() == nullptr) onClickAnimationChanged(-1.0f);
}

void DatePickerBody::onClicked() { activate(cursor_); }

void DatePickerBody::onClickAnimationChanged(float) {
  // Keep the departing and arriving cells as separate damage rectangles, so
  // distant selections do not repaint the unchanged dates between them.
  if (transition_from_ >= 0) invalidateInterior(cellBounds(transition_from_));
  if (getClickAnimation() == nullptr) transition_from_ = -2;
}

void DatePickerBody::resetCursor() {
  dirtyCell(cursor_);
  dirtyCell(painted_cursor_);
  dirtyCell(selectedCell(painted_draft_));
  const DatePickerSession& session = panel_.session();
  if (panel_.mode() == DatePickerMode::kDays) {
    int offset = (7 + session.month.dayOfWeek() - session.firstWeekday()) % 7;
    cursor_ = offset + ((session.draft.isValid() &&
                         MonthStart(session.draft) == session.month)
                            ? session.draft.day() - 1
                            : 0);
    if (!cellEnabled(cursor_)) {
      cursor_ = 0;
      while (cursor_ < cellCount() && !cellEnabled(cursor_)) ++cursor_;
      if (cursor_ == cellCount()) cursor_ = -1;
    }
  } else if (panel_.mode() == DatePickerMode::kMonths) {
    cursor_ = session.month.month() - 1;
  } else {
    cursor_ = std::max(0, std::min(cellCount() - 1,
                                   session.month.year() - panel_.firstYear()));
  }
  dirtyCell(cursor_);
}

void DatePickerBody::revealCursor() {
  if (cursor_ >= 0) panel_.scrollTo(cellBounds(cursor_));
}

bool DatePickerBody::onKeyEvent(const KeyEvent& event) {
  if (event.phase != KeyPhase::kDown && event.phase != KeyPhase::kRepeat)
    return false;
  const int previous_cursor = cursor_;
  int columns = panel_.mode() == DatePickerMode::kDays ? 7 : 3;
  int next = cursor_;
  switch (event.code) {
    case KeyCode::kPageUp:
    case KeyCode::kPageDown:
      panel_.activateHeader(event.code == KeyCode::kPageUp ? 0 : 3);
      return true;
    case KeyCode::kLeft:
      --next;
      break;
    case KeyCode::kRight:
      ++next;
      break;
    case KeyCode::kUp:
      next -= columns;
      break;
    case KeyCode::kDown:
      next += columns;
      break;
    case KeyCode::kHome:
      next = 0;
      break;
    case KeyCode::kEnd:
      next = cellCount() - 1;
      break;
    case KeyCode::kEnter:
    case KeyCode::kSpace:
      if (cellEnabled(cursor_)) {
        const Rect cell = cellBounds(cursor_);
        onSingleTapUp(cell.xMin() + cell.width() / 2,
                      cell.yMin() + cell.height() / 2);
      }
      return true;
    default:
      return false;
  }
  onCancel();
  dirtyCell(previous_cursor);
  cursor_ = std::max(0, std::min(cellCount() - 1, next));
  revealCursor();
  dirtyCell(cursor_);
  return true;
}

DatePickerPanel::Action::Action(ApplicationContext& context,
                                DatePickerPanel& panel, bool confirm)
    : Button(context,
             confirm ? panel.session().strings().ok_label
                     : panel.session().strings().cancel_label,
             ButtonVariant::kText),
      panel_(panel),
      confirm_(confirm) {}

Padding DatePickerPanel::Action::getPadding() const {
  const int8_t level = ResolveDensityLevel(panel_.session().density());
  const ButtonContentMetrics metrics =
      ResolveButtonContentMetrics(label(), icon(), size(), level);
  return ResolveButtonPadding(size(), smallButtonPadding(),
                              metrics.content_height, level);
}

void DatePickerPanel::Action::onShowPress(XDim x, YDim y) {
  panel_.prepareInteraction(*this);
  Button::onShowPress(x, y);
}

void DatePickerPanel::Action::onSingleTapUp(XDim x, YDim y) {
  panel_.prepareInteraction(*this);
  Button::onSingleTapUp(x, y);
}

void DatePickerPanel::Action::onClicked() {
  if (confirm_)
    panel_.session().accept();
  else {
    panel_.session().dismiss_reason = DatePickerDismissReason::kCancel;
    panel_.session().finish(PresentationFinishReason::kCancel);
  }
}

DatePickerPanel::Input::Input(ApplicationContext& context,
                              DatePickerPanel& panel)
    : TextField(context, panel.session().codec().placeholder(),
                TextFieldVariant::kOutlined),
      panel_(panel) {}

DensityOverride DatePickerPanel::Input::densityOverride() const {
  return DensityOverride::Explicit(panel_.session().density());
}

DatePickerPanel::DatePickerPanel(ApplicationContext& context,
                                 DatePickerSession& session)
    : Material3Container(context),
      session_(session),
      header_(context, *this),
      body_(context, *this),
      viewport_(context, body_),
      cancel_(context, *this, false),
      confirm_(context, *this, true),
      full_screen_(false),
      syncing_input_(false),
      reveal_pending_(true),
      focus_pending_(false) {
  attachChild(header_);
  attachChild(viewport_);
  attachChild(cancel_);
  attachChild(confirm_);
}

DatePickerPanel::~DatePickerPanel() {
  if (input_) detachChild(input_.get());
  detachChild(&confirm_);
  detachChild(&cancel_);
  detachChild(&viewport_);
  detachChild(&header_);
  viewport_.clearContents();
}

Widget* DatePickerPanel::preferredFocusChild() {
  return mode_ == DatePickerMode::kInput ? static_cast<Widget*>(input_.get())
                                         : static_cast<Widget*>(&body_);
}

ColorToken DatePickerPanel::containerRole() const {
  return theme().material3Theme().components.datePicker.container;
}

BorderStyle DatePickerPanel::getBorderStyle() const {
  return BorderStyle(full_screen_ ? 0 : Scaled(28), 0);
}

Dimensions DatePickerPanel::getSuggestedMinimumDimensions() const {
  const int cell_size = CellSize(*this);
  const int inset = InsetSize(*this);
  return Dimensions(7 * cell_size + 2 * inset,
                    HeaderHeight(*this, false, 0) + 7 * cell_size +
                        FooterHeight(*this, false, 0) + inset);
}

Dimensions DatePickerPanel::minimumViewportDimensions() const {
  const int span = Scaled(240 + 8 * ResolveDensityLevel(session_.density()));
  return Dimensions(span, span);
}

Widget& DatePickerPanel::getChild(int index) {
  switch (index) {
    case 0:
      return header_;
    case 1:
      return viewport_;
    case 2:
      return cancel_;
    case 3:
      return confirm_;
    default:
      return *input_;
  }
}

const Widget& DatePickerPanel::getChild(int index) const {
  switch (index) {
    case 0:
      return header_;
    case 1:
      return viewport_;
    case 2:
      return cancel_;
    case 3:
      return confirm_;
    default:
      return *input_;
  }
}

Dimensions DatePickerPanel::onMeasure(WidthSpec width, HeightSpec height) {
  const int cell_size = CellSize(*this);
  const int inset = InsetSize(*this);
  Dimensions desired = getSuggestedMinimumDimensions();
  int w = width.resolveSize(desired.width());
  int h = height.resolveSize(desired.height());
  const bool shrink = session_.shrinkToFit();
  const int header_height = HeaderHeight(*this, shrink, h);
  header_.measure(WidthSpec::Exactly(w - 2 * inset),
                  HeightSpec::Exactly(header_height - inset / 2));
  viewport_.measure(
      WidthSpec::Exactly(w - 2 * inset),
      HeightSpec::Exactly(h - header_height - FooterHeight(*this, shrink, h) -
                          BottomInset(*this, shrink, h)));
  MeasureChildWithMargins(cancel_, WidthSpec::Exactly(Scaled(96)),
                          HeightSpec::Exactly(cell_size));
  MeasureChildWithMargins(confirm_, WidthSpec::Exactly(Scaled(64)),
                          HeightSpec::Exactly(cell_size));
  if (input_)
    input_->measure(WidthSpec::Exactly(w - 2 * inset),
                    HeightSpec::Exactly(Scaled(80)));
  return Dimensions(w, h);
}

void DatePickerPanel::onLayout(bool, const Rect&) {
  const int inset = InsetSize(*this);
  const bool shrink = session_.shrinkToFit();
  const int header_height = HeaderHeight(*this, shrink, height());
  const int footer_height = FooterHeight(*this, shrink, height());
  const int bottom_inset = BottomInset(*this, shrink, height());
  header_.layout(
      Rect(inset, inset / 2, width() - inset - 1, header_height - 1));
  viewport_.layout(Rect(inset, header_height, width() - inset - 1,
                        height() - footer_height - bottom_inset - 1));
  LayoutChildWithMargins(
      cancel_,
      Rect(width() - inset - Scaled(160), height() - footer_height,
           width() - inset - Scaled(64) - 1, height() - bottom_inset - 1));
  LayoutChildWithMargins(
      confirm_, Rect(width() - inset - Scaled(64), height() - footer_height,
                     width() - inset - 1, height() - bottom_inset - 1));
  if (input_)
    input_->layout(Rect(inset, header_height, width() - inset - 1,
                        std::min<int>(height() - footer_height - 1,
                                      header_height + Scaled(80) - 1)));
  if (focus_pending_) {
    focus_pending_ = false;
    preferredFocusChild()->requestFocus();
  }
  if (reveal_pending_ && mode_ != DatePickerMode::kInput) {
    reveal_pending_ = false;
    body_.revealCursor();
  }
}

void DatePickerPanel::scrollTo(const Rect& cell) {
  auto pos = viewport_.getScrollPosition();
  int x = pos.x;
  int y = pos.y;
  if (cell.xMin() + x < 0)
    x = -cell.xMin();
  else if (cell.xMax() + x >= viewport_.width())
    x = viewport_.width() - cell.xMax() - 1;
  if (cell.yMin() + y < 0)
    y = -cell.yMin();
  else if (cell.yMax() + y >= viewport_.height())
    y = viewport_.height() - cell.yMax() - 1;
  viewport_.scrollTo(x, y);
}

void DatePickerPanel::updateSelection() {
  confirm_.setEnabled(session_.enabled(session_.draft));
  body_.resetCursor();
}

void DatePickerPanel::selectDate(CivilDay day) {
  if (!session_.enabled(day) || session_.draft == day) return;
  session_.draft = day;
  header_.dirtyPart(5);
  updateSelection();
}

void DatePickerPanel::selectMonth(int year, int month) {
  CivilDay next = CivilDay::FromYmd(year, month, 1);
  if (!next.isValid()) return;
  if (session_.draft.isValid()) {
    CivilDay candidate = CivilDay::FromYmd(year, month, session_.draft.day());
    session_.draft =
        session_.enabled(candidate) ? candidate : CivilDay::Invalid();
  }
  session_.month = next;
  setMode(DatePickerMode::kDays);
  updateSelection();
}

void DatePickerPanel::prepareInteraction(Widget& source) {
  if (&source != &header_) header_.onCancel();
  if (&source != &body_) body_.onCancel();
}

void DatePickerPanel::activateHeader(int control) {
  body_.onCancel();
  if (control == 4) {
    setMode(mode_ == DatePickerMode::kInput ? DatePickerMode::kDays
                                            : DatePickerMode::kInput);
  } else if (control == 1 || control == 2) {
    setMode(control == 1 ? DatePickerMode::kMonths : DatePickerMode::kYears);
  } else if (mode_ == DatePickerMode::kYears) {
    first_year_ =
        std::max(1, std::min(9961, first_year_ + (control == 0 ? -120 : 120)));
    body_.resetCursor();
    viewport_.scrollToTop();
    reveal_pending_ = true;
    requestLayout();
    body_.invalidateInterior();
  } else if (mode_ != DatePickerMode::kInput) {
    CivilDay next = ShiftMonth(session_.month, control == 0 ? -1 : 1);
    if (next.isValid()) {
      session_.month = next;
      header_.dirtyPart(1);
      header_.dirtyPart(2);
      body_.invalidateInterior();
      reveal_pending_ = true;
      requestLayout();
      updateSelection();
    }
  }
}

void DatePickerPanel::setMode(DatePickerMode mode) {
  body_.onCancel();
  if (mode_ == DatePickerMode::kInput && mode != DatePickerMode::kInput) {
    inputChanged();
    if (!session_.enabled(session_.draft)) return;
    session_.month = MonthStart(session_.draft);
  }
  if (mode == DatePickerMode::kInput && !input_) {
    input_.reset(new (std::nothrow) Input(context(), *this));
    if (!input_) return;
    attachChild(*input_);
  }
  mode_ = mode;
  focus_pending_ = true;
  reveal_pending_ = true;
  if (mode == DatePickerMode::kYears)
    first_year_ = (session_.month.year() - 1) / 120 * 120 + 1;
  if (input_) {
    syncing_input_ = true;
    if (mode == DatePickerMode::kInput) {
      char text[64] = {};
      if (session_.draft.isValid())
        session_.codec().format(session_.draft, text, sizeof(text));
      input_->setText(text);
    }
    syncing_input_ = false;
    input_->setVisibility(mode == DatePickerMode::kInput ? Visibility::kVisible
                                                         : Visibility::kGone);
  }
  viewport_.setVisibility(mode == DatePickerMode::kInput
                              ? Visibility::kGone
                              : Visibility::kVisible);
  viewport_.scrollToTop();
  requestLayout();
  invalidateInterior();
  updateSelection();
  if (mode == DatePickerMode::kInput) inputChanged();
}

void DatePickerPanel::inputChanged() {
  if (syncing_input_ || !input_ || mode_ != DatePickerMode::kInput) return;
  CivilDay parsed;
  bool valid = session_.codec().parse(input_->text(), parsed).status ==
                   roo_time::TextStatus::kOk &&
               session_.enabled(parsed);
  CivilDay next = valid ? parsed : CivilDay::Invalid();
  if (session_.draft != next) header_.dirtyPart(5);
  session_.draft = next;
  if (valid)
    input_->clearError();
  else
    input_->setErrorText(session_.strings().invalid_date);
  updateSelection();
}
}  // namespace roo_windows::material3::internal
