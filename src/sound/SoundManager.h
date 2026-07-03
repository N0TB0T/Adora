#pragma once
#include "../AdoraDeviceTypes.h"
#include <stddef.h>
#include <stdint.h>

#ifdef ADORA_HAS_SOUND

namespace adora {

#ifndef ADORA_MAX_SOUNDS
#define ADORA_MAX_SOUNDS 10
#endif

/// Non-blocking ESP8266Audio Wrapper.
/// Registriert PROGMEM WAV-Sounds, spielt sie per ID ab.
/// Unterstützt Loop-per-Sound und Mute-Modi.
class SoundManager {
 public:
  SoundManager();
  ~SoundManager();

  void begin();

  /// Sound registrieren. isAuto=true für automatische Sounds (Reverse-Beep etc.).
  bool addSound(SoundEvent event, const unsigned char* data, size_t length,
                bool isAuto = false);
  bool removeSound(SoundEvent event);

  /// Sound abspielen (non-blocking).
  bool play(SoundEvent event);
  void stop();
  bool isPlaying() const;
  SoundEvent currentSound() const;

  /// Loop pro Sound aktivieren/deaktivieren.
  void setLoop(SoundEvent event, bool enabled);
  bool getLoop(SoundEvent event) const;

  /// Lautstärke (0.0 - 4.0, Default 1.0).
  void setGain(float gain);
  float getGain() const;

  /// Mute-Modus: All (kein Sound), AutoOnly (nur manuell), Off (alles an).
  void setMuteMode(MuteMode mode);
  MuteMode getMuteMode() const;

  /// Prüft ob ein bestimmter Sound aktuell gemutet wäre.
  bool isMuted(SoundEvent event) const;

  /// Muss in loop() aufgerufen werden.
  void update();

 private:
  struct SoundEntry {
    SoundEvent event;
    const unsigned char* data;
    size_t length;
    bool isAuto;
    bool loopEnabled;
    bool valid;
  };

  SoundEntry sounds_[ADORA_MAX_SOUNDS];
  // Forward-declared ESP8266Audio Typen — Implementation in .cpp
  void* wav_;
  void* file_;
  void* out_;

  SoundEvent currentSound_;
  bool initialized_;
  float gain_;
  MuteMode muteMode_;

  SoundEntry* findSound(SoundEvent event);
  const SoundEntry* findSound(SoundEvent event) const;
  void cleanupPlayback();
};

}  // namespace adora

#endif  // ADORA_HAS_SOUND
