# Roo Windows Material 3 Slider Design

## Implementation status

**Implemented.** The defined scope is present in the current source tree. Dependency status and any separately scoped follow-up work are recorded in the [status index](../README.md).

## Objective

Extend `roo_windows::material3::Slider` so that it covers the Material 3 slider
features that are currently missing in `roo_windows`, while staying compatible
with the framework's embedded constraints and existing widget model.

The design should cover:

- the Material 3 slider variants: standard, centered, and range,
- continuous and discrete operation,
- optional value indicators,
- stop indicators and tick behavior,
- orientation and size presets,
- the Android slider interaction model where it materially affects API shape,
- and a compact customization surface for the richer appearance hooks exposed by
  Android.

This document defines the missing API family. It does not describe an existing
implementation.

## Current Status in `roo_windows`

The slider family described here is implemented, including semantic ranges,
discrete and centered modes, range selection, value indicators, orientation,
and size variants. The notes below record the narrow baseline from which the
implementation proceeded.

What exists today:

- `material3::Slider` is a single-thumb slider backed by a normalized
  `uint16_t pos_` value.
- It supports a continuous horizontal track, tap-to-jump, and drag.
- It paints a Material 3-like active track, inactive track, and line-handle.
- It supports enabled, disabled, pressed, and overlay behavior already handled
  by the widget framework.
- It reuses the existing `Widget::setOnInteractiveChange()` callback model.

What does not exist yet:

- no public `valueFrom` / `valueTo` domain,
- no discrete mode or step snapping,
- no range slider,
- no centered slider semantics,
- no value indicator bubble,
- no value formatter,
- no minimum thumb separation for range selection,
- no slider-specific interaction lifecycle callbacks,
- no orientation support beyond the current horizontal layout,
- no size variants beyond the current XS-like geometry,
- no API for inset icons, stop indicators, or richer track decoration,
- no public appearance override surface comparable to Android's slider family.

The current implementation is therefore best understood as a narrow,
continuous, horizontal standard slider.

## Motivation

The current widget is good enough for basic continuous controls such as volume
or brightness, but it leaves several Material 3 and Android use cases outside
the API:

- settings that require discrete choices,
- bipolar controls that should grow away from a centered zero point,
- min/max range selection,
- UI that must expose the numeric value while dragging,
- large-format slider hero surfaces,
- and product-specific visuals that need limited but explicit appearance
  customization.

The gap is not only visual. The current `uint16_t`-only API forces application
code to map real values onto an internal normalized domain. That makes common
features such as discrete steps, range selection, or minimum separation harder
to express and validate.

## Background

### Material 3 Signals

Material 3 defines three slider variants:

1. standard,
2. centered,
3. range.

Material 3 also treats the following as first-class slider configuration:

1. continuous vs. discrete operation,
2. optional value indicator,
3. optional stop indicators,
4. horizontal and vertical orientation,
5. size presets XS, S, M, L, XL,
6. optional inset icon for standard M, L, and XL sliders.

Material 3 behavior guidance also matters here:

1. value changes should take effect immediately while dragging,
2. discrete sliders snap to valid stops,
3. range sliders show a value indicator for the active thumb only,
4. centered sliders fill from the center rather than from the minimum edge,
5. vertical range sliders are discouraged, but the variant still exists.

### Android Signals

Android's slider implementations add several API signals that are important even
if `roo_windows` does not mirror every setter one-for-one.

The strongest signals are:

1. sliders expose a real value domain via `valueFrom`, `valueTo`, and `stepSize`,
2. range sliders expose multiple values and an active thumb index,
3. range sliders support minimum thumb separation,
4. value labels have explicit behavior and formatting hooks,
5. interaction lifecycle distinguishes changing, touch start, and touch end,
6. orientation and centered mode are explicit configuration,
7. appearance customization is broad, but mostly falls into track, thumb,
   tick, halo, stop-indicator, and icon groups.

These Android signals should influence `roo_windows` API shape even where the
exact Android surface is too large for an embedded-first library.

### Local Framework Constraints

The design should fit the existing `roo_windows` model:

- widgets typically keep state locally,
- `Widget::setOnInteractiveChange()` is the common low-level interaction hook,
- embedded RAM and code size matter,
- and newer APIs should preserve a low-allocation path.

That argues against copying the entire Android setter matrix directly. Instead,
`roo_windows` should expose a compact semantic API first, plus a grouped
appearance override surface where needed.

#### RAM Budget Is the Dominant Constraint

ESP32-class targets typically have ~4-16 MB of flash but only ~320 KB of
static RAM. Every widget instance lives in RAM, while vtables, default themes,
and const configuration structs live in flash. The `roo_windows` widget
catalog therefore follows three rules of thumb:

1. Per-instance RAM cost of the base case must stay close to the size of the
   `Widget` base class. Optional features must not enlarge instances that do
   not use them.
2. Optional behavior is added by subclassing or by overriding virtual
   no-op hooks, not by storing more `std::function` callbacks or option
   structs in the base. A `std::function` slot costs ~16 bytes on Xtensa
   even when empty, while an unused virtual hook costs zero per instance.
3. Configuration that is naturally shared across many instances (themes,
   geometry token sets, color palettes) is referenced by pointer to a
   shared, often `constexpr`/`PROGMEM` struct, instead of being copied into
   every widget.

The slider design must therefore be evaluated on per-instance RAM footprint,
not only API ergonomics. Approximate sizes assumed in this document:

- pointer / `vptr` / `Widget*`: 4 B,
- `std::function<...>`: ~16 B even when empty,
- `Widget` base: ~36 B,
- `BasicWidget`: ~40 B (with padding/margin storage and alignment),
- the existing single-thumb `material3::Slider`: ~44 B.

