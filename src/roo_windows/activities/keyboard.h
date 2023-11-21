#pragma once

#include <inttypes.h>
#include <pgmspace.h>
#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

#include "roo_windows/core/activity.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/keyboard_layout/keyboard_layout.h"
#include "roo_windows/widgets/button.h"

namespace roo_windows {

class KeyboardListener {
 public:
  virtual ~KeyboardListener() {}

  virtual void rune(uint32_t rune) = 0;
  virtual void enter() = 0;
  virtual void del() = 0;
};

class KeyboardWidget;

class Keyboard : public Activity {
 public:
  enum CapsState {
    CAPS_STATE_LOW = 0,
    CAPS_STATE_HIGH = 1,
    CAPS_STATE_HIGH_LOCKED = 2,
  };

  Keyboard(const Environment& env, const KeyboardSpec* spec);

  Widget& getContents() override;

  void setListener(KeyboardListener* listener);

  roo_display::Box getPreferredPlacement(const Task& task) override;

  void show();
  void hide();

  void setPage(int idx);
  CapsState caps_state() const;
  void setCapsState(CapsState caps_state);

 private:
  KeyboardWidget* contents();
  const KeyboardWidget* contents() const;

  std::unique_ptr<Widget> contents_;
};

}  // namespace roo_windows