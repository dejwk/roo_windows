#pragma once

#if defined(__GXX_RTTI)
#include <typeinfo>
#include "roo_logging/demangle.h"
#endif

namespace roo_windows {

/// Returns a demangled, human-readable type name for the dynamic type of `t`.
///
/// Falls back to the raw mangled `typeid().name()` if demangling fails, and
/// returns an empty string when the toolchain was built without RTTI.
template <typename T>
std::string GetTypeName(const T& t) {
#if defined(__GXX_RTTI)
  char out[256];  
  if (roo_logging::Demangle(typeid(t).name(), out, 256)) {
    return out;
  }
  return typeid(t).name();
#else
  return "";
#endif
}

}  // namespace roo_windows
