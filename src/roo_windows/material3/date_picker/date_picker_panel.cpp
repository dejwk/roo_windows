#include <algorithm>
#include <cstdio>
#include <new>

#include "roo_display/ui/text_label.h"
#include "roo_icons/filled/24/action.h"
#include "roo_icons/filled/24/editor.h"
#include "roo_icons/filled/24/navigation.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/date_picker/date_picker_internal.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3::internal {
using roo_time::CivilDay;
namespace {
constexpr int kCell = Scaled(48);
constexpr int kInset = Scaled(16);
constexpr int kHeader = Scaled(112);
constexpr int kFooter = Scaled(64);

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

/// Settles glyphs inside the safe center before registering a rounded
/// decoration.
void PaintCell(PaintContext& ctx, roo::string_view text, const Rect& cell,
               Color foreground, Color background, Color accent, bool selected,
               bool today, bool focused) {
  Rect circle(cell.xMin() + (cell.width() - Scaled(40)) / 2,
              cell.yMin() + Scaled(4),
              cell.xMin() + (cell.width() + Scaled(40)) / 2 - 1,
              cell.yMin() + Scaled(44) - 1);
  if (selected || today || focused) {
    Rect inner(circle.xMin() + Scaled(6), circle.yMin() + Scaled(6),
               circle.xMax() - Scaled(6), circle.yMax() - Scaled(6));
    // Four-digit years need a wider pill than a day marker.
    if (text.size() > 2) {
      circle = Rect(cell.xMin() + Scaled(4), cell.yMin() + Scaled(4),
                    cell.xMax() - Scaled(4), cell.yMin() + Scaled(44) - 1);
      inner = Rect(circle.xMin() + Scaled(8), circle.yMin() + Scaled(6),
                   circle.xMax() - Scaled(8), circle.yMax() - Scaled(6));
    }
    PaintText(ctx, text, text_style_body_medium(), foreground,
              selected ? accent : background, inner);
    PaintDecoration decoration;
    decoration.bounds = circle;
    decoration.background = selected ? accent : background;
    const uint8_t radius = Scaled(20);
    decoration.corner_radii = {radius, radius, radius, radius};
    decoration.outline_width = SmallNumber(focused ? 2 : today ? 1 : 0);
    decoration.outline_color = accent;
    ctx.addDecoration(decoration);
  } else {
    PaintText(ctx, text, text_style_body_medium(), foreground, background,
              cell);
  }
}
}  // namespace

DatePickerHeader::DatePickerHeader(ApplicationContext& context,
                                   DatePickerPanel& panel)
    : BasicWidget(context), panel_(panel) {}

Rect DatePickerHeader::controlBounds(int control) const {
  if (control == 4) return Rect(width() - kCell, 0, width() - 1, kCell - 1);
  int y = height() - kCell;
  if (control == 0) return Rect(0, y, kCell - 1, height() - 1);
  if (control == 3) return Rect(width() - kCell, y, width() - 1, height() - 1);
  // Reserve a full target for the year even in a 240 dp portrait window.
  int center =
      std::min(width() - 2 * kCell, (width() - 2 * kCell) * 3 / 5 + kCell);
  return control == 1 ? Rect(kCell, y, center - 1, height() - 1)
                      : Rect(center, y, width() - kCell - 1, height() - 1);
}

void DatePickerHeader::dirtyPart(int part) {
  dirty_parts_ |= 1 << part;
  setDirty(part == 5
               ? Rect(0, Scaled(24), width() - kCell - 1, height() - kCell - 1)
               : controlBounds(part));
}

