#pragma once

#if defined(__GXX_RTTI)
#include <typeinfo>
#include "demangle.h"
#endif

namespace roo_windows {

template <typename T>
const char* GetTypeName(const T& t) {
#if defined(__GXX_RTTI)
  char out[256];  
  if (roo_glog::Demangle(typeid(t).name(), out, 256)) {
    return out;
  }
  return typeid(t).name();
#else
  return "";
#endif
}

}  // namespace roo_windows
