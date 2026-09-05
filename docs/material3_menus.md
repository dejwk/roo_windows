# Material 3 menus

`roo_windows::material3::Menu` presents a transient, anchored set of actions in
the shared window host. It borrows an existing `Task` as the interaction owner,
captures placement geometry synchronously, moves focus into the menu, and
restores valid prior focus when the chain closes.

This is intentionally different from the legacy `roo_windows::menu::Menu`.
The legacy type is a full-screen `Destination` with a title and arbitrary
scrollable children; it remains available for route-like screens. The Material
3 type is not a destination and never changes navigation history.

## Choosing and migrating the type

| Legacy usage | Material 3 replacement |
| --- | --- |
| Full-screen route of settings or controls | Keep `roo_windows::menu::Menu`, or build a dedicated `Destination`. |
| Short action list opened from a button | Build `material3::MenuItem` rows and call `Menu::show(task, button)`. |
| Context actions at known coordinates | Call `Menu::showFromRect(task, bounds)`. |
| Arbitrary child widget added with `add()` | Model an action as `StandardMenuItem`, or derive `MenuItem`/`MenuEntry` for specialized content. |
| `BasicNavigationItem` entering a destination | Keep route navigation; a transient menu action may call the same navigation operation from `onInvoked()`. |
| Nested route screen | Keep navigation when the child is a screen; use `hasSubmenu()` and synchronous `populateSubmenu()` only for another action level. |

A menu's persistent root groups, rows, items, and borrowed slot widgets must
outlive the `Menu`, or be detached with `clearGroups()` while it is idle.
Member declarations should therefore put those objects before the menu. A
derived class whose members are borrowed by its menu must call
`prepareForDerivedDestruction()` at the start of its destructor. Adopted groups
and rows can instead be supplied with `std::unique_ptr`.

```cpp
class SaveItem final : public material3::StandardMenuItem {
 public:
  SaveItem() : StandardMenuItem({"Save", {}}) { setShortcut("Ctrl+S"); }
  void onInvoked() override { SaveConfiguration(); }
};

class Actions {
 public:
  explicit Actions(ApplicationContext& context)
      : save_row_(context), group_(context), menu_(context) {
    save_row_.setMenuItem(save_);
    group_.add(save_row_);
    menu_.addGroup(group_);
  }

  void Show(Task& owner, const Widget& button) {
    MenuShowResult result = menu_.show(owner, button);
    // kShown means focus and transient input ownership changed synchronously.
    // Other results leave the current presentation and focus unchanged.
    (void)result;
  }

 private:
  SaveItem save_;
  material3::MenuEntry save_row_;
  material3::MenuGroup group_;
  material3::Menu menu_;
};
```

Leaf invocation applies selection first, calls `onInvoked()`, then follows the
configured dismissal policy. Multiple-selection leaves remain open by default;
ordinary and single-selection leaves dismiss by default. Back/Escape closes
the deepest submenu first and then the root. Arrow keys, Home/End, and wrapping
Tab traversal operate only within the deepest visible level.

## Memory and allocation audit

The committed `material3_menu_size_probe` records named `sizeof` symbols. An
ESP32-C3 GCC 14.2.0 compile (32-bit pointers, 2026-09-05) measured:

```sh
bash benchmarks/material3_menu_size_probe.sh \
  /path/to/riscv32-esp-elf-g++ /path/to/riscv32-esp-elf-nm
```

| Type | Bytes |
| --- | ---: |
| `Menu` | 12 |
| internal `Menu::Impl` allocation | 456 |
| `MenuEntry` / `ListEntry` | 104 / 88 |
| `StandardMenuItem` | 32 |
| `MenuRow<StandardMenuItem>` | 136 |
| `MenuGroup` / `MenuOverlay` | 56 / 56 |
| `MenuPanel` / `SimpleScrollablePanel` | 280 / 168 |
| optional trailing payload / bound adornment state | 32 / 64 |
| generated single-line text slot | 48 |

For a deliberately busy three-level chain—two six-row root groups, one
four-row group in each of two visible child panels, eight adorned rows, and
four optional item payloads—the modeled live allocation payload is 5,688
bytes. This includes current vector capacities (128 bytes) and all menu-owned
objects and generated text slots; allocator headers and caller-owned strings,
icons, application objects, and the 12-byte stack-resident `Menu` are excluded.
The payload remains below the revised 6 KiB representative ceiling.

Configuration, group adoption, binding, submenu population, and presentation
admission may allocate. Paint, measure, layout, focus traversal, keyboard
dispatch, hover, and scrolling do not. Closing child levels releases their
presentation-scoped panels, groups, rows, text slots, and adornment state.