void DatePickerHeader::paint(PaintContext& ctx) const {
  const DatePickerSession& session = panel_.session();
  const auto& colors = theme().material3Theme().color;
  Color bg = panel_.background();
  bool input = panel_.mode() == DatePickerMode::kInput;
  if (isInvalidated())
    PaintText(ctx,
              input ? session.strings().headline_input_date
                    : session.strings().headline_select_date,
              text_style_label_large(), colors.onSurfaceVariant, bg,
              Rect(0, 0, width() - kCell - 1, Scaled(24) - 1),
              roo_display::kLeft | roo_display::kMiddle);
  if (isInvalidated() || (dirty_parts_ & (1 << 5))) {
    char text[64] = {};
    if (session.draft.isValid())
      session.codec().format(session.draft, text, sizeof(text));
    PaintText(ctx,
              session.draft.isValid() ? roo::string_view(text)
                                      : roo::string_view("—"),
              text_style_headline_small(), colors.onSurface, bg,
              Rect(0, Scaled(24), width() - kCell - 1, height() - kCell - 1),
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
        label = session.strings().month_names[session.month.month() - 1];
      } else {
        std::snprintf(year, sizeof(year), "%d", session.month.year());
        label = year;
      }
      PaintText(ctx, label, text_style_label_large(), foreground, bg, bounds);
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
    : BasicSurfaceWidget(context), panel_(panel) {}

Color DatePickerBody::background() const { return panel_.background(); }

int DatePickerBody::cellCount() const {
  return panel_.mode() == DatePickerMode::kDays ? 42
         : panel_.mode() == DatePickerMode::kMonths
             ? 12
             : std::min(120, 10000 - panel_.firstYear());
}

Dimensions DatePickerBody::getSuggestedMinimumDimensions() const {
  return Dimensions(7 * kCell, panel_.mode() == DatePickerMode::kDays
                                   ? 7 * kCell
                                   : ((cellCount() + 2) / 3) * kCell);
}

Rect DatePickerBody::cellBounds(int index) const {
  int columns = panel_.mode() == DatePickerMode::kDays ? 7 : 3;
  int w = width() / columns;
  int x = index % columns * w;
  int y = (index / columns + (columns == 7 ? 1 : 0)) * kCell;
  return Rect(x, y, x + w - 1, y + kCell - 1);
}

void DatePickerBody::paint(PaintContext& ctx) const {
  const DatePickerSession& session = panel_.session();
  const auto& colors = theme().material3Theme().color;
  bool days = panel_.mode() == DatePickerMode::kDays;
  if (days && isInvalidated()) {
    for (int col = 0; col < 7; ++col) {
      PaintText(
          ctx,
          session.strings()
              .weekday_narrow[(col + session.strings().first_weekday) % 7],
          text_style_body_medium(), colors.onSurfaceVariant, background(),
          Rect(col * (width() / 7), 0, (col + 1) * (width() / 7) - 1,
               kCell - 1));
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
      CivilDay day = GridDay(session.month, session.strings().first_weekday, i);
      old_selected = day.isValid() && day == painted_draft_;
      new_selected = day.isValid() && day == session.draft;
    }
    if (!isInvalidated() && old_selected == new_selected &&
        (painted_focused_ && painted_cursor_ == i) ==
            (isFocused() && cursor_ == i))
      continue;
    char number[8];
    roo::string_view text;
    bool enabled = true, selected = false, today = false;
    if (days) {
      CivilDay day = GridDay(session.month, session.strings().first_weekday, i);
      if (!day.isValid()) continue;
      enabled = day.month() == session.month.month() && session.enabled(day);
      selected = day == session.draft;
      today = day == session.today();
      std::snprintf(number, sizeof(number), "%d", day.day());
      text = number;
    } else if (panel_.mode() == DatePickerMode::kMonths) {
      text = session.strings().month_names[i];
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
              selected, today, isFocused() && cursor_ == i);
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
    CivilDay day =
        GridDay(session.month, session.strings().first_weekday, index);
    if (day.isValid() && day.month() == session.month.month())
      panel_.selectDate(day);
  } else if (panel_.mode() == DatePickerMode::kMonths) {
    panel_.selectMonth(session.month.year(), index + 1);
  } else {
    panel_.selectMonth(panel_.firstYear() + index, session.month.month());
  }
}

void DatePickerBody::onSingleTapUp(XDim x, YDim y) {
  int columns = panel_.mode() == DatePickerMode::kDays ? 7 : 3;
  int row = y / kCell - (columns == 7 ? 1 : 0);
  if (x < 0 || x >= width() || row < 0) return;
  int index = row * columns + x / (width() / columns);
  if (index >= cellCount()) return;
  cursor_ = index;
  setDirty();
  requestFocus();
  activate(index);
}

void DatePickerBody::resetCursor() {
  const DatePickerSession& session = panel_.session();
  if (panel_.mode() == DatePickerMode::kDays) {
    int offset =
        (7 + session.month.dayOfWeek() - session.strings().first_weekday) % 7;
    cursor_ = offset + ((session.draft.isValid() &&
                         MonthStart(session.draft) == session.month)
                            ? session.draft.day() - 1
                            : 0);
  } else if (panel_.mode() == DatePickerMode::kMonths) {
    cursor_ = session.month.month() - 1;
  } else {
    cursor_ = std::max(0, std::min(cellCount() - 1,
                                   session.month.year() - panel_.firstYear()));
  }
  setDirty();
}

void DatePickerBody::revealCursor() { panel_.scrollTo(cellBounds(cursor_)); }

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
    : Container(context),
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
  return ColorToken::kSurfaceContainerHigh;
}

Color DatePickerPanel::background() const {
  return theme().material3Theme().color.surfaceContainerHigh;
}

BorderStyle DatePickerPanel::getBorderStyle() const {
  return BorderStyle(full_screen_ ? 0 : Scaled(28), 0);
}

void DatePickerPanel::paint(PaintContext& ctx) const { ctx.clear(); }

Dimensions DatePickerPanel::getSuggestedMinimumDimensions() const {
  return Dimensions(7 * kCell + 2 * kInset,
                    kHeader + 7 * kCell + kFooter + kInset);
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
  Dimensions desired = getSuggestedMinimumDimensions();
  int w = width.resolveSize(desired.width()),
      h = height.resolveSize(desired.height());
  header_.measure(WidthSpec::Exactly(w - 2 * kInset),
                  HeightSpec::Exactly(kHeader - kInset / 2));
  viewport_.measure(WidthSpec::Exactly(w - 2 * kInset),
                    HeightSpec::Exactly(h - kHeader - kFooter - kInset));
  cancel_.measure(WidthSpec::Exactly(Scaled(96)), HeightSpec::Exactly(kCell));
  confirm_.measure(WidthSpec::Exactly(Scaled(64)), HeightSpec::Exactly(kCell));
  if (input_)
    input_->measure(WidthSpec::Exactly(w - 2 * kInset),
                    HeightSpec::Exactly(Scaled(80)));
  return Dimensions(w, h);
}

void DatePickerPanel::onLayout(bool, const Rect&) {
  header_.layout(Rect(kInset, kInset / 2, width() - kInset - 1, kHeader - 1));
  viewport_.layout(Rect(kInset, kHeader, width() - kInset - 1,
                        height() - kFooter - kInset - 1));
  cancel_.layout(Rect(width() - kInset - Scaled(160), height() - kFooter,
                      width() - kInset - Scaled(64) - 1,
                      height() - kInset - 1));
  confirm_.layout(Rect(width() - kInset - Scaled(64), height() - kFooter,
                       width() - kInset - 1, height() - kInset - 1));
  if (input_)
    input_->layout(
        Rect(kInset, kHeader, width() - kInset - 1,
             std::min<int>(height() - kFooter - 1, kHeader + Scaled(80) - 1)));
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
