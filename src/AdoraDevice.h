#pragma once

// ===========================
// ADORA DEVICE — Facade
// ===========================
// Central entry point — creates the correct motor driver based on board profile,
// provides simple API for driving, LEDs, sensors.
// Communication (ESP-NOW, WiFi) is available via separate modules (not orchestrated here).

#include "AdoraDeviceTypes.h"
#include "profile/BoardProfiles.h"
#include "profile/DeviceConfig.h"
#include "motor/MotorDriver.h"
#include "motor/DRV8833Driver.h"
#include "motor/I2CMotorDriver.h"
#include "motor/TankSteering.h"
#include "led/LedController.h"
#include "sensor/Battery.h"
#include "sensor/Button.h"

#ifdef ADORA_HAS_SOUND
#include "sound/SoundManager.h"
#endif

namespace adora {

class AdoraDevice {
 public:
    /// Construct from board profile (board capabilities = device features).
    explicit AdoraDevice(const BoardProfile& board = activeBoard());

    /// Construct from device config (board can do more than device uses).
    explicit AdoraDevice(const DeviceConfig& config);

    ~AdoraDevice();

    /// Initialize all sub-modules. Call in setup().
    bool begin();

    /// Update sensors + timeouts. Call in loop().
    void update();

    // ─── Motor ───
    void handleJoystick(int16_t x, int16_t y);
    void drive(int16_t leftPower, int16_t rightPower);
    void stop();
    void brake();
    void setMotorsEnabled(bool enabled);
    void setSteeringConfig(const SteeringConfig& cfg);

    // ─── Peripherie ───
    LedController& led();
    Battery& battery();
    Button& button();

#ifdef ADORA_HAS_SOUND
    SoundManager& sound();
#endif

    // ─── Status ───
    MotorState motorState() const;
    const BoardProfile& board() const;
    McuCapabilities mcuCaps() const;

 private:
    BoardProfile board_;
    MotorDriver* motor_ = nullptr;
    LedController led_;
    Battery battery_;
    Button button_;
    SteeringConfig steering_;
#ifdef ADORA_HAS_SOUND
    SoundManager sound_;
#endif
};

}  // namespace adora
