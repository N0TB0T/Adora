// 05_Sound — PROGMEM WAV playback via I2S NoDAC
//
// Demonstrates:
//   - SoundManager initialization
//   - Registering PROGMEM WAV samples to SoundEvent IDs
//   - play(), stop(), isPlaying()
//   - Gain (volume) control
//   - Mute modes (All, AutoOnly, Off)
//   - Loop mode per sound
//
// Hardware: Adora board with speaker on GPIO3/RX (I2S NoDAC)
// Upload:   pio run -e adoramini -t upload
// Note:     Serial output goes to GPIO1/TX only (GPIO3 is used by I2S)
//
// IMPORTANT: You must provide your own WAV data as PROGMEM arrays.
//   Convert with: xxd -i horn.wav > horn_wav.h
//   WAV format: 8-bit unsigned mono, 16000 Hz recommended.

#define ADORA_HAS_SOUND

#include <Arduino.h>
#include <sound/SoundManager.h>
#include <profile/BoardProfiles.h>

// ─── Example: inline tiny WAV for testing ───
// Replace these with your actual PROGMEM sound data.
// A real project would #include generated headers:
//   #include "horn_wav.h"    // provides horn_wav[] and horn_wav_len
//   #include "beep_wav.h"    // provides beep_wav[] and beep_wav_len

// Placeholder: 100 bytes of silence (replace with real data!)
static const unsigned char PROGMEM placeholder_wav[] = {
    // RIFF header (44 bytes) + 56 bytes silence
    0x52,0x49,0x46,0x46, 0x5C,0x00,0x00,0x00, 0x57,0x41,0x56,0x45,
    0x66,0x6D,0x74,0x20, 0x10,0x00,0x00,0x00, 0x01,0x00,0x01,0x00,
    0x80,0x3E,0x00,0x00, 0x80,0x3E,0x00,0x00, 0x01,0x00,0x08,0x00,
    0x64,0x61,0x74,0x61, 0x38,0x00,0x00,0x00,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, 0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, 0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, 0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, 0x80,0x80,0x80,0x80,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
};

static adora::SoundManager sound;

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n--- 05_Sound ---"));

    sound.begin();

    // Register sounds — map SoundEvent IDs to PROGMEM data
    sound.addSound(adora::SoundEvent::Horn,       placeholder_wav, sizeof(placeholder_wav));
    sound.addSound(adora::SoundEvent::Confirm,    placeholder_wav, sizeof(placeholder_wav));
    sound.addSound(adora::SoundEvent::ReverseBeep, placeholder_wav, sizeof(placeholder_wav), true);
    //                                                              isAuto=true ─────────────────┘
    // Auto sounds are muted in MuteMode::AutoOnly

    // Volume: 0.0 (silent) to 4.0 (max). Default 1.0.
    sound.setGain(1.5f);

    // Loop mode: ReverseBeep loops until stopped
    sound.setLoop(adora::SoundEvent::ReverseBeep, true);

    Serial.println(F("Commands via serial:"));
    Serial.println(F("  h = play Horn"));
    Serial.println(F("  c = play Confirm"));
    Serial.println(F("  r = play ReverseBeep (loops)"));
    Serial.println(F("  s = stop"));
    Serial.println(F("  + = volume up"));
    Serial.println(F("  - = volume down"));
    Serial.println(F("  m = cycle mute mode"));
}

void loop() {
    sound.update();  // Must be called every loop iteration

    if (Serial.available()) {
        char ch = Serial.read();
        switch (ch) {
            case 'h':
                sound.play(adora::SoundEvent::Horn);
                Serial.println(F("Playing: Horn"));
                break;
            case 'c':
                sound.play(adora::SoundEvent::Confirm);
                Serial.println(F("Playing: Confirm"));
                break;
            case 'r':
                sound.play(adora::SoundEvent::ReverseBeep);
                Serial.println(F("Playing: ReverseBeep (loop)"));
                break;
            case 's':
                sound.stop();
                Serial.println(F("Stopped"));
                break;
            case '+': {
                float g = sound.getGain() + 0.5f;
                if (g > 4.0f) g = 4.0f;
                sound.setGain(g);
                Serial.printf("Gain: %.1f\n", g);
                break;
            }
            case '-': {
                float g = sound.getGain() - 0.5f;
                if (g < 0.0f) g = 0.0f;
                sound.setGain(g);
                Serial.printf("Gain: %.1f\n", g);
                break;
            }
            case 'm': {
                // Cycle: Off → AutoOnly → All → Off
                auto mode = sound.getMuteMode();
                if (mode == adora::MuteMode::Off)
                    sound.setMuteMode(adora::MuteMode::AutoOnly);
                else if (mode == adora::MuteMode::AutoOnly)
                    sound.setMuteMode(adora::MuteMode::All);
                else
                    sound.setMuteMode(adora::MuteMode::Off);
                Serial.printf("Mute: %s\n",
                    sound.getMuteMode() == adora::MuteMode::Off ? "Off" :
                    sound.getMuteMode() == adora::MuteMode::AutoOnly ? "AutoOnly" : "All");
                break;
            }
        }
    }
}
