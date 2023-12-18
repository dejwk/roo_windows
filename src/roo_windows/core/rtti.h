#pragma once

#if defined(__GXX_RTTI)
#include <typeinfo>
#include "roo_logging/demangle.h"
#endif

namespace roo_windows {

template <typename T>
const char* GetTypeName(const T& t) {
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
