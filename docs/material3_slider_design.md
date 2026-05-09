# Roo Windows Material 3 Slider Design

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

As of 2026-05, `roo_windows` has a Material 3 slider implementation, but it is
intentionally narrow.

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

### Compatibility Requirements

1. Preserve source compatibility for existing code that constructs
   `material3::Slider(env, pos)`.
2. Preserve `getPos()` / `setPos()` as normalized compatibility accessors.
3. Allow the implementation to continue using a normalized fixed-point position
   internally, even if the public API moves to semantic values.

### Embedded Constraints

1. Do not require heap allocation for the common slider path.
2. Keep default construction cheap.
3. Prefer grouped configuration structs over dozens of independent virtual or
   stateful objects.
4. Avoid API choices that force dynamic string allocation during dragging.

## Design Overview

The proposed public surface has three layers:

1. semantic configuration,
2. value and interaction API,
3. optional appearance overrides.

The family should be split into two public widgets:

1. `material3::Slider` for standard and centered single-value sliders,
2. `material3::RangeSlider` for range selection.

This follows both Material 3 variant semantics and Android's class split. It is
cleaner than a single widget whose meaning changes based on thumb count.

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
  kShowStops,
};

struct SliderRange {
  float from = 0.0f;
  float to = 1.0f;
  float step = 0.0f;
};

struct SliderTrackIcons {
  // All pointers are non-owning. Each referenced `Drawable` must outlive the
  // slider that uses it. `nullptr` means "no icon in this slot".
  const roo_display::Drawable* inset = nullptr;
  const roo_display::Drawable* active_start = nullptr;
  const roo_display::Drawable* active_end = nullptr;
  const roo_display::Drawable* inactive_start = nullptr;
  const roo_display::Drawable* inactive_end = nullptr;
};

struct SliderStyle {
  SliderOrientation orientation = SliderOrientation::kHorizontal;
  SliderSize size = SliderSize::kExtraSmall;
  SliderValueIndicatorBehavior value_indicator =
      SliderValueIndicatorBehavior::kHidden;
  SliderTickMode tick_mode = SliderTickMode::kAuto;
  // Stop indicators are the small marks Material 3 paints at the ends of the
  // inactive track segment. They are independent from discrete tick marks,
  // which are controlled by `tick_mode`, and are visible on continuous
  // sliders too.
  bool show_stop_indicators = true;
  SliderTrackIcons icons;
};

struct SliderAppearanceOverrides {
  std::optional<roo_display::Color> active_track;
  std::optional<roo_display::Color> inactive_track;
  std::optional<roo_display::Color> thumb;
  std::optional<roo_display::Color> tick_active;
  std::optional<roo_display::Color> tick_inactive;
  std::optional<roo_display::Color> halo;
  std::optional<int16_t> track_height;
  std::optional<int16_t> thumb_width;
  std::optional<int16_t> thumb_height;
  std::optional<int16_t> thumb_track_gap;
  std::optional<int16_t> track_corner_size;
  std::optional<int16_t> track_inside_corner_size;
  std::optional<int16_t> stop_indicator_size;
};

class SliderLabelFormatter {
 public:
  virtual ~SliderLabelFormatter() = default;
  virtual roo_display::StringView format(
      float value, char* scratch, size_t scratch_size) const = 0;
};

}  // namespace material3
}  // namespace roo_windows
```

### Single-Value Slider

```cpp
class Slider : public BasicWidget {
 public:
  explicit Slider(const Environment& env, uint16_t pos = 0);

  Slider(const Environment& env, SliderRange range, float value = 0.0f,
         SliderVariant variant = SliderVariant::kStandard,
         SliderStyle style = {});

  const SliderRange& range() const;
  bool setRange(SliderRange range);

  SliderVariant variant() const;
  bool setVariant(SliderVariant variant);

  const SliderStyle& style() const;
  bool setStyle(SliderStyle style);

  float value() const;
  bool setValue(float value);

  uint16_t getPos() const;
  bool setPos(uint16_t pos);