All "per-instance cost" numbers below are approximate ESP32 (Xtensa, 32-bit,
`libstdc++` `std::function`) figures, rounded to natural alignment.

## Requirements

### Functional Requirements

1. Support a real value domain for single-value sliders.
2. Support range selection with two thumbs.
3. Support centered single-value sliders.
4. Support continuous and discrete modes.
5. Support value snapping and validation when a step size is configured.
6. Support optional value indicators with formatter customization.
7. Support optional stop indicators and discrete tick rendering.
8. Support horizontal and vertical orientation.
9. Support Material 3 size presets XS through XL.
10. Support a compact API for inset icons and Android-style track icons.
11. Support disabled and pressed states with token-driven defaults.
12. Preserve immediate interactive updates while dragging.

### Interaction Requirements

1. Keep the existing `setOnInteractiveChange()` behavior working.
2. Add explicit slider-level callbacks for interaction start and end.
3. Expose the active thumb for range slider interactions.
4. Make hit-testing and gesture arbitration depend on orientation.
5. Preserve tap-to-jump and drag-to-adjust behavior.

### API Requirements

1. Expose only semantic construction and value access for Material 3 sliders.
2. Keep the default constructor shape cheap by treating `Slider(context)` as a
  unit-range slider over `[0.0f, 1.0f]`.
3. Keep implementation math expressed in semantic values rather than a
  normalized fixed-point compatibility layer.

### Embedded Constraints

1. Do not require heap allocation for the common slider path.
2. Keep default construction cheap.
3. Keep the per-instance RAM footprint of the base `Slider` as close as
   practical to the existing ~44 B baseline. Optional features must not
   enlarge sliders that do not use them.
4. Move features that are off-by-default and rarely used (icons, multiple
   interaction-lifecycle callbacks, broad appearance overrides) into
   subclasses, virtual no-op hooks, or shared theme references rather than
   into the base widget's stored state.
5. Prefer grouped configuration structs that can be `constexpr`/`PROGMEM`
   and shared by pointer over per-instance copies.
6. Avoid API choices that force dynamic string allocation during dragging.

## Design Overview

The proposed public surface has three layers:

1. semantic configuration on the base widget,
2. value and interaction API on the base widget,
3. optional features added by subclassing or by referencing shared,
   often `constexpr` configuration structs.

The family is split into two public widgets:

1. `material3::Slider` for standard and centered single-value sliders,
2. `material3::RangeSlider` for range selection.

This follows both Material 3 variant semantics and Android's class split. It is
cleaner than a single widget whose meaning changes based on thumb count.

Layered widgets within each family:

- `Slider` / `RangeSlider` are the small base widgets. Optional features that
  carry per-instance RAM cost only when used are added by subclasses
  (`SliderWithInsetIcon`, `SliderWithLabel`, etc.).
- Customization that is naturally shared (color overrides, geometry token
  sets, large token tables) is defined as `const`/`constexpr` structs and referenced
  by pointer, defaulting to the active `Theme`.
- Slider-specific interaction events are exposed as virtual no-op hooks
  rather than as stored `std::function` slots, matching the framework's
  existing pattern (`onLayout`, `onTouchCanceled`, `onEditFinished`,
  `onEnter`, etc.).

## Proposed Public API

### Core Types

```cpp
namespace roo_windows {
namespace material3 {

enum class SliderVariant : uint8_t {
  kStandard,
  kCentered,
};

enum class SliderOrientation : uint8_t {
  kHorizontal,
  kVertical,
};

enum class SliderSize : uint8_t {
  kExtraSmall,
  kSmall,
  kMedium,
  kLarge,
  kExtraLarge,
};

enum class SliderValueIndicatorBehavior : uint8_t {
  kHidden,
  kShowOnInteraction,
  kWithinBounds,
  kAlways,
};

enum class SliderTickMode : uint8_t {
  // Ticks are rendered when the slider is discrete (`step > 0`) and hidden
  // otherwise. This is the recommended default.
  kAuto,
  // Ticks are never rendered, even in discrete mode.
  kHidden,
  // Ticks are rendered for every valid step. Only meaningful in discrete mode.
  kShowTicks,
};

struct SliderRange {
  float from = 0.0f;
  float to = 1.0f;
  float step = 0.0f;
};

// Compact, packed semantic style. Designed to fit into ~2 B and to be passed
// by value cheaply.
struct SliderStyle {
  SliderOrientation orientation : 1;
  SliderSize size : 3;
  SliderValueIndicatorBehavior value_indicator : 2;
  SliderTickMode tick_mode : 2;
  bool show_stop_indicators : 1;
  // Reserved bits left for future expansion.
};

// Optional, normally `constexpr` and shared. A non-owning pointer to one
// of these structs lives in the slider; `nullptr` means "use the active
// theme defaults". Putting overrides behind a pointer keeps unconfigured
// sliders at the base ~4 B cost rather than copying ~50+ B per instance.
struct SliderAppearance {
  // Optional value-bearing colors. A field with `has_*` set to false
  // falls back to the theme.
  Color active_track;
  Color inactive_track;
  Color thumb;
  Color tick_active;
  Color tick_inactive;
  Color halo;
  // Geometry overrides. Negative values mean "use theme default".
  int16_t track_height = -1;
  int16_t thumb_width = -1;
  int16_t thumb_height = -1;
  int16_t thumb_track_gap = -1;
  int16_t track_corner_size = -1;
  int16_t track_inside_corner_size = -1;
  int16_t stop_indicator_size = -1;
  // Bitfield indicating which color fields are populated.
  uint8_t has_colors = 0;  // bit 0..5 for the six color fields above
};

// Tiny value type returned by Slider::getInsetIcon(). The dedicated
// `SliderWithInsetIcon` subclass stores one of these inline; custom slider
// subclasses can synthesize one on demand.
struct InsetIcon {
  // Non-owning. The referenced `Pictogram` must outlive the slider that uses
  // it. `nullptr` means "no inset icon".
  const roo_display::Pictogram* icon = nullptr;
  SliderTrackIconAnchor anchor = SliderTrackIconAnchor::kStart;
};

}  // namespace material3
}  // namespace roo_windows
```

