#pragma once

#include "roo_windows/core/container.h"

namespace roo_windows::material3 {

/// Container whose background follows its Material 3 container role.
/// Adds no per-instance state; subclasses provide children and layout as usual.
class Material3Container : public Container {
 public:
  /// Creates a Material 3 container using @p context for theme and services.
  explicit Material3Container(ApplicationContext& context)
      : Container(context) {}

  /// Resolves the owned role against the active Material 3 color scheme.
  /// With kNone, inherits the parent's actual background, or the framework
  /// surface color when detached. Transparent fills require an override.
  Color background() const override {
    const ColorToken role = containerRole();
    return role == ColorToken::kNone
               ? Container::background()
               : theme().material3Theme().color.resolve(role);
  }
};

}  // namespace roo_windows::material3
