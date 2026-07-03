# adora-device

Shared Arduino library for [Adora](https://github.com/N0TB0T/Adora26) robotic vehicles — ESP8266 and ESP32 based.

Provides a hardware-abstraction layer with board profiles, motor drivers, LED control, battery monitoring, button handling, and sound playback. One library, multiple board variants, zero `#ifdef` spaghetti in your firmware.

## Features

| Module | Description |
|--------|-------------|
| **BoardProfile** | Declarative hardware description — pins, peripherals, MCU capabilities |
| **Motor** | `DRV8833Driver` (GPIO PWM), `I2CMotorDriver` (DRV8830), abstract `MotorDriver` interface |
| **TankSteering** | Joystick-to-tank math with deadzone, response curves (linear/squared/cubic), turn compensation |
| **LedController** | NeoPixel wrapper with automatic R/G swap for RGB vs GRB strips |
| **Battery** | ADC monitoring with 5 battery types (NiMH, Li-Ion, LiFePO4, Alkaline, USB), auto-detection |
| **Button** | Hardware-debounced input with multi-level press actions and preview callbacks |
| **SoundManager** | ESP8266Audio wrapper for PROGMEM WAV playback, loop mode, mute control |
| **CommManager** | ESP-NOW + WiFi connection state machine with pairing protocol |
| **AdoraDevice** | Facade — wires everything together from a single `BoardProfile` |

## Supported Boards

| Board | MCU | Motors | LEDs | Sound | Defined in |
|-------|-----|--------|------|-------|------------|
| AdoraOrig | ESP8266 | DRV8833 (GPIO) | 1× RGB | — | `BoardProfiles.h` |
| Adora26 | ESP8266 | DRV8833 (GPIO) | 3× GRB | I2S NoDAC | `BoardProfiles.h` |
| Adora26Pro | ESP8266 | DRV8830 (I2C) | 3× GRB | I2S NoDAC | `BoardProfiles.h` |

Custom boards: define your own `BoardProfile` struct — no subclassing needed.

## Quick Start

```cpp
#include <AdoraDevice.h>

// Use a predefined board profile
adora::AdoraDevice device(adora::boardAdora26());

void setup() {
  device.begin();
}

void loop() {
  device.update();

  // Drive with joystick input
  device.handleJoystick(joystickX, joystickY);

  // Direct LED access
  device.led().setColor(0, 255, 0);  // Green
  device.led().setBrightness(128);

  // Battery voltage
  float volts = device.battery().voltage();
}
```

## Using Individual Modules

You don't have to use the `AdoraDevice` facade. Each module works standalone:

```cpp
#include <motor/TankSteering.h>
#include <led/LedController.h>
#include <sensor/Button.h>

// Motor math only — no hardware init
adora::SteeringConfig cfg;
cfg.maxSpeed = 200;
cfg.responseCurve = adora::ResponseCurve::Squared;
auto [left, right] = adora::TankSteering::joystickToMotors(x, y, cfg);

// LED on custom pin
adora::BoardProfile board = adora::boardAdora26();
adora::LedController led(board);
led.begin();
led.setColor(255, 0, 0);

// Button with actions
adora::Button btn(board);
adora::ButtonAction actions[] = {
  {2000, 1, adora::ButtonTrigger::OnRelease},   // 2s hold → action 1
};
btn.begin();
btn.setActions(actions, 1);
btn.onAction([](uint8_t type, uint8_t cycle) { /* handle */ });
btn.update();
```

## Installation (PlatformIO)

Add to `platformio.ini`:

```ini
lib_deps =
    symlink://../shared/library/adora-device
```

For sound support, add the build flag:

```ini
build_flags =
    -DADORA_HAS_SOUND
```

## Architecture

```
src/
├── AdoraDevice.h/.cpp          Facade
├── AdoraDeviceTypes.h          Shared enums & structs
├── profile/
│   ├── BoardProfile.h          Hardware description struct
│   ├── BoardProfiles.h         Predefined board profiles
│   ├── DeviceConfig.h          Runtime feature config
│   └── McuCapabilities.h       MCU enum + capability queries
├── motor/
│   ├── MotorDriver.h           Abstract interface
│   ├── DRV8833Driver.h/.cpp    GPIO PWM driver
│   ├── I2CMotorDriver.h/.cpp   I2C DRV8830 driver
│   └── TankSteering.h/.cpp     Joystick math
├── led/
│   └── LedController.h/.cpp    NeoPixel wrapper
├── sensor/
│   ├── Battery.h/.cpp          ADC + battery types
│   └── Button.h/.cpp           Debounce + actions
├── sound/
│   └── SoundManager.h/.cpp     ESP8266Audio wrapper
└── comm/
    ├── CommManager.h/.cpp       Connection state machine
    ├── EspNowComm.h/.cpp        ESP-NOW abstraction
    ├── WifiComm.h/.cpp          WiFi management
    ├── PairingManager.h/.cpp    Pairing protocol
    └── PairingProtocol.h        Shared protocol structs
```

## Custom Board Profile

```cpp
#include <profile/BoardProfile.h>

inline constexpr adora::BoardProfile myBoard() {
  return {
    .name = "MyRobot",
    .mcu = adora::Mcu::Esp8266,
    .gpioMotor = {D1, D2, D6, D5, D0},  // A1, A2, B1, B2, Sleep
    .i2cMotor  = {false, 0, 0},
    .led       = {D7, 3, adora::LedColorOrder::GRB},
    .sound     = {false},
    .servo     = {0, 0},
    .button    = {D8, false},
    .battery   = {A0, 0.010377f, adora::BatteryType::Aaa4xNiMH},
    .i2c       = {D4, D3},
    .distance  = {true, 0},
  };
}
```

## Examples

| # | Name | What it shows |
|---|------|---------------|
| 01 | **Drive** | DRV8833Driver + TankSteering with serial joystick |
| 02 | **LEDs** | Named colors, RGB gradient, brightness ramp, blink mode, per-pixel |
| 03 | **Button** | Multi-level press actions, preview callbacks, short press |
| 04 | **Battery** | ADC voltage readout, percent, low-battery LED warning |
| 05 | **Sound** | PROGMEM WAV playback, gain, mute modes, loop |
| 06 | **DistanceLed** | VL53L0X + LedController distance-to-color mapping |
| 07 | **EspNow** | ESP-NOW pairing, joystick callbacks, connection state |
| 08 | **FullDevice** | AdoraDevice facade — all features combined |

Each example is self-contained. Run with: `pio run -e adoramini -t upload`

## Testing

Native tests (no hardware required):

```bash
cd shared/library/adora-device
pio test -e native
```

## Requirements

- PlatformIO Core 6+
- Arduino framework
- ESP8266 or ESP32 platform
- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) (for LED module)
- [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio) (optional, for sound module)

## License

MIT — see [LICENSE](LICENSE).

---
*Published from the [Adora26 monorepo](https://github.com/N0TB0T/Adora26)
(`shared/library/adora-device`, commit a525e87) via `tools/publish_adora_library.sh`.
The monorepo is the source of truth — do not develop here. Protocol contract
headers are vendored at publish time.*
