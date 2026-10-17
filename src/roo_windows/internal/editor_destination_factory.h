#pragma once

#include <memory>

namespace roo_windows {
class ApplicationContext;
class EditorDestination;
namespace internal {
// Concrete construction belongs to the corresponding design-system module.
std::unique_ptr<EditorDestination> CreateMaterial2EditorDestination(
    ApplicationContext& context);
std::unique_ptr<EditorDestination> CreateMaterial3EditorDestination(
    ApplicationContext& context);
}  // namespace internal
}  // namespace roo_windows