### Single-Value Slider

The base `Slider` widget keeps its public API focused on the common path.
Optional features are added either by overriding virtual no-op hooks or by
using a derived class.

```cpp
class Slider : public BasicWidget {
 public:
  Slider(ApplicationContext& context, SliderRange range = {}, float value = 0.0f,
         SliderVariant variant = SliderVariant::kStandard,
         SliderStyle style = {});

  // --- Semantic configuration. ---
  const SliderRange& range() const;
  bool setRange(SliderRange range);

  SliderVariant variant() const;
  bool setVariant(SliderVariant variant);

  SliderStyle style() const;
  bool setStyle(SliderStyle style);

  // --- Value access. ---
  float value() const;
  bool setValue(float value);

  // --- Optional shared appearance overrides. ---
  // Non-owning. The pointed-to struct must outlive the slider, or be cleared
  // with setAppearance(nullptr) before destruction. Defaults to nullptr,
  // which means "use the active theme".
  void setAppearance(const SliderAppearance* appearance);
  const SliderAppearance* appearance() const;

  // --- Optional interaction lifecycle hooks. ---
  // Override in a subclass. Default implementations are empty and cost zero
  // bytes per instance. setOnInteractiveChange() (inherited from Widget)
  // continues to fire for user-originated value changes.
  virtual void onValueChange(float value, bool from_user) {}
  virtual void onInteractionStart() {}
  virtual void onInteractionEnd(float value) {}

  // --- Optional value label formatting. ---
  // Override in a subclass to customize the value indicator text. The default
  // implementation writes a compact decimal representation of `value` into
  // `scratch` and returns a view into it. Always allocation-free.
  virtual roo_display::StringView formatLabel(
      float value, char* scratch, size_t scratch_size) const;

 private:
  // Approximate per-instance layout (ESP32, 32-bit):
  //   BasicWidget                  ~28 B (vptr, context*, parent*, Rect,
  //                                   state, padding/margins)
  //   float value_                    4 B
  //   SliderRange range_             12 B (3 floats)
  //   const SliderAppearance* app_    4 B  (nullptr unless overridden)
  //   SliderVariant variant_          1 B
  //   SliderStyle style_              2 B  (packed bitfield)
  //   bool is_dragging_               1 B  (or folded into Widget::state_)
  //   alignment padding              ~0-4 B
  //   ---------------------------------
  //   Total:                        ~52-56 B
  //
  // Adding a SliderAppearance override or overriding any virtual hook
  // does not increase per-instance size.
};
```

### Single-Value Slider with Inset Icon

Icons are an optional feature that costs RAM only when actually used.

```cpp
// Adds Material 3's optional inset icon.
// Per-instance overhead beyond Slider: ~8 B (one icon pointer + anchor,
// stored inline because sliders are unlikely to share inset-icon config).
class SliderWithInsetIcon : public Slider {
 public:
  using Slider::Slider;

  // Non-owning. The pointed-to pictogram must outlive this widget.
  void setIcon(const roo_display::Pictogram* icon,
               SliderTrackIconAnchor anchor = SliderTrackIconAnchor::kStart);

 private:
  InsetIcon icon_;  // ~8 B
};
```

### Range Slider

```cpp
class RangeSlider : public BasicWidget {
 public:
  RangeSlider(ApplicationContext& context, SliderRange range,
              float start_value, float end_value,
              SliderStyle style = {});

  const SliderRange& range() const;
  bool setRange(SliderRange range);

  SliderStyle style() const;
  bool setStyle(SliderStyle style);

  float startValue() const;
  float endValue() const;
  bool setValues(float start_value, float end_value);

  float minSeparation() const;
  bool setMinSeparation(float value);

  // Returns 0 for the start thumb, 1 for the end thumb, or -1 when no thumb
  // is currently being interacted with.
  int activeThumbIndex() const;

  void setAppearance(const SliderAppearance* appearance);
  const SliderAppearance* appearance() const;

  // Optional virtual hooks; default to no-op.
  virtual void onValueChange(float start, float end, int active_thumb,
                             bool from_user) {}
  virtual void onInteractionStart(int active_thumb) {}
  virtual void onInteractionEnd(float start, float end) {}

  virtual roo_display::StringView formatLabel(
      float value, char* scratch, size_t scratch_size) const;

 private:
  // Approximate per-instance layout (ESP32):
  //   BasicWidget                  ~40 B
  //   float start_value_              4 B
  //   float end_value_                4 B
  //   float min_separation_           4 B
  //   SliderRange range_             12 B
  //   const SliderAppearance* app_    4 B
  //   SliderStyle style_              2 B
  //   int8_t active_thumb_            1 B
  //   uint8_t flags_                  1 B  (is_dragging, etc.)
  //   alignment padding              ~4 B
  //   ---------------------------------
  //   Total:                        ~76 B
};
```

### Per-Instance Footprint Summary

| Widget                       | ESP32 size | Compared to current `Slider` |
|------------------------------|-----------:|-----------------------------:|
| Existing `material3::Slider` |     ~44 B  |                          ref |
| New `Slider` (base)          |     ~68 B  |                       +24 B  |
| `SliderWithInsetIcon`        |     ~76 B  |                       +32 B  |
| `RangeSlider`                |     ~76 B  |                       +32 B  |

