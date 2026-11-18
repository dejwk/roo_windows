#include <algorithm>
#include <cstdio>
#include <new>

#include "roo_display/shape/smooth.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_icons/filled/24/action.h"
#include "roo_icons/filled/24/editor.h"
#include "roo_icons/filled/24/navigation.h"
#include "roo_windows/core/child_layout.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/date_picker/date_picker_internal.h"
#include "roo_windows/material3/internal/density.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3::internal {
using roo_time::CivilDay;
namespace {
constexpr int kHeader = Scaled(112);
constexpr int kFooter = Scaled(64);

/// Resolves eligible calendar whitespace directly from the shared theme.
int CellSize(const Theme& theme) {
  return Scaled(48 + 4 * ResolveDensityLevel(theme.material3Theme().density));
}

int InsetSize(const Theme& theme) {
  return Scaled(16 + 2 * ResolveDensityLevel(theme.material3Theme().density));
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
                 Color fill, Color outline, int outline_width)
      : cell_(cell.asBox()),
        surface_(surface),
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
      marker_.readColors(x, y, batch, fill);
      outline_.readColors(x, y, batch, outline);
      for (uint32_t i = 0; i < batch; ++i) {
        result[i] = AlphaBlend(AlphaBlend(surface_, fill[i]), outline[i]);
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
  roo_display::SmoothShape marker_;
  roo_display::SmoothShape outline_;
};

/// Chooses legible compact text without retaining per-cell font state.
const TextStyle& CellTextStyle(const Rect& cell, int nominal_cell) {
  if (cell.height() >= nominal_cell) return text_style_body_medium();
  return cell.height() < Scaled(18)   ? text_style_label_small()
         : cell.height() < Scaled(32) ? text_style_body_small()
                                      : text_style_body_medium();
}

/// Resolves glyphs, rounded markers, and surface together in a single tile.
void PaintCell(PaintContext& ctx, roo::string_view text, const Rect& cell,
               Color foreground, Color background, Color accent, bool selected,
               bool today, bool focused, int nominal_cell) {
  const TextStyle& style = CellTextStyle(cell, nominal_cell);
  if (!selected && !today && !focused) {
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
  const CellBackground composed(
      cell, marker, background, selected ? accent : background,
      today || focused ? accent : roo_display::color::Transparent,
      focused ? Scaled(2) : Scaled(1));
  roo_display::Tile tile(&label, cell.asBox(),
                         roo_display::kCenter | roo_display::kMiddle);
  tile.setBackground(&composed);
  // Keep background reads in short bands so Tile's background filter never
  // allocates a stream. Sources are borrowed only through this synchronous
  // draw.
  for (int y = cell.yMin(); y <= cell.yMax(); y += 8) {
    PaintContext band = ctx.clipped(
        Rect(cell.xMin(), y, cell.xMax(), std::min<int>(y + 7, cell.yMax())));
    band.drawObject(tile);
  }
  ctx.addExclusion(cell);
}

/// Makes room for seven compact calendar rows on short displays.
int HeaderHeight(const Theme& theme, bool shrink, int height) {
  const int level = ResolveDensityLevel(theme.material3Theme().density);
  return shrink && height < Scaled(320) ? Scaled(80 + 2 * level)
                                        : kHeader + Scaled(4 * level);
}

int FooterHeight(const Theme& theme, bool shrink, int height) {
  const int level = ResolveDensityLevel(theme.material3Theme().density);
  return (shrink && height < Scaled(320) ? Scaled(48) : kFooter) +
         Scaled(4 * level);
}

int BottomInset(const Theme& theme, bool shrink, int height) {
  return shrink && height < Scaled(320) ? InsetSize(theme) / 2
                                        : InsetSize(theme);
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
  const int cell_size = CellSize(theme());
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
  return Dimensions(2 * CellSize(theme()) + Scaled(112),
                    HeaderHeight(theme(), false, 0));
}

roo::string_view DatePickerHeader::monthLabel(int month) const {
  const roo_locale::CalendarLocale& locale = panel_.session().locale();
  return abbreviated_months_ != 0 ? locale.month_standalone_abbreviated[month]
                                  : locale.month_standalone_wide[month];
}

Dimensions DatePickerHeader::onMeasure(WidthSpec width, HeightSpec height) {
  const int cell_size = CellSize(theme());
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
  setDirty(part == 5 ? Rect(0, 0, width() - CellSize(theme()) - 1,
                            controlBounds(0).yMin() - 1)
                     : controlBounds(part));
}

void DatePickerHeader::paint(PaintContext& ctx) const {
  const int cell_size = CellSize(theme());
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
    if (control == 0 || control == 3 || control == 4) {
      MonoIcon icon = control == 0 ? ic_filled_24_navigation_chevron_left()
                      : control == 3
                          ? ic_filled_24_navigation_chevron_right()
                          : (input ? ic_filled_24_action_calendar_today()
                                   : ic_filled_24_editor_mode_edit());
      icon.color_mode().setColor(foreground);
      PaintContext local = ctx.clipped(bounds);
      local.setBgcolor(bg);
      local.drawTiled(icon, bounds,
                      roo_display::kCenter | roo_display::kMiddle);
      ctx.addExclusion(bounds);
    } else {
      roo::string_view label;
      char year[16];
      if (control == 1) {
        label = monthLabel(session.month.month() - 1);
      } else {
        std::snprintf(year, sizeof(year), "%d", session.month.year());
        label = year;
      }
      PaintText(ctx, label, navigationTextStyle(), foreground, bg, bounds);
    }
  }
  dirty_parts_ = 0;
}

void DatePickerHeader::onSingleTapUp(XDim x, YDim y) {
  for (int control = 0; control < 5; ++control) {
    if (controlBounds(control).contains(x, y)) {
      dirtyPart(focused_control_);
      focused_control_ = control;
      dirtyPart(focused_control_);
      requestFocus();
      panel_.activateHeader(control);
      return;
    }
  }
}

bool DatePickerHeader::onKeyEvent(const KeyEvent& event) {
  if (event.phase != KeyPhase::kDown && event.phase != KeyPhase::kRepeat)
    return false;
  if (event.code == KeyCode::kLeft || event.code == KeyCode::kRight) {
    dirtyPart(focused_control_);
    focused_control_ =
        (focused_control_ + (event.code == KeyCode::kLeft ? 4 : 1)) % 5;
    dirtyPart(focused_control_);
    return true;
  }
  if (event.code == KeyCode::kEnter || event.code == KeyCode::kSpace) {
    panel_.activateHeader(focused_control_);
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
  const int cell_size = CellSize(theme());
  return Dimensions(7 * cell_size, panel_.mode() == DatePickerMode::kDays
                                       ? 7 * cell_size
                                       : ((cellCount() + 2) / 3) * cell_size);
}

Dimensions DatePickerBody::onMeasure(WidthSpec width, HeightSpec height) {
  const Dimensions desired = getSuggestedMinimumDimensions();
  int w = desired.width();
  int h = desired.height();
  if (panel_.session().shrinkToFit()) {
    if (width.value() > 0) w = std::min<int>(w, width.value());
    if (height.value() > 0 && panel_.mode() != DatePickerMode::kYears)
      h = std::min<int>(h, height.value());
  }
  abbreviated_months_ = false;
  if (panel_.mode() == DatePickerMode::kMonths) {
    const TextStyle& style =
        CellTextStyle(Rect(0, 0, w / 3 - 1, h / 4 - 1), CellSize(theme()));
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
  if (days && isInvalidated()) {
    for (int col = 0; col < 7; ++col) {
      const Rect first = cellBounds(col);
      PaintText(
          ctx,
          session.locale().weekday_narrow[(col + session.firstWeekday()) % 7],
          CellTextStyle(first, CellSize(theme())), colors.onSurfaceVariant,
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
    bool enabled = true, selected = false, today = false;
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
    Color color = selected  ? colors.onPrimary
                  : enabled ? colors.onSurface
                            : colors.onSurfaceVariant;
    if (!enabled)
      color =
          AlphaBlend(background(), Color(97, color.r(), color.g(), color.b()));
    PaintContext cell_ctx = ctx.clipped(cell);
    PaintCell(cell_ctx, text, cell, color, background(), colors.primary,
              selected, today, isFocused() && cursor_ == i, CellSize(theme()));
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

void DatePickerBody::onSingleTapUp(XDim x, YDim y) {
  const int index = cellAt(x, y);
  if (!cellEnabled(index)) return;
  cursor_ = index;
  setDirty();
  requestFocus();
  activate(index);
}

void DatePickerBody::resetCursor() {
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
  setDirty();
}

void DatePickerBody::revealCursor() {
  if (cursor_ >= 0) panel_.scrollTo(cellBounds(cursor_));
}

bool DatePickerBody::onKeyEvent(const KeyEvent& event) {
  if (event.phase != KeyPhase::kDown && event.phase != KeyPhase::kRepeat)
    return false;
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
      activate(cursor_);
      return true;
    default:
      return false;
  }
  cursor_ = std::max(0, std::min(cellCount() - 1, next));
  revealCursor();
  setDirty();
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
  const int cell_size = CellSize(theme());
  const int inset = InsetSize(theme());
  return Dimensions(7 * cell_size + 2 * inset,
                    HeaderHeight(theme(), false, 0) + 7 * cell_size +
                        FooterHeight(theme(), false, 0) + inset);
}

Dimensions DatePickerPanel::minimumViewportDimensions() const {
  const int span =
      Scaled(240 + 8 * ResolveDensityLevel(theme().material3Theme().density));
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
  const int cell_size = CellSize(theme());
  const int inset = InsetSize(theme());
  Dimensions desired = getSuggestedMinimumDimensions();
  int w = width.resolveSize(desired.width()),
      h = height.resolveSize(desired.height());
  const bool shrink = session_.shrinkToFit();
  const int header_height = HeaderHeight(theme(), shrink, h);
  header_.measure(WidthSpec::Exactly(w - 2 * inset),
                  HeightSpec::Exactly(header_height - inset / 2));
  viewport_.measure(
      WidthSpec::Exactly(w - 2 * inset),
      HeightSpec::Exactly(h - header_height - FooterHeight(theme(), shrink, h) -
                          BottomInset(theme(), shrink, h)));
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
  const int inset = InsetSize(theme());
  const bool shrink = session_.shrinkToFit();
  const int header_height = HeaderHeight(theme(), shrink, height());
  const int footer_height = FooterHeight(theme(), shrink, height());
  const int bottom_inset = BottomInset(theme(), shrink, height());
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
  int x = pos.x, y = pos.y;
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

void DatePickerPanel::activateHeader(int control) {
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
