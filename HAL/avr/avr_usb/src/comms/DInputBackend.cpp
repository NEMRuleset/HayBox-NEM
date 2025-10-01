#include "comms/DInputBackend.hpp"

#include "core/CommunicationBackend.hpp"
#include "core/state.hpp"

#include <Joystick.h>

DInputBackend::DInputBackend(
    InputState &inputs,
    InputSource **input_sources,
    size_t input_source_count
)
    : CommunicationBackend(inputs, input_sources, input_source_count) {
    _joystick.begin();

    //Set up hardware timer
    TCCR1A = 0;
    TCCR1B = (1 << CS11) | (1 << CS10);//64 clock divider -> 4 microsecond resolution
    zeroTcnt1();
}

DInputBackend::~DInputBackend() {
    _joystick.end();
}

CommunicationBackendId DInputBackend::BackendId() {
    return COMMS_BACKEND_DINPUT;
}

void DInputBackend::SendReport() {
    static uint sampleSpacing = 350;
    //static uint16_t loopTime = 16383;

    while(readTcnt1() < sampleSpacing) {
        //spinlock to make things about 720 hz, about the fastest it'll go while running the b0xx input viewer
    }

    //read how long this loop took
    //loopTime = readTcnt1();//each unit is 4 microseconds
    //zero the counter so that it doesn't overflow
    zeroTcnt1();
    //Serial.print("Loop time: ");
    //Serial.println(loopTime);

    ScanInputs();

    // Run gamemode logic.
    UpdateOutputs();

    //if(_nerfOn) {
    if(_gamemode->isMelee()) {
        //APPLY NERFS HERE
        OutputState nerfedOutputs;
        limitOutputs(sampleSpacing, _nerfOn ? AB_A : AB_B, _inputs, _outputs, nerfedOutputs);

        // Digital outputs
        _joystick->setButton(0, nerfedOutputs.b);
        _joystick->setButton(1, nerfedOutputs.a);
        _joystick->setButton(2, nerfedOutputs.y);
        _joystick->setButton(3, nerfedOutputs.x);
        _joystick->setButton(4, nerfedOutputs.buttonR);
        _joystick->setButton(5, nerfedOutputs.triggerRDigital);
        _joystick->setButton(6, nerfedOutputs.buttonL);
        _joystick->setButton(7, nerfedOutputs.triggerLDigital);
        _joystick->setButton(8, nerfedOutputs.select);
        _joystick->setButton(9, nerfedOutputs.start);
        _joystick->setButton(10, nerfedOutputs.rightStickClick);
        _joystick->setButton(11, nerfedOutputs.leftStickClick);
        _joystick->setButton(12, nerfedOutputs.home);

        // Analog outputs
        _joystick->setXAxis(nerfedOutputs.leftStickX);
        _joystick->setYAxis(255 - nerfedOutputs.leftStickY);
        _joystick->setRxAxis(nerfedOutputs.rightStickX);
        _joystick->setRyAxis(255 - nerfedOutputs.rightStickY);
        _joystick->setZAxis(nerfedOutputs.triggerLAnalog + 1);
        _joystick->setRzAxis(nerfedOutputs.triggerRAnalog + 1);

        // D-pad Hat Switch
        _joystick->setHatSwitch(_outputs.dpadLeft, _outputs.dpadRight, _outputs.dpadDown, _outputs.dpadUp);
    } else {
        // Digital outputs
        _joystick->setButton(0, _outputs.b);
        _joystick->setButton(1, _outputs.a);
        _joystick->setButton(2, _outputs.y);
        _joystick->setButton(3, _outputs.x);
        _joystick->setButton(4, _outputs.buttonR);
        _joystick->setButton(5, _outputs.triggerRDigital);
        _joystick->setButton(6, _outputs.buttonL);
        _joystick->setButton(7, _outputs.triggerLDigital);
        _joystick->setButton(8, _outputs.select);
        _joystick->setButton(9, _outputs.start);
        _joystick->setButton(10, _outputs.rightStickClick);
        _joystick->setButton(11, _outputs.leftStickClick);
        _joystick->setButton(12, _outputs.home);

        // Analog outputs
        _joystick->setXAxis(_outputs.leftStickX);
        _joystick->setYAxis(255 - _outputs.leftStickY);
        _joystick->setRxAxis(_outputs.rightStickX);
        _joystick->setRyAxis(255 - _outputs.rightStickY);
        _joystick->setZAxis(_outputs.triggerLAnalog + 1);
        _joystick->setRzAxis(_outputs.triggerRAnalog + 1);

        // D-pad Hat Switch
        _joystick->setHatSwitch(_outputs.dpadLeft, _outputs.dpadRight, _outputs.dpadDown, _outputs.dpadUp);
    }
}
