# Material 3 date pickers

## Status and scope

Implemented on 2026-09-19: single-date modal calendar, modal numeric input,
month/year selection, and docked date field with compact-screen promotion.
This document reconciles the earlier proposal with the current shared
transient host and the requested exclusive calendar focus behavior.

The implementation uses `roo_time::CivilDay`, never midnight timestamps.
Calendar selection is independent of time zones and daylight-saving changes.
Applications supply `today` explicitly, using their own clock and local-date
policy. No picker reads a clock, retains a time zone, or duplicates civil-date
arithmetic. Valid civil years are 1 through 9999.

Range selection, simultaneous field editing while the calendar is open,
live anchor repositioning, and arbitrary nested transient surfaces are future
work. Nothing in this component changes the transient-host model.

## Public API

Headers are under `roo_windows/material3/date_picker/`:

- `date_picker_types.h`: `DatePickerBounds`, `DatePickerEntryMode`,
  `DatePickerDismissReason`, `DatePickerStrings`, and `DateTextCodec`.
- `date_picker.h`: reusable `ModalDatePicker` presenter.
- `docked_date_picker_field.h`: `DockedDatePickerField`, a `TextField` subclass.

`ModalDatePicker` is a lightweight presenter, rather than a persistent widget
container. Configure it while closed, then call `open(Task&)`. The return value
is `PresentationStartResult`; unavailable owners, invalid bounds, unsuitable
viewports, and a busy host fail without taking over the current presentation.
It never replaces another active transient.

```cpp
#include "roo_windows/material3/date_picker/date_picker.h"

class MaintenancePicker : public roo_windows::material3::ModalDatePicker {
 public:
  using ModalDatePicker::ModalDatePicker;
 protected:
  void onAccepted(roo_time::CivilDay day) override {
    // Persist the confirmed civil date in the application model.
  }
};

// Keep the presenter alive for the session.
// MaintenancePicker picker(context);
// picker.setValue(roo_time::CivilDay::FromYmd(2026, 10, 1));
// picker.setToday(local_today);
// picker.setBounds({local_today, roo_time::CivilDay::FromYmd(2028, 12, 31)});
// picker.setEntryMode(DatePickerEntryMode::kCalendar);  // Or kInput.
// PresentationStartResult result = picker.open(owner_task);
```

Modal values use `setValue()/value()`. Docked fields use `setDate()/date()` to
avoid colliding with the inherited text editor's `value()` virtual method.
Both expose `setToday()`, `setBounds()`, and `setDisplayedMonth()`. Setters
require a closed picker. An invalid date represents no selection; invalid
bound endpoints mean unbounded. Reversed bounds reject admission.

Override `isDateEnabled(CivilDay)` for application filtering, and
`datePickerStrings()` / `dateTextCodec()` for localization. Filtering and
conversion must not mutate the presentation. Shared labels and codecs must
outlive their use; format output, including NUL, must fit a 64-byte buffer.

English defaults use Sunday-first and `MM/DD/YYYY`. `ROO_LANG=ROO_LANG_pl`
selects Monday-first and `DD.MM.RRRR`. Defaults delegate strict date-only
parsing and formatting to `roo_time`; partial, impossible, or out-of-bounds
input disables confirmation and displays a validation error.

## Session and commit semantics

An active-only `DatePickerSession` owns the panel, draft, visible month and
focus scope. Opening seeds the visible month from the explicit month, value,
today, lower bound, upper bound, then January 1970. Invalid or disabled values
start with an empty draft. The committed value remains unchanged until OK.

Day selection updates only the draft. Previous/next month navigation preserves
the draft. Explicit month/year selection preserves the day number when it is
valid and enabled in the destination month; otherwise it clears the draft.
No silent end-of-month clamping occurs. Switching calendar to input formats
the draft. Valid input returns to its calendar month; invalid input must be
corrected before switching back. Cancel and Back remain available.

Completion detaches the surface, restores focus, and frees session state
before delivering exactly one terminal hook: `onAccepted(day)` or
`onDismissed(reason)`. Hooks may destroy or reopen the presenter. Cancellation
preserves the committed value. Dismissal distinguishes Cancel, outside tap,
Back, programmatic dismissal, and an unavailable owner. Destruction silently
cancels without application callbacks. The registration cancels before its
owned panel and focus scope are destroyed.

Back closes a month/year body first, returning to the day grid. From calendar
or input it dismisses the picker. Outside interaction dismisses through the
host barrier, after pointer dispatch unwinds, without activating underlying
content. Owner navigation ends the session through the shared host contract.

## Docked field and focus

A docked field opens on activation or fresh focus. Focus-triggered admission
is scheduled after focus dispatch unwinds: the focus manager must not be
reentered from `onFocusChanged()`. Pending admission is canceled on blur or
destruction. A failed admission releases temporary state.

The open calendar owns an exclusive focus scope. The source field stops
editing and cannot resume editing while that scope is active. Focus restoration
after dismissal does not reopen the calendar. This deliberately replaces the
proposal's simultaneous source-field and calendar editing; that behavior is
future work.

