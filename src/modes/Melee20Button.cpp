#include "modes/Melee20Button.hpp"

//legacy wavedash angle

#define ANALOG_STICK_MIN (128-112)
#define ANALOG_STICK_NEUTRAL 128
#define ANALOG_STICK_MAX (128+112)

Melee20Button::Melee20Button() : ControllerMode() {
}

void Melee20Button::SetConfig(GameModeConfig &config, const MeleeOptions options) {
    InputMode::SetConfig(config);
    _options = options;
}

void Melee20Button::HandleSocd(InputState &inputs) {
    //force neutral socd
    if (inputs.lf3 && inputs.lf1) {
        inputs.lf3 = false;
        inputs.lf1 = false;
    }
    if (inputs.lf2 && inputs.rf4) {
        inputs.lf2 = false;
        inputs.rf4 = false;
    }
    if (inputs.rt3 && inputs.rt5) {
        inputs.rt3 = false;
        inputs.rt5 = false;
    }
    if (inputs.rt2 && inputs.rt4) {
        inputs.rt2 = false;
        inputs.rt4 = false;
    }
    InputMode::HandleSocd(inputs);
}

void Melee20Button::UpdateDigitalOutputs(const InputState &inputs, OutputState &outputs) {
    outputs.a = inputs.rt1;
    outputs.b = inputs.rf1;
    outputs.x = inputs.rf2;
    outputs.y = inputs.rf6;
    outputs.buttonR = inputs.rf3;
    if (inputs.nunchuk_connected) {
        outputs.triggerLDigital = inputs.nunchuk_z;
    } else {
        outputs.triggerLDigital = inputs.lf4;
    }
    outputs.triggerRDigital = inputs.rf5;
    outputs.start = inputs.mb1;

    // Activate D-Pad layer by holding Mod X + Mod Y or Nunchuk C button.
    if ((inputs.lt1 && inputs.lt2) || inputs.nunchuk_c) {
        outputs.dpadUp = inputs.rt4;
        outputs.dpadDown = inputs.rt2;
        outputs.dpadLeft = inputs.rt3;
        outputs.dpadRight = inputs.rt5;
    }

    if (inputs.mb3)
        outputs.dpadLeft = true;
    if (inputs.mb2)
        outputs.dpadRight = true;
}

