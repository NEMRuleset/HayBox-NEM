#include "comms/GamecubeBackend.hpp"

#include "core/InputSource.hpp"

#include <GamecubeConsole.hpp>
#include <hardware/pio.h>
#include <hardware/timer.h>

GamecubeBackend::GamecubeBackend(
    InputState &inputs,
    InputSource **input_sources,
    size_t input_source_count,
    uint data_pin,
    PIO pio,
    int sm,
    int offset
)
    : CommunicationBackend(inputs, input_sources, input_source_count),
      _gamecube(data_pin, pio, sm, offset) {
    _report = default_gc_report;
}

CommunicationBackendId GamecubeBackend::BackendId() {
    return COMMS_BACKEND_GAMECUBE;
}

void GamecubeBackend::SendReport() {
    // Update slower inputs before we start waiting for poll.
    if (!_gamemode->isMelee()) {
        ScanInputs(InputScanSpeed::SLOW);
        ScanInputs(InputScanSpeed::MEDIUM);
    }

    // Read inputs
    _gamecube.WaitForPollStart();

    // Update fast inputs in response to poll.
    // But wait 40us first so that we read inputs at the start of the 3rd byte of the poll command
    // not the second, so inputs are maximum 40us old by the time we start sending the report.
    busy_wait_us(40);
    if (!_gamemode->isMelee()) {
        ScanInputs(InputScanSpeed::FAST);

        // Run gamemode logic.
        UpdateOutputs();
    } else {
        //wait a bit extra to ensure comms are reliable
        busy_wait_us(3);
    }

    if (!_gamemode->isMelee()) {
        // Digital outputs
        _report.a = _outputs.a;
        _report.b = _outputs.b;
        _report.x = _outputs.x;
        _report.y = _outputs.y;
        _report.z = _outputs.buttonR;
        _report.l = _outputs.triggerLDigital;
        _report.r = _outputs.triggerRDigital;
        _report.start = _outputs.start;
        _report.dpad_left = _outputs.dpadLeft | _outputs.select;
        _report.dpad_right = _outputs.dpadRight | _outputs.home;
        _report.dpad_down = _outputs.dpadDown;
        _report.dpad_up = _outputs.dpadUp;

        // Analog outputs
        _report.stick_x = _outputs.leftStickX;
        _report.stick_y = _outputs.leftStickY;
        _report.cstick_x = _outputs.rightStickX;
        _report.cstick_y = _outputs.rightStickY;
        _report.l_analog = _outputs.triggerLAnalog;
        _report.r_analog = _outputs.triggerRAnalog;
    } else {
        // Digital outputs
        _report.a = _finalOutputs.a;
        _report.b = _finalOutputs.b;
        _report.x = _finalOutputs.x;
        _report.y = _finalOutputs.y;
        _report.z = _finalOutputs.buttonR;
        _report.l = _finalOutputs.triggerLDigital;
        _report.r = _finalOutputs.triggerRDigital;
        _report.start = _finalOutputs.start;
        _report.dpad_left = _finalOutputs.dpadLeft | _finalOutputs.select;
        _report.dpad_right = _finalOutputs.dpadRight | _finalOutputs.home;
        _report.dpad_down = _finalOutputs.dpadDown;
        _report.dpad_up = _finalOutputs.dpadUp;

        // Analog outputs
        _report.stick_x = _finalOutputs.leftStickX;
        _report.stick_y = _finalOutputs.leftStickY;
        _report.cstick_x = _finalOutputs.rightStickX;
        _report.cstick_y = _finalOutputs.rightStickY;
        _report.l_analog = _finalOutputs.triggerLAnalog;
        _report.r_analog = _finalOutputs.triggerRAnalog;
    }

    // Send outputs to console unless poll command is invalid.
    if (_gamecube.WaitForPollEnd() != PollStatus::ERROR) {
        _gamecube.SendReport(&_report);
    }
}

int GamecubeBackend::GetOffset() {
    return _gamecube.GetOffset();
}
