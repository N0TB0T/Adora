#include "SoundManager.h"

#ifdef ADORA_HAS_SOUND

#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2SNoDAC.h"

namespace adora {

SoundManager::SoundManager()
    : wav_(nullptr),
      file_(nullptr),
      out_(nullptr),
      currentSound_(SoundEvent::None),
      initialized_(false),
      gain_(1.0f),
      muteMode_(MuteMode::Off) {
  for (auto& s : sounds_) {
    s.valid = false;
    s.loopEnabled = false;
  }
}

SoundManager::~SoundManager() {
  stop();
  delete static_cast<AudioGeneratorWAV*>(wav_);
  delete static_cast<AudioFileSourcePROGMEM*>(file_);
  delete static_cast<AudioOutputI2SNoDAC*>(out_);
}

void SoundManager::begin() {
  if (initialized_) return;
  out_ = new AudioOutputI2SNoDAC();
  static_cast<AudioOutputI2SNoDAC*>(out_)->SetGain(gain_);
  wav_ = new AudioGeneratorWAV();
  initialized_ = true;
}

bool SoundManager::addSound(SoundEvent event, const unsigned char* data,
                             size_t length, bool isAuto) {
  if (!data || length == 0) return false;
  for (auto& s : sounds_) {
    if (!s.valid) {
      s.event = event;
      s.data = data;
      s.length = length;
      s.isAuto = isAuto;
      s.loopEnabled = false;
      s.valid = true;
      return true;
    }
  }
  return false;
}

bool SoundManager::removeSound(SoundEvent event) {
  for (auto& s : sounds_) {
    if (s.valid && s.event == event) {
      if (currentSound_ == event) stop();
      s.valid = false;
      return true;
    }
  }
  return false;
}

bool SoundManager::play(SoundEvent event) {
  if (!initialized_) return false;
  if (isMuted(event)) return false;

  SoundEntry* entry = findSound(event);
  if (!entry) return false;

  // Laufende Wiedergabe stoppen
  auto* wav = static_cast<AudioGeneratorWAV*>(wav_);
  if (wav->isRunning()) {
    wav->stop();
  }

  // Alte Source aufräumen
  delete static_cast<AudioFileSourcePROGMEM*>(file_);
  file_ = new AudioFileSourcePROGMEM(entry->data, entry->length);

  auto* out = static_cast<AudioOutputI2SNoDAC*>(out_);
  out->SetGain(gain_);

  if (wav->begin(static_cast<AudioFileSourcePROGMEM*>(file_), out)) {
    currentSound_ = event;
    return true;
  }
  return false;
}

void SoundManager::stop() {
  if (!initialized_) return;
  auto* wav = static_cast<AudioGeneratorWAV*>(wav_);
  if (wav->isRunning()) {
    wav->stop();
  }
  currentSound_ = SoundEvent::None;
}

bool SoundManager::isPlaying() const {
  if (!initialized_) return false;
  return static_cast<AudioGeneratorWAV*>(wav_)->isRunning();
}

SoundEvent SoundManager::currentSound() const {
  return currentSound_;
}

void SoundManager::setLoop(SoundEvent event, bool enabled) {
  SoundEntry* entry = findSound(event);
  if (entry) entry->loopEnabled = enabled;
}

bool SoundManager::getLoop(SoundEvent event) const {
  const SoundEntry* entry = findSound(event);
  return entry ? entry->loopEnabled : false;
}

void SoundManager::setGain(float gain) {
  gain_ = gain;
  if (out_) {
    static_cast<AudioOutputI2SNoDAC*>(out_)->SetGain(gain);
  }
}

float SoundManager::getGain() const {
  return gain_;
}

void SoundManager::setMuteMode(MuteMode mode) {
  muteMode_ = mode;
  // Laufenden Sound stoppen wenn er durch neuen Mute-Modus gemutet wird
  if (isPlaying() && isMuted(currentSound_)) {
    stop();
  }
}

MuteMode SoundManager::getMuteMode() const {
  return muteMode_;
}

bool SoundManager::isMuted(SoundEvent event) const {
  if (muteMode_ == MuteMode::All) return true;
  if (muteMode_ == MuteMode::Off) return false;
  // AutoOnly: nur automatische Sounds gemutet
  const SoundEntry* entry = findSound(event);
  return entry && entry->isAuto;
}

void SoundManager::update() {
  if (!initialized_) return;
  auto* wav = static_cast<AudioGeneratorWAV*>(wav_);
  if (wav->isRunning()) {
    if (!wav->loop()) {
      wav->stop();
    }
  } else if (currentSound_ != SoundEvent::None) {
    // Wiedergabe beendet — Loop-Restart oder aufräumen
    SoundEntry* entry = findSound(currentSound_);
    if (entry && entry->loopEnabled && !isMuted(currentSound_)) {
      SoundEvent ev = currentSound_;
      cleanupPlayback();
      play(ev);
    } else {
      cleanupPlayback();
      currentSound_ = SoundEvent::None;
    }
  }
}

SoundManager::SoundEntry* SoundManager::findSound(SoundEvent event) {
  for (auto& s : sounds_) {
    if (s.valid && s.event == event) return &s;
  }
  return nullptr;
}

const SoundManager::SoundEntry* SoundManager::findSound(
    SoundEvent event) const {
  for (const auto& s : sounds_) {
    if (s.valid && s.event == event) return &s;
  }
  return nullptr;
}

void SoundManager::cleanupPlayback() {
  delete static_cast<AudioFileSourcePROGMEM*>(file_);
  file_ = nullptr;
}

}  // namespace adora

#endif  // ADORA_HAS_SOUND