The extra ~24 B on the base `Slider` pays for the semantic value domain
(`float value_` + `SliderRange`) and the slot for an optional shared
appearance pointer. A naive translation of the Android setter matrix
(value-stored inset-icon config + value-stored `SliderAppearanceOverrides`
with ~13 `std::optional` fields + three `std::function` callbacks) would
add well over 150 B per instance even in unconfigured sliders, which is not
acceptable for this library.

## Key Decisions

### 1. Public Values Move to a Real Domain

The primary API should use semantic values, not only normalized positions.

Rationale:

- Material 3 and Android both define sliders in terms of a value range,
- step validation belongs with the slider, not every caller,
- range sliders need domain-aware validation anyway,
- keeping semantic values end-to-end avoids quantization surprises and removes
  a compatibility layer the range slider never needed.

### 2. Range Slider Is a Separate Class

`RangeSlider` should not be a mode of `Slider`.

Rationale:

- the semantic payload differs,
- the virtual hook signatures differ,
- minimum separation and active-thumb handling are range-only concerns,
- it mirrors Android and keeps call sites more readable,
- it avoids paying for a second value's storage in every single-value slider.

### 3. Centered Mode Stays on `Slider`

Centered mode is a single-value semantic variant, not a separate widget.

Its behavior is:

- the visual active track grows from the center anchor,
- the value domain may still be arbitrary,
- zero does not need to be hard-coded, but the centered anchor defaults to the
  midpoint of `range.from` and `range.to`,
- the center anchor is purely visual and is not required to coincide with a
  valid step in discrete mode.

Future extension can add an explicit `center_value` if a real product case
needs it, but the initial API should not overfit that case.

### 4. Optional Features Are Added by Subclassing or Virtual Hooks, Not Stored Slots

The base `Slider` and `RangeSlider` classes should not pay RAM for features
that most instances never use.

Applied here:

- inset icons live on a derived `SliderWithInsetIcon` class (~8 B for one
  per-instance icon pointer plus anchor), not on the base. Sliders without
  inset icons pay zero,
- value indicator label formatting is a virtual `formatLabel()` method on
  the widget itself. Customization happens by subclassing, not by storing a
  `SliderLabelFormatter*`. Cost when not customized: zero,
- interaction-lifecycle events (`onValueChange`, `onInteractionStart`,
  `onInteractionEnd`) are virtual no-op methods, not stored
  `std::function`s. Three `std::function` slots would have cost ~48 B per
  instance even when empty,
- `setOnInteractiveChange()` (already on `Widget`) keeps working unchanged
  and remains the cheap path for callers that want to stay with lambdas.

This matches the framework's existing convention: see `onLayout`,
`onTouchCanceled`, `onEditFinished`, `onEnter`, `onExit`, etc.

### 5. Appearance Overrides Are Shared, Not Copied

Appearance overrides should be referenced by pointer to a const, often
`constexpr`/`PROGMEM` `SliderAppearance` struct, not stored by value.

Rationale:

- a UI typically has at most a handful of distinct slider appearances;
  copying ~50+ B into every slider instance is wasteful,
- the same `SliderAppearance` instance can be shared across many sliders
  without per-instance cost beyond a single 4 B pointer,
- `nullptr` means "use the active theme", which is also the natural place
  for library-wide defaults to live,
- this mirrors how `Theme` itself is consumed via `Environment` rather than
  copied per widget.

When a single slider needs a one-off override, the application can construct
a local `static constexpr SliderAppearance` next to the widget and point at
it; the struct lives in flash, not RAM.

### 6. Visual Style Stays Inline as a Compact Bitfield

Unlike appearance overrides, the small enums on `SliderStyle` (orientation,
size, value-indicator behavior, tick mode, stop-indicator policy) are part
of the slider's identity and read on every paint. Storing them as a packed
2 B bitfield inside the widget is cheaper than indirection, and they do not
benefit from sharing.

### 7. Value Labels Use a Virtual Method, Not a Pointer to a Formatter Object

Dragging a slider can update every frame, so the label formatter must not
allocate per-change.

The formatter is exposed as a virtual `formatLabel(value, scratch, n)`
method that writes into caller-provided scratch storage and returns a
lightweight view. Customization happens by subclassing, which costs zero
per-instance bytes when not used (vs. ~4 B for a stored
`SliderLabelFormatter*`).

### 8. Slider-Specific Lifecycle Hooks Complement, Not Replace, `setOnInteractiveChange()`

Existing code already relies on `Widget::setOnInteractiveChange()`. That
callback should continue to fire for user-originated value changes.

The new slider-specific virtual hooks add the missing Android-like
lifecycle:

1. start of interaction,
2. value changes with `from_user`,
3. end of interaction.

This avoids forcing a framework-wide callback redesign just to support
slider behavior, and keeps the per-instance cost at zero for sliders that
only need the existing `setOnInteractiveChange()` path.

## Behavior Details

### Discrete Mode

- `step == 0` means continuous mode.
- `step > 0` means discrete mode.
- `step < 0` is invalid.
- The slider validates that `step` evenly divides `range.to - range.from`
  within a small numeric tolerance.
- Programmatic values are clamped to the range and snapped to the nearest valid
  step when discrete mode is enabled.
- `setRange()` clamps the current value into the new domain and re-snaps it to
  the new step grid in discrete mode. For `RangeSlider`, both thumb values are
  clamped and re-ordered if the new domain inverts them, and `minSeparation`
  is itself clamped to the new domain width.