  // Non-owning. The pointed-to formatter must outlive the slider, or be
  // cleared with `setLabelFormatter(nullptr)` before destruction.
  void setLabelFormatter(const SliderLabelFormatter* formatter);
  bool setAppearanceOverrides(SliderAppearanceOverrides overrides);

  void setOnValueChange(std::function<void(float value, bool from_user)> cb);
  void setOnInteractionStart(std::function<void()> cb);
  void setOnInteractionEnd(std::function<void(float value)> cb);
};
```

### Range Slider

```cpp
class RangeSlider : public BasicWidget {
 public:
  RangeSlider(const Environment& env, SliderRange range,
              float start_value, float end_value,
              SliderStyle style = {});

  const SliderRange& range() const;
  bool setRange(SliderRange range);

  const SliderStyle& style() const;
  bool setStyle(SliderStyle style);

  float startValue() const;
  float endValue() const;
  bool setValues(float start_value, float end_value);

  float minSeparation() const;
  bool setMinSeparation(float value);

  // Returns 0 for the start thumb, 1 for the end thumb, or -1 when no thumb
  // is currently being interacted with.
  int activeThumbIndex() const;

  void setLabelFormatter(const SliderLabelFormatter* formatter);
  bool setAppearanceOverrides(SliderAppearanceOverrides overrides);

  void setOnValueChange(
      std::function<void(float start, float end, int active_thumb,
                         bool from_user)> cb);
  void setOnInteractionStart(std::function<void(int active_thumb)> cb);
  void setOnInteractionEnd(
      std::function<void(float start, float end)> cb);
};
```

## Key Decisions

### 1. Public Values Move to a Real Domain

The primary API should use semantic values, not only normalized positions.

Rationale:

- Material 3 and Android both define sliders in terms of a value range,
- step validation belongs with the slider, not every caller,
- range sliders need domain-aware validation anyway,
- the implementation can still map everything to the current normalized
  `uint16_t` representation internally.

`getPos()` / `setPos()` remain as compatibility APIs and should be documented as
normalized helpers rather than the primary interface.

### 2. Range Slider Is a Separate Class

`RangeSlider` should not be a mode of `Slider`.

Rationale:

- the semantic payload differs,
- the callback signatures differ,
- minimum separation and active-thumb handling are range-only concerns,
- it mirrors Android and keeps call sites more readable.

### 3. Centered Mode Stays on `Slider`

Centered mode is a single-value semantic variant, not a separate widget.

Its behavior is:

- the visual active track grows from the center anchor,
- the value domain may still be arbitrary,
- zero does not need to be hard-coded, but the centered anchor defaults to the
  midpoint of `range.from` and `range.to`,
- the center anchor is purely visual and is not required to coincide with a
  valid step in discrete mode.

Future extension can add an explicit `center_value` if a real product case needs
it, but the initial API should not overfit that case.

### 4. Use Grouped Style Structs Instead of Android's Full Setter Matrix

`roo_windows` should not expose every Android appearance setter directly in the
first version.

Instead, the first public style layer should group the stable concepts:

- orientation,
- size,
- value indicator behavior,
- tick and stop-indicator policy,
- and icon slots.

If a second layer of appearance overrides becomes necessary, it should also be
struct-based, as shown in `SliderAppearanceOverrides` above.

This keeps the public design open to Android-equivalent richness without making
the main semantic API noisy.

### 5. Value Labels Need a Formatter Interface, Not a `std::string` Contract

Dragging a slider can update every frame. The label formatter therefore should
not require allocating a new string on every change.

The formatter interface should write into caller-provided scratch storage and
return a lightweight view. That keeps the common path allocation-free while
still allowing application-defined formatting.

### 6. Slider-Specific Lifecycle Callbacks Should Complement, Not Replace, `setOnInteractiveChange()`

Existing code already relies on `setOnInteractiveChange()`. That callback should
continue to fire for user-originated value changes.

The new slider-specific callbacks add the missing Android-like lifecycle:

1. start of interaction,
2. value changes with `from_user`,
3. end of interaction.

This avoids forcing a framework-wide callback redesign just to support slider
behavior.

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

### Track Icons

The API should cover both:

1. Material 3's inset icon,
2. Android's active/inactive track endpoint icons.

Rules:

- inset icons are only valid for standard sliders,
- inset icons are only painted on M, L, and XL sizes,
- centered and range sliders ignore `icons.inset`,
- endpoint icons are optional and may be used across variants if their geometry
  fits the chosen size.

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
material3::Slider slider(env, 32768);
```

