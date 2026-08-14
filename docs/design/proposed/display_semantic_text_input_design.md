# Semantic Software Text Input Design

## Objective

Deliver software-keyboard editing operations to one active editor per
application without synthesizing physical key transitions or storing emitter
connections in tasks.

## Motivation

A physical keyboard reports switches changing state. A software keyboard first
resolves a pointer gesture, then commits text or invokes an editor operation.
Expanding that result into Down and Up exposes software input to task navigation
and control activation and makes cancellation and long-press alternatives
artificial.

## Background

The built-in [`Keyboard`](../../../src/roo_windows/activities/keyboard.h)
currently calls a `KeyboardListener` with rune, Enter, and delete callbacks.
Each task owns a `TextFieldEditor`, but the one built-in keyboard listener makes
software editing effectively application-global already.

A touch character cannot commit at pointer down because a long press can replace
the base character with an alternative. Backspace is different: deletion begins
on press and repeats while held. Hardware events handled by a focused text field
already reach the same editor operations through `TextFieldEditor`.

## Requirements

1. Software input must expose semantic rune commit, backward deletion, and Done
   operations rather than `KeyEvent` phases.
2. One emitter must target at most one application, and one application must
   have at most one active software editing session.
3. Character and Space controls must commit exactly once on successful release
   and do nothing at pointer down or cancellation.
4. Backspace must delete once at press start and repeat at the existing delay
   and interval until release or cancellation.
5. Enter must perform Done. Caps and page controls must remain keyboard-local.
6. Operations must bypass widget key dispatch, task navigation, focus movement,
   and fallback control activation.
7. An unconnected emitter or an application with no active editor must return
   false without side effects.
8. Delivery must be one synchronous, constant-stack entry on the destination
   UI thread. The design must add no semantic event queue.
9. Emitter or application destruction must disconnect safely before destination
   storage disappears.
10. Keyboard visibility, layout pages, composition policy, and alternatives
    must remain outside the connection contract.
11. Delivery, repeat, connection, disconnection, and teardown must not allocate
    after initialization.

## Design Overview

The proposal introduces two concepts:

- A **text-input emitter** is the software controller's producer endpoint. It
  owns its single connection to a destination application.
- **Application text input** is a stable, application-owned endpoint that
  records the one currently active `TextFieldEditor` and applies emitter
  operations to it.

```text
software keyboard
      |
      v
TextInputEmitter --> ApplicationTextInput --> active TextFieldEditor
                                                ^
                                                |
                                    focused TextField session
```

The application keeps an intrusive list of connected emitters using a link in
each emitter. This list exists only for application teardown; tasks store no
emitter or connection pointers. The active editor separately registers while
its editing session exists.

Delivery is synchronous because no payload storage exists. Invalidation caused
by editing requests the destination application's ticker through the
[event-driven invalidation path](display_event_driven_input_design.md); delivery
does not recursively tick or paint that application.

## Design Details

### One active application editor

`ApplicationTextInput` stores one nullable `TextFieldEditor*`. Starting a
software-enabled editing session first cancels the previous active session, then
registers the new editor. Ending, cancelling, or destroying that editor clears
the pointer before its storage disappears.

Tasks retain independent focus managers and editors. Only the application-wide
active session receives software operations. Hardware input continues to target
the focused text field within its explicitly routed task and does not depend on
this pointer.

This makes destination selection stable across keyboard touches without
routing to a task by z-order or most-recent pointer activity.

### Emitter connection and lifetime

`TextInputEmitter::connect(Application&)` checks that the emitter is
disconnected, the destination is not stopping, and all post-start use shares
the destination UI thread. It inserts the emitter into the application's
incoming list and stores the application endpoint pointer. `disconnect()` is
idempotent.

Emitter destruction disconnects itself. Application destruction first marks
the endpoint unavailable, clears every incoming emitter's destination and link,
clears the active editor, and then destroys tasks. A surviving emitter therefore
becomes unconnected without dereferencing destroyed storage.

There is no standalone `TextInputBinding`: the emitter already has one route,
and the application registry supplies reverse teardown.

### Semantic operations

`commitRune()` validates one Unicode scalar and delegates to the active editor's
insertion logic. `deleteBackward()` removes the selection or preceding glyph.
`performAction(kDone)` follows the existing accepted-edit path and ends the
session. Each operation returns true only when an active editor applies it.

Hardware `TextField::onKeyEvent()` calls the same editor implementation for
character, Space, Backspace, Delete, movement, and Done behavior. The paths
share final editing results but not intermediate events.

### Software keyboard behavior

The `Keyboard` owns one `TextInputEmitter`:

- Character releases resolve case and call `commitRune()` once.
- Space release commits U+0020.
- Enter release performs `TextInputAction::kDone`.
- Backspace press calls `deleteBackward()` and the existing scheduler repeats
  it at the layout delay and interval. Release and cancellation stop repeat.
