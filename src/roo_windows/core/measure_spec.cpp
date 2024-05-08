#include "roo_windows/core/measure_spec.h"

namespace roo_windows {

roo_logging::Stream& operator<<(roo_logging::Stream& os, WidthSpec spec) {
  switch (spec.kind()) {
    case EXACTLY: {
      os << "width:EXACTLY";
      break;
    }
    case AT_MOST: {
      os << "width:AT_MOST";
      break;
    }
    case UNSPECIFIED: {
      os << "width:UNSPECIFIED";
      break;
    }
    default: {
      os << "width:UNKNOWN";
      break;
    }
  }
  os << "(" << spec.value() << ")";
  return os;
}

roo_logging::Stream& operator<<(roo_logging::Stream& os, HeightSpec spec) {
  switch (spec.kind()) {
    case EXACTLY: {
      os << "height:EXACTLY";
      break;
    }
    case AT_MOST: {
      os << "height:AT_MOST";
      break;
    }
    case UNSPECIFIED: {
      os << "height:UNSPECIFIED";
      break;
    }
    default: {
      os << "height:UNKNOWN";
      break;
    }
  }
  os << "(" << spec.value() << ")";
  return os;
}

}  // namespace roo_windows