continues to work unchanged.

Compatibility behavior:

- the existing constructor remains,
- it maps to a normalized single-value slider with range `[0.0f, 1.0f]`,
- `getPos()` / `setPos()` continue to work,
- `setOnInteractiveChange()` keeps its current meaning, firing only for
  user-originated changes,
- programmatic `setValue()` / `setPos()` updates fire `setOnValueChange()`
  with `from_user == false` and do not fire the interaction lifecycle
  callbacks or `setOnInteractiveChange()`.

New code should prefer the semantic constructors and `setValue()` / `value()`.

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

Example usage after this step:

```cpp
material3::Slider slider(env, 32768);
slider.setPos(49152);
```

```cpp
material3::Slider brightness(env, 0);
brightness.setOnInteractiveChange([]() { LOG(INFO) << "Brightness changed"; });
```

### Step 1: Extract a Shared Internal Slider Model

Move the geometry, value normalization, and gesture bookkeeping into an
internal implementation layer that can support one or two thumbs, but keep the
public widget unchanged in this step.

Scope:

1. separate semantic value handling from paint code,
2. represent track axis, thumb hit-testing, and normalized position in a shared
  helper,
3. keep `material3::Slider(env, pos)` as the only public constructor for now.

Deliverable:

- no new user-visible features,
- lower-risk foundation for subsequent API additions,
- tests proving the refactor preserves legacy behavior.

Example usage after this step:

```cpp
material3::Slider slider(env, 16384);
slider.setPos(32768);
```

```cpp
material3::Slider volume(env, 50000);
volume.setOnInteractiveChange([]() { /* Existing callback behavior unchanged. */ });
```

### Step 2: Add Semantic Value Range to `Slider`

Introduce the real-domain API for the single-value slider while keeping the
normalized compatibility API intact.

Scope:

1. add `SliderRange`, `value()`, `setValue()`, `range()`, and `setRange()`,
2. clamp invalid programmatic input into the configured domain,
3. map the existing normalized constructor to `[0.0f, 1.0f]`,
4. document `getPos()` / `setPos()` as compatibility helpers.

Deliverable:

- existing callers continue to compile unchanged,
- new callers can stop doing their own float-to-`uint16_t` mapping,
- tests cover value-to-position round-tripping.

Example usage after this step:

```cpp
material3::Slider temperature(
  env, material3::SliderRange{10.0f, 40.0f}, 28.5f);
temperature.setValue(30.0f);
```

```cpp
material3::Slider ph(env, material3::SliderRange{6.8f, 8.0f}, 7.2f);
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

Example usage after this step:

```cpp
material3::Slider fan_speed(
  env, material3::SliderRange{0.0f, 5.0f, 1.0f}, 2.0f);
fan_speed.setValue(3.6f);  // Snaps to 4.0f.
```

```cpp
material3::Slider setpoint(
  env, material3::SliderRange{18.0f, 30.0f, 0.5f}, 24.0f);
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

Example usage after this step:

```cpp
material3::Slider trim(
  env, material3::SliderRange{-5.0f, 5.0f, 0.5f}, 0.0f,
  material3::SliderVariant::kCentered);
```

```cpp
material3::Slider balance(
  env, material3::SliderRange{-100.0f, 100.0f}, -20.0f,
  material3::SliderVariant::kCentered);
```

### Step 5: Add Slider-Specific Interaction Lifecycle

Add explicit start and end callbacks after the value model is stable, so the
new interaction API lands on top of already-correct value updates.

Scope:

1. add `setOnValueChange()`, `setOnInteractionStart()`, and
  `setOnInteractionEnd()` for `Slider`,
2. keep `setOnInteractiveChange()` firing for compatibility,
3. ensure callback order is deterministic,
4. define whether programmatic changes trigger only value-change callbacks or no
  interaction lifecycle at all.

Deliverable:

- Android-like interaction lifecycle for the single-value slider,
- backward compatibility for existing widget-level listeners.

Example usage after this step:

```cpp
material3::Slider volume(env, material3::SliderRange{0.0f, 100.0f}, 35.0f);
volume.setOnInteractionStart([]() { LOG(INFO) << "Volume drag start"; });
volume.setOnValueChange([](float value, bool from_user) {
  if (from_user) audio.setVolume(value);
});
```

```cpp
material3::Slider volume(env, material3::SliderRange{0.0f, 100.0f}, 35.0f);
volume.setOnInteractionEnd([](float value) {
  prefs::save_volume(value);
});
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

Example usage after this step:

```cpp
material3::RangeSlider quiet_hours(
  env, material3::SliderRange{0.0f, 24.0f, 0.5f}, 6.0f, 22.0f);
```

```cpp
material3::RangeSlider target_band(
  env, material3::SliderRange{24.0f, 34.0f}, 27.0f, 30.5f);
target_band.setValues(26.5f, 31.0f);
```

### Step 7: Add Active-Thumb and Minimum-Separation Behavior

Once `RangeSlider` exists, complete the Android-relevant interaction semantics
that are specific to multiple thumbs.

Scope:

1. expose `activeThumbIndex()` and range-specific interaction callbacks,
2. add `minSeparation()` and `setMinSeparation()`,
3. implement overlap resolution and directional-intent thumb selection,
4. ensure discrete mode and minimum separation interact predictably.

Deliverable:

- range dragging feels intentional rather than ambiguous,
- applications can enforce meaningful minimum intervals.

Example usage after this step:

```cpp
material3::RangeSlider operating_band(
  env, material3::SliderRange{10.0f, 40.0f, 0.5f}, 22.0f, 30.0f);
operating_band.setMinSeparation(2.0f);
```

```cpp
material3::RangeSlider operating_band(
    env, material3::SliderRange{10.0f, 40.0f, 0.5f}, 22.0f, 30.0f);
operating_band.setOnValueChange(
    [](float start, float end, int active_thumb, bool from_user) {
      LOG(INFO) << "Thumb " << active_thumb << ": " << start << " - " << end;
    });
```

### Step 8: Add Value Indicators and Label Formatting

Add value labels only after the semantic and interaction layers are stable,
since labels depend on both the active thumb and the current interaction state.

Scope:

1. add `SliderValueIndicatorBehavior`,
2. add `SliderLabelFormatter`,
3. show only the active thumb label for `RangeSlider`,
4. provide a default allocation-free formatter path,
5. define indicator layout and clipping rules for small slider sizes.

Deliverable:

- both widgets can display values during interaction,
- product code can customize formatting without per-frame allocations.

Example usage after this step:

```cpp
material3::SliderStyle style;
style.value_indicator =
  material3::SliderValueIndicatorBehavior::kShowOnInteraction;
material3::Slider temperature(env, material3::SliderRange{20.0f, 40.0f}, 28.0f,
                material3::SliderVariant::kStandard, style);
```

```cpp
class CelsiusFormatter : public material3::SliderLabelFormatter {
 public:
  roo_display::StringView format(
    float value, char* scratch, size_t scratch_size) const override;
};

material3::Slider temperature(env, material3::SliderRange{20.0f, 40.0f}, 28.0f);
CelsiusFormatter celsius_formatter;
temperature.setLabelFormatter(&celsius_formatter);
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

Example usage after this step:

```cpp
material3::SliderStyle style;
style.orientation = material3::SliderOrientation::kVertical;
material3::Slider tank_level(env, material3::SliderRange{0.0f, 100.0f}, 62.0f,
                             material3::SliderVariant::kStandard, style);
```

```cpp
material3::SliderStyle style;
style.orientation = material3::SliderOrientation::kVertical;
material3::RangeSlider humidity_band(
  env, material3::SliderRange{0.0f, 100.0f}, 35.0f, 55.0f, style);
```

### Step 10: Add Size Presets, Ticks, Stop Indicators, and Icons

After the semantic and gesture surface is complete, expand the visual feature
set in one pass around a stable style struct.

