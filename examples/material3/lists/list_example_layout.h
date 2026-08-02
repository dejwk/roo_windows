#pragma once

#include "roo_windows/containers/flex_layout.h"

namespace roo_windows::material3_examples {

// A vertical scroll panel measures wrap-content children at their natural
// width. List example content should instead span the available viewport.
class FullWidthColumn : public FlexLayout {
 public:
  explicit FullWidthColumn(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn) {}

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }
};

}  // namespace roo_windows::material3_examples
