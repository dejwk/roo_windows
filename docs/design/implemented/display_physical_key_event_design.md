# Physical Key Event Design

## Objective

Preserve physical switch identity and overlapping key transitions while
offering each event to the focused widget before task-level semantics.

## Motivation

`KeyCode` describes a framework meaning, not the switch that produced it. It
cannot distinguish left and right modifiers, main and keypad controls, or two
overlapping printable switches that resolve to the same character. The current
fallback activation also matches only semantic codes and can complete on an Up
from the wrong switch.

## Background

[`KeySource`](../../../src/roo_windows/core/key_source.h) supplies ordered
`KeyEvent` batches. A [task](../glossary.md#task) owns focus and handles Back,
Escape, Tab, directional navigation, and Enter-or-Space activation around
focused-widget dispatch.

A physical keyboard reports Down when a switch closes, optional Repeat while it
remains closed, and Up when it opens. A printable Down or Repeat also carries
the character resolved by the adapter's current layout and modifiers.

The task currently remembers one unhandled Enter or Space press so it can show
the focused widget's pressed state and click on release. This document calls
that state the **pending fallback activation**.

## Requirements

1. Every delivered event must retain phase, normalized switch identity,
   semantic code, modifiers, and resolved rune without creating a second
   character event.
2. One switch must use the same nonzero identity for Down, Repeat, and Up.
3. Distinct overlapping switches must remain ordered and independent.
4. Printable Down and Repeat events must use `KeyCode::kCharacter` and contain
   their rune. Their Up event must retain identity and code with a zero rune.
5. Space must remain `KeyCode::kSpace`; main/keypad controls and left/right
   modifiers must remain distinct.
6. The focused widget must receive each event before task-level Back, Escape,
   Tab, navigation, or fallback activation.
7. Standard widgets that decline an event must retain existing semantic
   behavior.
8. A pending fallback activation must complete only for the same focused widget
   and physical switch that began it.
9. Disconnecting the creating source, detaching the focused subtree, changing
   focus, or receiving a replacement activation must cancel the pressed state.
10. `KeyEvent` and task object sizes must not increase on the 32-bit target.

## Design Overview

`PhysicalKey` is a one-byte USB HID Keyboard/Keypad usage identity. It occupies
the existing padding before `KeyEvent::rune`, so the event remains eight bytes.

`Widget::onKeyEvent()` is the single widget entry point for the complete event.
Task dispatch changes order rather than adding a second virtual hook:

```text
KeyEvent
   |
   +--> focused widget and ancestors: onKeyEvent()
   |
   +--> task Back / Escape / Tab / directional semantics
   |
   `--> Enter-or-Space fallback activation
```

The source-detachment path cancels fallback activation before a task loses its
sole physical source. The task therefore needs no source or route identity in
its pending state.

## Design Details

### Physical representation

`PhysicalKey::kNone` is reserved for default initialization and is invalid in an
event delivered by `KeySource`. Other values follow the one-byte USB HID
Keyboard/Keypad usage page.

For a printable switch, Down and Repeat carry `KeyCode::kCharacter` and one
Unicode scalar in `rune`. Up carries the same `PhysicalKey` and `KeyCode` with
`rune == 0`. Non-character keys always carry a zero rune. Space uses
`KeyCode::kSpace`; a text field inserts U+0020 from an unmodified Space Down or
Repeat.

This is a simple hardware layout result, not an IME protocol. Composition,
multi-scalar commits, candidates, and surrounding-text operations use the
[semantic text-input path](../proposed/display_semantic_text_input_design.md).

### Widget-first dispatch

The focused widget and then its ancestors receive `onKeyEvent()` before any
task semantic shortcut. Returning true consumes the transition completely. A
special-purpose widget can maintain its own 256-bit pressed set, while ordinary
widgets continue returning false for events they do not handle.

Using the existing hook prevents one physical transition from visiting a
widget twice. It also makes capture-before-navigation a general key-dispatch
ordering rule rather than a physical-key-only API.

### Fallback activation

The task retains one pending fallback activation containing the focused widget
and `PhysicalKey`. A new unhandled Enter or Space Down cancels the previous
pressed visual before replacing it. An Up completes the click only when both
values match. Disconnecting the task's routed physical source cancels the
pending activation first; the application input router preserves this order.

## Proposed API

```cpp
enum class PhysicalKey : uint8_t {
  kNone = 0x00,
  // Remaining values follow USB HID Keyboard/Keypad usages.
};

struct KeyEvent {
  KeyPhase phase;
  KeyCode code;
  uint8_t modifiers;
  PhysicalKey physical_key;
  uint32_t rune;
};
```

`Widget::onKeyEvent(const KeyEvent&)` remains the only public widget hook. Task
dispatch receives no binding or source object.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Completed: preserve and dispatch physical identity

Added `PhysicalKey`, updated production, fake, and test adapters, preserved
overlapping transition sequences, reordered the existing `onKeyEvent()` path
ahead of task semantics, and qualified fallback activation with switch
identity. Validation covers ABI size, adapter rollover, character payloads,
widget consumption, pressed-state replacement, and source-disconnect, focus,
and subtree cancellation.

Landed validation:

```sh
bazel test //:key_source_test //:task_test //fake:fltk_key_source_test
```

Landed commit: `d23a103` (`Implemented physical key event support in
lib/roo_windows.`)

## Testing Plan

Focused `KeySource`, fake adapter, task, text-field, and non-touch navigation
tests cover representation, ordering, widget consumption, hardware text entry,
and fallback matching. Size probes assert an eight-byte `KeyEvent`; removing the
temporary task source pointer must not be offset by new task state.

## Caveats

Events from one source retain source order. The first version accepts one source
per task. Disconnection does not synthesize Up events, so pressed-state
consumers must clear their own state on focus loss or device reset.

### Rejected Alternatives

#### Add `onPhysicalKeyEvent()`

Rejected because an unconsumed transition would then visit the same focused
widget through two hooks. Extending and reordering `onKeyEvent()` expresses the
same capability with one dispatch contract.

#### Infer identity from `KeyCode` or rune

Rejected because both describe resolved meaning. Different switches can share
either value.

#### Emit a separate character event

Rejected because it duplicates one transition and loses the physical identity
needed to pair Down, Repeat, and Up.

## Future Work

Several physical sources can later target one task by adding an internal route
identity to fallback activation. That extension must define cross-source order
and fairness from concrete device requirements.

Special-purpose virtual D-pad and game-controller adapters can generate
explicit physical transitions. The built-in software text keyboard continues
to use semantic text input.
