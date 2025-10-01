#ifndef _MODES_DEFAULTKEYBOARDMODE_HPP
#define _MODES_DEFAULTKEYBOARDMODE_HPP

#include "core/KeyboardMode.hpp"
#include "core/state.hpp"

class DefaultKeyboardMode : public KeyboardMode {
  public:
    DefaultKeyboardMode();
    bool isMelee() {return false;};

  private:
    void UpdateKeys(const InputState &inputs);
};

#endif
