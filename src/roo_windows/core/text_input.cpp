#include "roo_windows/core/text_input.h"

#include "roo_windows/core/application.h"

namespace roo_windows {

TextInputEmitter::~TextInputEmitter() { disconnect(); }

void TextInputEmitter::connect(Application& destination) {
  destination.connectTextInputEmitter(*this);
}

void TextInputEmitter::disconnect() {
  if (destination_application_ != nullptr) {
    destination_application_->disconnectTextInputEmitter(*this);
  }
}

bool TextInputEmitter::commitRune(uint32_t rune) {
  return destination_application_ != nullptr &&
         destination_application_->commitTextInputRune(rune);
}

bool TextInputEmitter::deleteBackward() {
  return destination_application_ != nullptr &&
         destination_application_->deleteTextInputBackward();
}

bool TextInputEmitter::performAction(TextInputAction action) {
  return destination_application_ != nullptr &&
         destination_application_->performTextInputAction(action);
}

}  // namespace roo_windows