### Range Slider Validation

- `range.from < range.to` is required.
- `start_value <= end_value` is required.
- both values must fall within the configured range,
- both values must land on valid steps in discrete mode,
- `minSeparation` is validated in domain units,
- when two thumbs overlap at touch-down, thumb selection waits for directional
  intent before locking the active thumb, matching Android behavior.

### Value Indicator

- hidden by default,
- shown according to `SliderValueIndicatorBehavior`,
- for `RangeSlider`, only the active thumb shows a value indicator,
- indicators that need to escape ancestor clipping should paint through the
  shared [transient_presentation_pins_design.md](../proposed/transient_presentation_pins_design.md)
  path rather than by requiring a full `ParentClipMode::kUnclipped` ancestor
  chain,
- if no custom formatter is set, a compact default decimal formatter is used.

### Orientation

- horizontal remains the default,
- vertical reverses the primary drag axis,
- standard and centered sliders support both orientations,
- range sliders should also support both orientations for API completeness, but
  the documentation should note that vertical range sliders are discouraged by
  Material guidance.

### Size Presets

`SliderSize` should switch the geometry token set used for:

- track height,
- track corner radius,
- handle height,
- value-indicator placement,
- icon allowance and icon size.

The default remains `kExtraSmall`, preserving the current visual footprint.

At the travel extremes, the thumb center should align to the terminal stop
mark, not to the widget edge or to half the thumb width. In the current
Material 3 geometry this means the endpoint center sits `Scaled(6)` from the
widget edge, not `Scaled(2)` for a `Scaled(4)` handle. This is backed by
observation of stop-mark placement and endpoint behavior in the Material Figma
assets. At those extremes only the inboard side of the track should remain
visible: the visual gap from the widget edge to the handle edge is
`Scaled(4)`, while the track-to-handle gap remains `Scaled(6)`.

### Track Icons

The API should cover Material 3's inset icon.

Rules:

- inset icons are only valid for standard sliders,
- inset icons are only painted on M, L, and XL sizes,
- centered and range sliders ignore `icons.inset`,
- the icon is anchored at the track start in widget layout coordinates and may
  jump past the thumb when needed to preserve legibility.

## Non-Goals for the First Implementation

The first implementation does not need to ship every Android appearance detail.

The following can be explicitly deferred as long as the API leaves room for
them:

- per-thumb custom drawable arrays,
- thumb stroke and elevation overrides,
- independently configurable active and inactive tick radii,
- every separate track icon tint setter,
- keyboard focus movement APIs beyond what the base framework already supports.

Those are useful, but they are secondary to the missing semantic surface.

## Migration and Compatibility

Existing code such as:

```cpp
material3::Slider slider(context, 32768);
```

continues to work through Steps 0-12. Step 13 then removes the shim.

Behavior during Steps 0-12:

- `Slider(context)` remains the unit-range convenience form,
- `setOnInteractiveChange()` keeps its current meaning, firing only for
  user-originated changes,
- programmatic `setValue()` updates fire `onValueChange()`
  with `from_user == false` and do not fire the interaction lifecycle
  hooks or `setOnInteractiveChange()`.

After Step 13:

- callers use `SliderRange` and `value()` / `setValue()` exclusively.

## Implementation Plan

The implementation should be split into small, shippable increments. Each step
should leave the slider in a usable state and avoid mixing semantic API changes
with visual expansion unless the latter depends directly on the former.

### Step 0: Lock Down the Current Baseline

Before changing behavior, add or update tests for the current widget so the
refactor has a stable safety net.

Scope:

1. preserve current normalized `pos_` mapping behavior,
2. preserve current tap-to-jump and drag behavior,
3. preserve current preferred-size and paint expectations,
4. preserve `setOnInteractiveChange()` firing semantics.

Deliverable:

- no public API change yet,
- a baseline test suite that fails if the compatibility path regresses.

Incremental per-instance RAM cost: **0 B** (no fields added).

Example usage after this step:

```cpp
material3::Slider slider(context);
slider.setValue(0.75f);
```

```cpp
material3::Slider brightness(context);
brightness.setOnInteractiveChange([]() { LOG(INFO) << "Brightness changed"; });
```

### Step 1: Extract a Shared Internal Slider Model

Move the geometry, value normalization, and gesture bookkeeping into an
internal implementation layer that can support one or two thumbs, but keep the
public widget unchanged in this step.

Scope:

1. separate semantic value handling from paint code,
2. represent track axis and thumb hit-testing in a shared helper,
3. keep `Slider(context)` as the default unit-range constructor.

Deliverable:

- no new user-visible features,
- lower-risk foundation for subsequent API additions,
- tests proving the refactor preserves legacy behavior.

Incremental per-instance RAM cost: **0 B** (refactor only). Total Slider
footprint: ~44 B.

Example usage after this step:

```cpp
material3::Slider slider(context);
slider.setValue(0.5f);
```

```cpp
material3::Slider volume(context, material3::SliderRange{}, 0.76f);
volume.setOnInteractiveChange([]() { /* Existing callback behavior unchanged. */ });
```

### Step 2: Add Semantic Value Range to `Slider`

Introduce the real-domain API for the single-value slider while keeping the
normalized compatibility API intact.

Scope:

1. add `SliderRange`, `value()`, `setValue()`, `range()`, and `setRange()`,
2. clamp invalid programmatic input into the configured domain,
3. make `Slider(context)` the explicit unit-range convenience form.

Deliverable:

- existing callers continue to compile unchanged,
- new callers can stop doing their own float-to-`uint16_t` mapping,
- tests cover value-to-position round-tripping.