Scope:

1. add `SliderSize`, `SliderTickMode`, and stop-indicator policy,
2. define geometry tokens for XS through XL,
3. add inset-icon and track-endpoint icon slots,
4. restrict icon rendering to variants and sizes where it fits,
5. verify that discrete ticks render consistently with snapping behavior.

Deliverable:

- the slider family now covers the major missing Material 3 visual variants,
- styling still flows through a compact configuration surface instead of many
  one-off setters.

Example usage after this step:

```cpp
material3::SliderStyle style;
style.size = material3::SliderSize::kMedium;
style.tick_mode = material3::SliderTickMode::kShowStops;
style.show_stop_indicators = true;
material3::Slider speed(env, material3::SliderRange{0.0f, 10.0f, 1.0f}, 4.0f,
                        material3::SliderVariant::kStandard, style);
```

```cpp
material3::SliderStyle style;
style.size = material3::SliderSize::kExtraLarge;
style.icons.inset = &kThermostatDrawable;
material3::Slider heating(env, material3::SliderRange{18.0f, 30.0f}, 24.0f,
                          material3::SliderVariant::kStandard, style);
```

### Step 11: Add Optional Appearance Overrides

Appearance overrides should be last, after the semantic API and tokenized style
model are proven stable.

Scope:

1. add a grouped override struct rather than a large Android-style setter
  matrix,
2. expose only the override points that are justified by real product needs,
3. keep token defaults as the primary path,
4. avoid expanding override support into separate per-state object graphs.

Deliverable:

- advanced callers can customize the widget beyond Material defaults,
- the common API remains small and embedded-friendly.

Example usage after this step:

```cpp
material3::SliderAppearanceOverrides overrides;
overrides.active_track = roo_display::color::Blue;
overrides.thumb = roo_display::color::Orange;
material3::Slider slider(env, material3::SliderRange{0.0f, 100.0f}, 50.0f);
slider.setAppearanceOverrides(overrides);
```

```cpp
material3::SliderAppearanceOverrides overrides;
overrides.track_height = 8;
overrides.stop_indicator_size = 6;
material3::RangeSlider range_slider(
  env, material3::SliderRange{0.0f, 10.0f, 1.0f}, 2.0f, 7.0f);
range_slider.setAppearanceOverrides(overrides);
```

### Step 12: Finish Documentation and Example Coverage

Every public step above should land with tests, but the final step should make
the new feature family discoverable and easy to adopt.

Scope:

1. expand the Material 3 slider example to show each variant,
2. document migration from normalized `pos_` usage to semantic values,
3. document which features are single-value only, range-only, or size-limited,
4. add short examples for label formatting and range minimum separation.

Deliverable:

- the feature rollout is complete from both API and adoption perspectives,
- callers can migrate incrementally instead of rewriting existing slider code.

Example usage after this step:

```cpp
material3::Slider legacy(env, 32768);
material3::Slider migrated(env, material3::SliderRange{0.0f, 1.0f}, 0.5f);
```

```cpp
material3::RangeSlider schedule(
  env, material3::SliderRange{0.0f, 24.0f, 1.0f}, 8.0f, 18.0f);
schedule.setMinSeparation(2.0f);
```

### Suggested Review Boundaries

For code review and release management, the steps above should be grouped into
small PR-sized increments:

1. baseline tests plus internal refactor,
2. semantic single-value API plus discrete mode,
3. centered mode plus slider lifecycle callbacks,
4. range slider plus active-thumb and separation logic,
5. value indicators plus orientation,
6. size presets, icons, ticks, and stop indicators,
7. optional appearance overrides, examples, and final documentation.

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
9. compatibility behavior of `getPos()` / `setPos()`,
10. size-token measurement and preferred-size output.

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
4. preserve the current normalized API as a compatibility layer,
5. use compact configuration structs rather than mirroring Android's entire
   setter surface,
6. cover Material 3's missing variants and Android's interaction model first,
7. leave room for richer appearance overrides without making the first API too
   large.

That gives `roo_windows` a slider family that is materially closer to Material 3
and Android, while still fitting the library's low-allocation, embedded-first
design constraints.