- Caps and page controls remain local. One-shot caps resets only after a
  successful character commit.

Character pointer down changes only keyboard-local gesture and visual state.
This preserves cancellation and permits a future long press to suppress the
base rune and commit an alternative.

### Visibility policy

Connection does not show or hide a keyboard. The standard single-display
application uses small private integration glue: software-enabled editor
activation shows the built-in keyboard, session completion hides it, and the
keyboard emitter remains connected to its owning application.

A cross-application keyboard owner chooses its own visibility policy. Controls
are disabled while its emitter is unconnected. A connected emitter can remain
visible when the destination has no active editor; operations then return
false.

### Resource budget

Each emitter stores a destination pointer and one next link, replacing the
keyboard's listener pointer and adding one pointer. `ApplicationTextInput`
stores the incoming-list head and active-editor pointer. Tasks and text fields
gain no connection state. Direct operations create no temporary `KeyEvent` or
payload object.

## Proposed API

```cpp
enum class TextInputAction : uint8_t {
  kDone,
};

class TextInputEmitter {
 public:
  TextInputEmitter() = default;
  ~TextInputEmitter();

  /// Connects this emitter to one application's active editor endpoint.
  void connect(Application& destination);

  /// Removes the destination. Idempotent.
  void disconnect();

  /// Returns whether this emitter has a destination application.
  bool isConnected() const;

  bool commitRune(uint32_t rune);
  bool deleteBackward();
  bool performAction(TextInputAction action);

  TextInputEmitter(const TextInputEmitter&) = delete;
  TextInputEmitter& operator=(const TextInputEmitter&) = delete;
  TextInputEmitter(TextInputEmitter&&) = delete;
  TextInputEmitter& operator=(TextInputEmitter&&) = delete;
};

class Keyboard {
 public:
  Widget& content();
  TextInputEmitter& textInputEmitter();
};
```

`ApplicationTextInput` and editor activation entries remain private framework
APIs. Public declarations receive complete ownership, affinity, return-value,
and checked-precondition documentation.

## Implementation Plan

Implementation follows the
[embedded C++ guidance](../../../.github/instructions/embedded-cpp-code-authoring.instructions.md).

### Step 1: add application text input

Add `ApplicationTextInput`, producer-owned emitter connections, active-editor
registration, and shared editor methods. Remove the editor's `Keyboard*` and
validate activation replacement, inactive return values, both endpoint
destruction orders, thread checks, and zero routing allocation.

Proposed commit: `feat: add application-scoped text input`

### Step 2: convert the built-in keyboard

Replace `KeyboardListener` with `TextInputEmitter`. Convert release-time rune,
Space, and Done operations plus press-time repeated Backspace. Add deterministic
gesture-clock tests for release, cancellation, repeat, one-shot caps, and
long-press-ready suppression.

Proposed commit: `feat: emit semantic software text input`

### Step 3: integrate visibility and cross-application use

Add the standard private show/hide glue and a two-application example. Validate
that keyboard touches preserve editor-task focus, input never reaches ordinary
key dispatch, and editing invalidation wakes a dormant destination.

Proposed commit: `feat: integrate software keyboard editor sessions`

## Testing Plan

Focused endpoint and editor tests cover lifetime, affinity, inactive sessions,
operation semantics, and allocation. Keyboard tests cover gesture timing and
local controls. Integration compares final editor results from equivalent
physical and software operations without requiring equal intermediate events.

## Caveats

`commitRune()` accepts one Unicode scalar because that is the current keyboard
layout vocabulary. Editor-owned strings and glyph data can still grow when
content is accepted; embedded clients must impose their own content bounds.

Cross-application synchronous delivery requires both applications and the
emitter to use the same UI thread. Cross-thread semantic input needs an
explicit bounded operation queue and is outside this design.

### Rejected Alternatives

#### Route software input through `KeyEvent`

Rejected because a resolved editor operation has no physical switch lifecycle.
Synthetic phases expose it to unrelated task navigation and activation.

#### Target a task directly

Rejected because the existing built-in keyboard already represents one active
software editing session. A stable application endpoint eliminates task
connection lists and centralizes session replacement and teardown.

#### Keep `KeyboardListener`

Rejected because it preserves an implicit callback lifetime beside the new
semantic endpoint. Backward compatibility is not required for Phase 6.

#### Queue operations through the scheduler

Rejected because runes and actions would require bounded storage, ordering,
overflow, and cancellation policies. Same-thread synchronous entry is smaller
and preserves immediate operation results.

## Future Work

A richer text-input session can add committed strings, composing ranges,
selection queries, editor capabilities, cursor movement, and a bounded
cross-thread queue. Alternate-character popups remain keyboard-owned gesture UI
and commit their selected result through `commitRune()`.