Incremental per-instance RAM cost: **+16 B** (`float value_` 4 B +
`SliderRange range_` 12 B), replacing the existing 2 B `pos_`. Total Slider
footprint: ~58 B.

Example usage after this step:

```cpp
material3::Slider temperature(
  context, material3::SliderRange{10.0f, 40.0f}, 28.5f);
temperature.setValue(30.0f);
```

```cpp
material3::Slider ph(context, material3::SliderRange{6.8f, 8.0f}, 7.2f);
ph.setRange(material3::SliderRange{6.5f, 8.5f});
```

### Step 3: Add Discrete Stepping and Validation

Once the single-value widget has a real domain, add discrete mode via
`SliderRange.step` and keep all snapping logic local to the widget.

Scope:

1. validate `step == 0` as continuous and `step > 0` as discrete,
2. reject invalid ranges or incompatible step sizes,
3. snap programmatic values and user drags to valid steps,
4. decide and document the numeric tolerance used for divisibility checks.

Deliverable:

- continuous and discrete `Slider` variants both work,
- snapping logic is covered before range or value labels are added.

Incremental per-instance RAM cost: **0 B** (`step` already lives in
`SliderRange`).

Example usage after this step:

```cpp
material3::Slider fan_speed(
  context, material3::SliderRange{0.0f, 5.0f, 1.0f}, 2.0f);
fan_speed.setValue(3.6f);  // Snaps to 4.0f.
```

```cpp
material3::Slider setpoint(
  context, material3::SliderRange{18.0f, 30.0f, 0.5f}, 24.0f);
```

### Step 4: Add Centered Single-Value Semantics

Extend `Slider` with `SliderVariant::kCentered` after the basic semantic value
model is proven.

Scope:

1. fill the active track from the center anchor instead of the minimum edge,
2. keep the thumb motion and value mapping identical to standard sliders,
3. default the center anchor to the midpoint of the configured range,
4. ensure the centered variant works in both continuous and discrete modes.

Deliverable:

- `Slider` now covers both standard and centered Material 3 single-value
  variants,
- no range-slider complexity is introduced yet.

Incremental per-instance RAM cost: **+1 B** (`SliderVariant variant_`,
likely absorbed by existing alignment padding for net 0 B). Total Slider
footprint: ~58-60 B.

Example usage after this step:

```cpp
material3::Slider trim(
  context, material3::SliderRange{-5.0f, 5.0f, 0.5f}, 0.0f,
  material3::SliderVariant::kCentered);
```

```cpp
material3::Slider balance(
  context, material3::SliderRange{-100.0f, 100.0f}, -20.0f,
  material3::SliderVariant::kCentered);
```

### Step 5: Add Slider-Specific Interaction Lifecycle

Add explicit start and end virtual hooks after the value model is stable, so
the new interaction API lands on top of already-correct value updates.

Scope:

1. add `virtual void onValueChange(float, bool)`,
   `virtual void onInteractionStart()`, and
   `virtual void onInteractionEnd(float)` to `Slider`, all defaulting to
   no-op,
2. keep `setOnInteractiveChange()` firing for compatibility,
3. ensure callback order is deterministic,
4. define whether programmatic changes trigger only `onValueChange` or no
   interaction lifecycle at all.

Deliverable:

- Android-like interaction lifecycle for the single-value slider,
- backward compatibility for existing widget-level listeners.

Incremental per-instance RAM cost: **0 B** (virtual no-op hooks share the
existing `vptr`; no per-instance storage is added). This is the key win
over storing three `std::function` slots, which would have added ~48 B.

Example usage after this step:

```cpp
class VolumeSlider : public material3::Slider {
 public:
  using material3::Slider::Slider;
  void onInteractionStart() override { LOG(INFO) << "Volume drag start"; }
  void onValueChange(float value, bool from_user) override {
    if (from_user) audio.setVolume(value);
  }
};
```

```cpp
class PersistedVolumeSlider : public material3::Slider {
 public:
  using material3::Slider::Slider;
  void onInteractionEnd(float value) override { prefs::save_volume(value); }
};
```

### Step 6: Add `RangeSlider` Core Semantics

With the shared engine and single-value semantics in place, introduce the new
two-thumb widget as a separate class.

Scope:

1. add `RangeSlider` construction and storage for two semantic values,
2. reuse the shared internal axis and normalization logic,
3. expose `startValue()`, `endValue()`, and `setValues()`,
4. guarantee ordered values and shared-range validation.

Deliverable:

- the library now supports Material 3's range variant,
- the new widget reuses the same value and gesture foundation rather than
  duplicating logic.

Per-instance RAM cost: a `RangeSlider` instance is approximately ~76 B (see
the per-instance footprint summary above). Existing `Slider` instances are
not affected.

Example usage after this step:

```cpp
material3::RangeSlider quiet_hours(
  context, material3::SliderRange{0.0f, 24.0f, 0.5f}, 6.0f, 22.0f);
```

```cpp
material3::RangeSlider target_band(
  context, material3::SliderRange{24.0f, 34.0f}, 27.0f, 30.5f);
target_band.setValues(26.5f, 31.0f);
```

### Step 7: Add Active-Thumb and Minimum-Separation Behavior

Once `RangeSlider` exists, complete the Android-relevant interaction semantics
that are specific to multiple thumbs.

Scope:

1. expose `activeThumbIndex()` and range-specific virtual interaction hooks
   (`onValueChange`, `onInteractionStart`, `onInteractionEnd`),
2. add `minSeparation()` and `setMinSeparation()`,
3. implement overlap resolution and directional-intent thumb selection,
4. ensure discrete mode and minimum separation interact predictably.

Deliverable:

- range dragging feels intentional rather than ambiguous,
- applications can enforce meaningful minimum intervals.