A valid unconfirmed typed date seeds the calendar draft without committing it.
Cancellation preserves the field's pre-open text and committed date. OK
updates both, then calls `onAccepted()`. When the calendar is closed, explicit
text editing and edit confirmation use the same codec and bounds. Invalid
text stays visible with an error.

The field observes presentation changes while open, dismissing when detached
or hidden. The anchor is captured at admission. Live movement/repositioning
is not implemented.

## Presentation and geometry

One `TransientSurfaceHost` owns attachment, barrier paint, input isolation and
focus restoration. The presenter supplies an explicit interaction owner and
its focus scope. There are no extra popup tasks, no second active-presentation
authority, and no nested menus. Month and year selection replace the panel
body inside the existing surface.

At 100% scale the regular panel is 368 by 528 dp, with 16 dp outer clearance.
Modal presentation centers it with a scrim. Docked presentation uses the menu
placement resolver's below-start placement, with flipping/clamping and a
transparent barrier. If the regular panel plus clearance does not fit, both
variants promote to the full window with a scrim and square corners.
Viewports smaller than 256 by 240 dp reject admission.

The full-screen fallback pins the header and Cancel/OK actions. Its body
scrolls in both directions, retaining 48 dp calendar cells; it does not shrink
touch targets to fit. On a 320 by 240 display only a small part of the grid is
visible at once. The selected/cursor cell is revealed after layout and keyboard
movement; users can drag the body to reach other rows and columns. Numeric
input uses the same stable outer panel geometry across mode switches.

The day grid computes 42 cells arithmetically, including disabled adjacent
month dates. Month selection uses 12 cells. Year selection uses pages of up to
120 years, three columns, with previous/next page controls. This bounds scroll
coordinates for the library's 16-bit geometry instead of allocating or laying
out thousands of year rows.

Tab traverses header, body and actions (or the input field). Within the header,
Left/Right selects a control and Enter/Space activates it. In the body,
arrows move the cursor, Home/End reach the first/last cell, PageUp/PageDown
navigate months or year pages, and Enter/Space selects. Disabled days cannot
be committed through either touch or keyboard.

## Rendering and resources

The header and day/month/year cells are owner-painted; there are no per-day
widgets or row objects. Foreground glyphs settle against their final background
before rounded decorations and remaining surface fill. Clipping skips
non-visible body cells. Calendar updates currently invalidate the body;
finer cell-level damage tracking is a possible later optimization.

Idle presenters retain only configuration and a session pointer. Numeric
input is allocated lazily when that mode is first requested, then retained
until the session ends. The docked wrapper's deferred-open executable is also
active-only. There are no persistent calendar strings, timers, or shared-host
fields added to existing widgets.

Measured with the installed ESP32-C3 RISC-V compiler, `-fno-exceptions` and
`-fno-rtti` (object sizes, excluding allocator metadata and external host/editor
state):

| Object | Bytes |
| --- | ---: |
| Closed `ModalDatePicker` | 36 |
| Base Material 3 `TextField` | 108 |
| Closed `DockedDatePickerField` | 136 |
| Active `DatePickerSession`, including panel | 448 |
| `DatePickerPanel` (included in session) | 408 |
| Header / body (each, included in panel) | 36 |

Run `benchmarks/material3_date_picker_size_probe.sh COMPILER NM` with sibling
Roo libraries, or set `ROO_LIBRARIES_ROOT`. This is a target-ABI object-size
probe, not a firmware flash-size or peak-heap measurement.

The resource regression asserts zero allocations for warmed model navigation.
Ten full calendar paint frames currently record 1080 allocations through the
existing `roo_display` glyph-stream renderer. As with Material 3 text fields,
removing those upstream allocations is deferred; this implementation does not
claim allocation-free paint.

## Validation and example

Focused tests cover civil bounds/locale helpers, drafts and terminal hooks,
input validation, month/year transitions, civil endpoints, shared-host rejection,
owner navigation, outside dismissal, callback reopening/destruction, field
focus restoration/destruction, and editor isolation. Golden tests cover modal
calendar/input/error/month/year bodies, compact clipping, and closed/anchored/
promoted docked fields. A separate target verifies Polish defaults.

The runnable example is
`//examples/material3/date_picker/maintenance_date:maintenance_date`. It uses
real ILI9341/XPT2046 setup and ROO_TESTING emulation, demonstrates modal calendar,
modal input and the docked field, and saves accepted dates into the form.
Its compact screen demonstrates promotion; the larger golden fixture exercises
anchored placement. Physical touchscreen validation remains manual.

The seven date-picker test targets pass with picker sources and test subclasses
compiled using `--per_file_copt='.*date_picker.*\.cpp@-fno-exceptions,-fno-rtti'`.
The presentation and docked behavior targets also pass with `--config=asan`.
Related text-field, keyboard-avoidance, dialog, menu and transient-host
regressions pass. The example builds; interactive hardware checks were not run.

## Future work

- Simultaneous editing of the docked source while its calendar is open.
- Live anchor tracking and repositioning during a session.
- A denser compact layout, dynamic input-only panel height, and finer repaint
  damage tracking, while preserving usable touch targets.
- Range selection and richer locale-specific headline formatting.
- Upstream allocation-free glyph rendering.