void Melee20Button::UpdateAnalogOutputs(const InputState &inputs, OutputState &outputs) {
    // Coordinate calculations to make modifier handling simpler.
    UpdateDirections(
        inputs.lf3, // Left
        inputs.lf1, // Right
        inputs.lf2, // Down
        inputs.rf4, // Up
        inputs.rt3, // C-Left
        inputs.rt5, // C-Right
        inputs.rt2, // C-Down
        inputs.rt4, // C-Up
        ANALOG_STICK_MIN,
        ANALOG_STICK_NEUTRAL,
        ANALOG_STICK_MAX,
        outputs
    );

    const bool shield_button_pressed = inputs.lf4 || inputs.rf5 || inputs.rf7 || inputs.rf8;
    if (directions.diagonal) {
        // q1/2 = 7000 7000
        outputs.leftStickX = 128 + (directions.x * 56);
        outputs.leftStickY = 128 + (directions.y * 61);
        // L, R, LS, and MS + q3/4 = 7000 6875 (For vanilla shield drop. Gives 44.5
        // degree wavedash). Also used as default q3/4 diagonal if crouch walk option select is
        // enabled.
        if (directions.y == -1 && (shield_button_pressed || _options.crouch_walk_os)) {
            outputs.leftStickX = 128 + (directions.x * 61);
            outputs.leftStickY = 128 + (directions.y * 56);
        }
    }

    /* Mod X */
    if (inputs.lt1) {
        // MX + Horizontal (even if shield is held) = 6625 = 53
        if (directions.horizontal) {
            outputs.leftStickX = 128 + (directions.x * 53);
        }
        // MX + Vertical (even if shield is held) = 5375 = 43
        // y=-0.5500 (44) is solo nana ice block, so we reduce this by one
        // MX + Vertical (even if shield is held) = 5250 = 42
        if (directions.vertical) {
            outputs.leftStickY = 128 + (directions.y * 42);
        }
        if (directions.diagonal && shield_button_pressed) {
            // Use custom airdodge angle if set, otherwise B0XX standard default.
            if (_options.has_custom_airdodge) {
                outputs.leftStickX = 128 + (directions.x * _options.custom_airdodge.x);
                outputs.leftStickY = 128 + (directions.y * _options.custom_airdodge.y);
            } else {
                if (!inputs.rf1) {
                    // MX + L, R, LS, and MS + q1/2/3/4 = 6375 3750 = 51 30
                    outputs.leftStickX = 128 + (directions.x * 51);
                    outputs.leftStickY = 128 + (directions.y * 30);
                } else {
                    // Extended angle to have magnitude similarity for DI
                    // 8500 5125 - 31.09 deg - 68 41
                    outputs.leftStickX = 128 + (directions.x * 68);
                    outputs.leftStickY = 128 + (directions.y * 41);
                }
            }
        }

        /* Up B angles */
        if (directions.diagonal && !shield_button_pressed) {
            if (!inputs.rf1) {
                // 7250 3125 - 23.32deg - 58 25 - modX
                // 7000 3625 - 27.38deg - 56 29 - modX + cDown
                // 6625 4125 - 31.91deg - 53 33 - modX + cLeft
                // 6375 4625 - 35.96deg - 51 37 - modX + cUp
                // 6125 5125 - 39.92deg - 49 41 - modX + cRight
                outputs.leftStickX = 128 + (directions.x * 58);
                outputs.leftStickY = 128 + (directions.y * 25);
                if (inputs.rt2) {
                    outputs.leftStickX = 128 + (directions.x * 56);
                    outputs.leftStickY = 128 + (directions.y * 29);
                }
                if (inputs.rt3) {
                    outputs.leftStickX = 128 + (directions.x * 53);
                    outputs.leftStickY = 128 + (directions.y * 33);
                }
                if (inputs.rt4) {
                    outputs.leftStickX = 128 + (directions.x * 51);
                    outputs.leftStickY = 128 + (directions.y * 37);
                }
                if (inputs.rt5) {
                    outputs.leftStickX = 128 + (directions.x * 49);
                    outputs.leftStickY = 128 + (directions.y * 41);
                }
            } else {
                /* Extended Up B Angles */
                // 9125 3875 - 23.01deg - 73 31 - modX + B
                // 8750 4500 - 27.22deg - 70 36 - modX + B + cDown
                // 8500 5250 - 31.70deg - 68 42 - modX + B + cLeft
                // 7250 5250 - 35.91deg - 58 42 - modX + B + cUp
                // 6375 5250 - 39.47deg - 51 42 - modX + B + cRight
                outputs.leftStickX = 128 + (directions.x * 73);
                outputs.leftStickY = 128 + (directions.y * 31);
                if (inputs.rt2) {
                    outputs.leftStickX = 128 + (directions.x * 70);
                    outputs.leftStickY = 128 + (directions.y * 36);
                }
                if (inputs.rt3) {
                    outputs.leftStickX = 128 + (directions.x * 68);
                    outputs.leftStickY = 128 + (directions.y * 42);
                }
                if (inputs.rt4) {
                    outputs.leftStickX = 128 + (directions.x * 58);
                    outputs.leftStickY = 128 + (directions.y * 42);
                }
                if (inputs.rt5) {
                    outputs.leftStickX = 128 + (directions.x * 51);
                    outputs.leftStickY = 128 + (directions.y * 42);
                }
            }
        }

        // Angled fsmash
        if (directions.cx != 0) {
            // 8500 5250 = 68 42
            outputs.rightStickX = 128 + (directions.cx * 68);
            outputs.rightStickY = 128 + (directions.y * 42);
        }
    }

    /* Mod Y */
    if (inputs.lt2) {
        // MY + Horizontal (even if shield is held) = 3375 = 27
        if (directions.horizontal) {
            outputs.leftStickX = 128 + (directions.x * 27);
        }
        // MY + Vertical (even if shield is held) = 7375 = 59
        if (directions.vertical) {
            outputs.leftStickY = 128 + (directions.y * 59);
        }
        if (directions.diagonal && shield_button_pressed) {
            // MY + L, R, LS, and MS + q1/2 = 4750 8750 = 38 70
            outputs.leftStickX = 128 + (directions.x * 38);
            outputs.leftStickY = 128 + (directions.y * 70);
            // MY + L, R, LS, and MS + q3/4 = 5000 8500 = 40 68
            if (directions.y == -1) {
                outputs.leftStickX = 128 + (directions.x * 40);
                outputs.leftStickY = 128 + (directions.y * 68);
            }
        }

        // Turnaround neutral B nerf
        if (inputs.rf1) {
            outputs.leftStickX = 128 + (directions.x * 80);
        }

        /* Up B angles */
        if (directions.diagonal && !shield_button_pressed) {
            if (!inputs.rf1) {
                // 3250 7625 - 23.09deg - 26 61 - modY
                // 3625 7000 - 27.38deg - 29 56 - modY + cDown
                // 4500 6875 - 33.21deg - 36 55 - modY + cLeft (this is shield drop straight down but the cstick will buffer)
                // 5125 7000 - 36.21deg - 41 56 - modY + cUp
                // 5750 7125 - 38.90deg - 46 57 - modY + cRight
                outputs.leftStickX = 128 + (directions.x * 26);
                outputs.leftStickY = 128 + (directions.y * 61);
                if (inputs.rt2) {
                    outputs.leftStickX = 128 + (directions.x * 29);
                    outputs.leftStickY = 128 + (directions.y * 56);
                }
                if (inputs.rt3) {
                    //outputs.leftStickX = 128 + (directions.x * 35);
                    //outputs.leftStickY = 128 + (directions.y * 56);
                    outputs.leftStickX = 128 + (directions.x * 36);
                    outputs.leftStickY = 128 + (directions.y * 55);
                }
                if (inputs.rt4) {
                    outputs.leftStickX = 128 + (directions.x * 41);
                    outputs.leftStickY = 128 + (directions.y * 56);
                }
                if (inputs.rt5) {
                    outputs.leftStickX = 128 + (directions.x * 46);
                    outputs.leftStickY = 128 + (directions.y * 57);
                }
            } else {
                /* Extended Up B Angles */
                // 3875 9125 - 23.01deg - 31 73 - modY + B
                // 4625 8750 - 27.86deg - 37 70 - modY + B + cDown
                // 5375 8250 - 33.08deg - 42 68 - modY + B + cLeft
                // 5750 7875 - 36.14deg - 46 63 - modY + B + cUp
                // 5750 7125 - 38.90deg - 46 57 - modY + B + cRight
                outputs.leftStickX = 128 + (directions.x * 31);
                outputs.leftStickY = 128 + (directions.y * 73);
                if (inputs.rt2) {
                    outputs.leftStickX = 128 + (directions.x * 37);
                    outputs.leftStickY = 128 + (directions.y * 70);
                }
                if (inputs.rt3) {
                    outputs.leftStickX = 128 + (directions.x * 43);
                    outputs.leftStickY = 128 + (directions.y * 66);
                }
                if (inputs.rt4) {
                    outputs.leftStickX = 128 + (directions.x * 46);
                    outputs.leftStickY = 128 + (directions.y * 63);
                }
                if (inputs.rt5) {
                    outputs.leftStickX = 128 + (directions.x * 46);
                    outputs.leftStickY = 128 + (directions.y * 57);
                }
            }
        }
    }

    // C-stick ASDI Slideoff angle overrides any other C-stick modifiers (such as
    // angled fsmash).
    if (directions.cx != 0 && directions.cy != 0) {
        // 5250 8500 = 42 68
        outputs.rightStickX = 128 + (directions.cx * 42);
        outputs.rightStickY = 128 + (directions.cy * 68);
    }

    /*
    // Horizontal SOCD overrides X-axis modifiers (for ledgedash maximum jump
    // trajectory).
    if (!_options.disable_ledgedash_socd_override && _horizontal_socd && !directions.vertical) {
        outputs.leftStickX = 128 + (directions.x * 80);
    }
    */

    if (inputs.rf7) {
        outputs.triggerRAnalog = 49;
    }
    if (inputs.rf8) {
        outputs.triggerRAnalog = 94;
    }

    if (outputs.triggerLDigital) {
        outputs.triggerLAnalog = 140;
    }
    if (outputs.triggerRDigital) {
        outputs.triggerRAnalog = 140;
    }

    // Shut off C-stick when using D-Pad layer.
    if ((inputs.lt1 && inputs.lt2) || inputs.nunchuk_c || inputs.lt3) {
        outputs.rightStickX = 128;
        outputs.rightStickY = 128;
    }

    /*
    // Nunchuk overrides left stick.
    if (inputs.nunchuk_connected) {
        outputs.leftStickX = inputs.nunchuk_x;
        outputs.leftStickY = inputs.nunchuk_y;
    }
    */
}