Incremental per-instance RAM cost on `RangeSlider`: **+4 B** for
`min_separation_` (already accounted for in the ~76 B summary above; the
active thumb index and dragging flag share an existing flags byte).

Example usage after this step:

```cpp
material3::RangeSlider operating_band(
  context, material3::SliderRange{10.0f, 40.0f, 0.5f}, 22.0f, 30.0f);
operating_band.setMinSeparation(2.0f);
```

```cpp
class OperatingBandSlider : public material3::RangeSlider {
 public:
  using material3::RangeSlider::RangeSlider;
  void onValueChange(float start, float end, int active_thumb,
                     bool from_user) override {
    LOG(INFO) << "Thumb " << active_thumb << ": " << start << " - " << end;
  }
};
```

### Step 8: Add Value Indicators and Label Formatting

Add value labels only after the semantic and interaction layers are stable,
since labels depend on both the active thumb and the current interaction state.

Scope:

1. add `SliderValueIndicatorBehavior` (already part of the packed
   `SliderStyle` bitfield),
2. add the virtual `formatLabel(value, scratch, n)` method on `Slider` and
   `RangeSlider`, with an allocation-free decimal default,
3. show only the active thumb label for `RangeSlider`,
4. define indicator layout and clipping rules for small slider sizes.

Deliverable:

- both widgets can display values during interaction,
- product code can customize formatting without per-frame allocations and
  without paying for a stored formatter pointer.

Incremental per-instance RAM cost: **0 B**. The indicator behavior is part
of the already-allocated `SliderStyle` bitfield, and label formatting is a
virtual method that costs zero per instance until overridden.

Example usage after this step:

```cpp
material3::SliderStyle style{};
style.value_indicator =
  material3::SliderValueIndicatorBehavior::kShowOnInteraction;
material3::Slider temperature(context, material3::SliderRange{20.0f, 40.0f}, 28.0f,
                material3::SliderVariant::kStandard, style);
```

```cpp
class CelsiusSlider : public material3::Slider {
 public:
  using material3::Slider::Slider;
  roo_display::StringView formatLabel(
      float value, char* scratch, size_t scratch_size) const override;
};
```

### Step 9: Add Orientation Support

Orientation should come after the core gesture semantics are done, because it
widens every hit-test and drag-path branch.

Scope:

1. add `SliderOrientation`,
2. implement vertical measurement, painting, and drag handling,
3. validate orientation-specific thumb picking for both `Slider` and
  `RangeSlider`,
4. document that vertical range sliders are supported but discouraged.

Deliverable:

- the API now covers the Material 3 orientation surface,
- gesture logic remains shared rather than forked per widget.

Incremental per-instance RAM cost: **0 B** (orientation lives in the
`SliderStyle` bitfield).

Example usage after this step:

```cpp
material3::SliderStyle style{};
style.orientation = material3::SliderOrientation::kVertical;
material3::Slider tank_level(context, material3::SliderRange{0.0f, 100.0f}, 62.0f,
                             material3::SliderVariant::kStandard, style);
```

```cpp
material3::SliderStyle style{};
style.orientation = material3::SliderOrientation::kVertical;
material3::RangeSlider humidity_band(
  context, material3::SliderRange{0.0f, 100.0f}, 35.0f, 55.0f, style);
```

### Step 10: Add Size Presets, Ticks, Stop Indicators, and Track Icons

After the semantic and gesture surface is complete, expand the visual feature
set in one pass around a stable style struct, and add the optional
`SliderWithInsetIcon` subclass.

Scope:

1. add `SliderSize`, `SliderTickMode`, and stop-indicator policy (already
   part of the packed `SliderStyle` bitfield),
2. define geometry tokens for XS through XL, stored as shared `constexpr`
   tables in flash and selected by `SliderSize`,
3. introduce `material3::SliderWithInsetIcon` with one inline inset-icon
  descriptor,
4. restrict icon rendering to variants and sizes where it fits,
5. verify that discrete ticks render consistently with snapping behavior.

Deliverable:

- the slider family now covers the major missing Material 3 visual variants,
- styling still flows through a compact configuration surface plus a
  dedicated inset-icon subclass that keeps the base widget lean.

Incremental per-instance RAM cost:

- on the base `Slider`: **0 B** (size/tick/stop policy live in the existing
  `SliderStyle` bitfield),
- on `SliderWithInsetIcon`: **+8 B** for one inline icon pointer plus anchor.

Example usage after this step:

```cpp
material3::SliderStyle style{};
style.size = material3::SliderSize::kMedium;
style.tick_mode = material3::SliderTickMode::kShowTicks;
material3::Slider speed(context, material3::SliderRange{0.0f, 10.0f, 1.0f}, 4.0f,
                        material3::SliderVariant::kStandard, style);
```

```cpp
material3::SliderStyle style{};
style.size = material3::SliderSize::kExtraLarge;
material3::SliderWithInsetIcon heating(
    context, material3::SliderRange{18.0f, 30.0f}, 24.0f,
    material3::SliderVariant::kStandard, style);
heating.setIcon(&kThermostatDrawable);
```

### Step 11: Add Optional Appearance Overrides

Appearance overrides should be last, after the semantic API and tokenized style
model are proven stable.

Scope:

1. introduce the `SliderAppearance` struct as a shared, normally
   `constexpr` configuration object,
2. add a `setAppearance(const SliderAppearance*)` slot on `Slider` and
   `RangeSlider` (default `nullptr` falls back to the active `Theme`),
3. expose only the override points that are justified by real product
   needs,
4. keep token defaults as the primary path,
5. avoid expanding override support into separate per-state object graphs.

Deliverable:

