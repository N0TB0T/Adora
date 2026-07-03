#pragma once
#include "../AdoraDeviceTypes.h"
#include "../profile/BoardProfile.h"
#include <stdint.h>

namespace adora {

/// Hardware-Button mit Debouncing, Multi-Level Actions und Preview-Callbacks.
///
/// Zwei Modi:
/// - **Einfach:** onPress() / onLongPress() — ein Schwellwert, fertig.
/// - **Action-System:** setActions() + onAction() / onPreview() / onShortPress()
///   für komplexe Multi-Level Logik (Modus-Wechsel, Factory-Reset etc.).
class Button {
 public:
  static constexpr uint8_t MAX_ACTIONS = 4;

  using SimpleCallback = void (*)();
  using ActionCallback = void (*)(uint8_t actionType, uint8_t cycleCount);
  using PreviewCallback =
      void (*)(uint8_t actionType, uint8_t cycleCount,
               uint8_t* r, uint8_t* g, uint8_t* b);

  explicit Button(const BoardProfile& board);

  void begin();
  void update();
  bool isPressed() const;

  // ── Einfaches API ──
  void onPress(SimpleCallback cb);
  void onLongPress(SimpleCallback cb, uint16_t durationMs = 1000);

  // ── Action-System ──
  void setActions(const ButtonAction* actions, uint8_t count);
  void onShortPress(SimpleCallback cb);
  void onAction(ActionCallback cb);
  void onPreview(PreviewCallback cb);

  /// Wie lange der Button aktuell gedrückt ist (0 wenn nicht gedrückt).
  unsigned long holdDurationMs() const;

 private:
  uint8_t pin_;
  bool activeLow_;

  // Debounce
  bool lastState_;
  bool currentState_;
  unsigned long lastDebounceMs_;
  static constexpr unsigned long DEBOUNCE_MS = 30;

  // Press timing
  unsigned long pressStartMs_;
  bool longPressTriggered_;

  // Simple API
  SimpleCallback onPressCb_;
  SimpleCallback onLongPressCb_;
  uint16_t longPressDurationMs_;

  // Action system
  ButtonAction actions_[MAX_ACTIONS];
  uint8_t actionCount_;
  bool actionTriggered_[MAX_ACTIONS];
  uint8_t cycleCount_;
  SimpleCallback shortPressCb_;
  ActionCallback actionCb_;
  PreviewCallback previewCb_;

  bool readRaw() const;
  void updateSimple(unsigned long now);
  void updateActions(unsigned long now, bool justPressed, bool justReleased);
};

}  // namespace adora
