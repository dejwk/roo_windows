#pragma once

#if defined(__GXX_RTTI)
#include <typeinfo>
#endif

namespace roo_windows {

template <typename T>
const char* GetTypeName(const T& t) {
#if defined(__GXX_RTTI)
  return typeid(t).name();
#else
  return "";
#endif
}

}  // namespace roo_windows