- advanced callers can customize the widget beyond Material defaults,
- the common API remains small and embedded-friendly,
- a single `SliderAppearance` instance can be shared by any number of
  sliders without duplication.

Incremental per-instance RAM cost: **+4 B** (one `const SliderAppearance*`
slot, already accounted for in the per-instance footprint summary). The
struct it points to lives in flash when declared `constexpr`.

Example usage after this step:

```cpp
static constexpr material3::SliderAppearance kBlueOrangeSlider{
  /*active_track=*/roo_display::color::Blue,
  /*inactive_track=*/{},
  /*thumb=*/roo_display::color::Orange,
  // ...remaining fields default.
  /*has_colors=*/0b000101,  // active_track + thumb
};
material3::Slider slider(context, material3::SliderRange{0.0f, 100.0f}, 50.0f);
slider.setAppearance(&kBlueOrangeSlider);
```

```cpp
static constexpr material3::SliderAppearance kCompactRangeSlider{
  // colors default,
  /*track_height=*/8,
  // ...
  /*stop_indicator_size=*/6,
};
material3::RangeSlider range_slider(
  context, material3::SliderRange{0.0f, 10.0f, 1.0f}, 2.0f, 7.0f);
range_slider.setAppearance(&kCompactRangeSlider);
```

### Step 12: Finish Documentation and Example Coverage

Every public step above should land with tests, but this step should make
the new feature family discoverable and easy to adopt.

Scope:

1. expand the Material 3 slider example to show each variant,
2. document migration from normalized `pos_` usage to semantic values,
3. document which features are single-value only, range-only, or size-limited,
4. add short examples for label formatting and range minimum separation.

Deliverable:

- the feature family is discoverable from both API and adoption perspectives,
- callers can migrate incrementally before the compatibility shims are
  removed in Step 13.

Incremental per-instance RAM cost: **0 B** (documentation and examples).

Example usage after this step:

```cpp
material3::Slider legacy(context, 32768);
material3::Slider migrated(context, material3::SliderRange{0.0f, 1.0f}, 0.5f);
```

```cpp
material3::RangeSlider schedule(
  context, material3::SliderRange{0.0f, 24.0f, 1.0f}, 8.0f, 18.0f);
schedule.setMinSeparation(2.0f);
```

### Step 13: Remove Compatibility Shims

After all callers have migrated to the semantic API (verified in-repo since
the library owner controls all usages), remove the legacy normalized-position
surface so the slider family carries no long-term compatibility cost.

Scope:

1. remove the `Slider(ApplicationContext& context, uint16_t pos)` constructor,
2. remove `getPos()` and `setPos()` from `Slider`,
3. remove the special-case handling that treated the legacy constructor as
   `SliderRange{0.0f, 1.0f}`,
4. delete tests that exclusively cover the legacy normalized API,
5. update the example at `examples/material3/slider/slider.ino` to use only
   the semantic API,
6. note the breaking change in the changelog.

Deliverable:

- a single, semantic public API surface,
- no compatibility shims to maintain or document going forward.

Incremental per-instance RAM cost: **negligible, possibly slightly
negative**. The widget no longer needs to retain any normalized
`uint16_t pos_` mirror, and small amounts of conversion code can be removed
from flash.

Example usage after this step:

```cpp
material3::Slider brightness(context, material3::SliderRange{0.0f, 1.0f}, 0.5f);
brightness.setValue(0.75f);
```

```cpp
material3::RangeSlider quiet_hours(
  context, material3::SliderRange{0.0f, 24.0f, 0.5f}, 22.0f, 6.0f + 24.0f);
```

### Suggested Review Boundaries

For code review and release management, the steps above should be grouped into
small PR-sized increments:

1. baseline tests plus internal refactor,
2. semantic single-value API plus discrete mode,
3. centered mode plus slider lifecycle hooks,
4. range slider plus active-thumb and separation logic,
5. value indicators plus orientation,
6. size presets, ticks, stop indicators, and `SliderWithInsetIcon`,
7. shared appearance overrides, examples, and final documentation,
8. removal of compatibility shims (final, separately reviewed breaking
   change).

## Test Plan

The implementation should add focused tests for:

1. `value` to normalized-position mapping,
2. continuous and discrete snapping,
3. invalid range and step configuration rejection,
4. centered active-track geometry,
5. range thumb picking and overlap resolution,
6. minimum separation enforcement,
7. value-indicator visibility rules,
8. orientation-aware drag behavior,
9. size-token measurement and preferred-size output.

The example at `examples/material3/slider/slider.ino` should also be expanded to
demonstrate:

1. standard continuous,
2. standard discrete,
3. centered,
4. range,
5. value indicator formatting,
6. at least one non-`kExtraSmall` size.

## Summary

The recommended direction is:

1. keep `material3::Slider` as the single-value widget,
2. add `material3::RangeSlider` for the range variant,
3. move the public API from normalized position to semantic value ranges,
4. preserve the current normalized API as a compatibility layer through
   Step 12, then remove it in Step 13,
5. keep the base `Slider` near the existing ~44-68 B per-instance footprint
   by adding optional features through subclassing
  (`SliderWithInsetIcon`, custom `formatLabel()` / `onValueChange()`
  overrides) and through shared, normally-`constexpr` configuration objects
  (`SliderAppearance`),
6. expose interaction-lifecycle events as virtual no-op hooks rather than
   stored `std::function` slots,
7. cover Material 3's missing variants and Android's interaction model
   first, and leave room for richer appearance overrides without making the
   first API too large.

That gives `roo_windows` a slider family that is materially closer to Material 3
and Android, while still fitting the library's low-allocation, RAM-first
embedded design constraints.
