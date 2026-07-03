#include "AdoraDevice.h"

namespace adora {

AdoraDevice::AdoraDevice(const BoardProfile& board)
    : board_(board), led_(board), battery_(board), button_(board)
#ifdef ADORA_HAS_SOUND
    , sound_()
#endif
{
}

AdoraDevice::AdoraDevice(const DeviceConfig& config)
    : board_(config.board), led_(config.board), battery_(config.board), button_(config.board)
#ifdef ADORA_HAS_SOUND
    , sound_()
#endif
{
}

AdoraDevice::~AdoraDevice() {
    delete motor_;
}

bool AdoraDevice::begin() {
    // Create motor driver based on board capabilities
    if (board_.hasI2CMotor()) {
        motor_ = new I2CMotorDriver(board_);
    } else if (board_.hasGpioMotor()) {
        motor_ = new DRV8833Driver(board_);
    }

    if (motor_) motor_->begin();
    led_.begin();
    battery_.begin();
    button_.begin();

#ifdef ADORA_HAS_SOUND
    if (board_.hasSound()) {
        sound_.begin();
    }
#endif

    return true;
}

void AdoraDevice::update() {
    battery_.update();
    button_.update();

#ifdef ADORA_HAS_SOUND
    if (board_.hasSound()) {
        sound_.update();
    }
#endif
}

// ─── Motor ───

void AdoraDevice::handleJoystick(int16_t x, int16_t y) {
    int16_t left, right;
    TankSteering::joystickToMotors(x, y, steering_, left, right);
    drive(left, right);
}

void AdoraDevice::drive(int16_t leftPower, int16_t rightPower) {
    if (motor_) motor_->drive(leftPower, rightPower);
}

void AdoraDevice::stop() {
    if (motor_) motor_->stop();
}

void AdoraDevice::brake() {
    if (motor_) motor_->brake();
}

void AdoraDevice::setMotorsEnabled(bool enabled) {
    if (motor_) motor_->setEnabled(enabled);
}

void AdoraDevice::setSteeringConfig(const SteeringConfig& cfg) {
    steering_ = cfg;
}

// ─── Peripherie ───

LedController& AdoraDevice::led() { return led_; }
Battery& AdoraDevice::battery() { return battery_; }
Button& AdoraDevice::button() { return button_; }

#ifdef ADORA_HAS_SOUND
SoundManager& AdoraDevice::sound() { return sound_; }
#endif

// ─── Status ───

MotorState AdoraDevice::motorState() const {
    if (motor_) return motor_->state();
    return MotorState{};
}

const BoardProfile& AdoraDevice::board() const { return board_; }
McuCapabilities AdoraDevice::mcuCaps() const { return board_.mcuCaps(); }

}  // namespace adora
