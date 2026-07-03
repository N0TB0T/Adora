#include "Button.h"
#include <Arduino.h>

namespace adora {

Button::Button(const BoardProfile& board)
    : pin_(board.button.pin),
      activeLow_(board.button.activeLow),
      lastState_(false),
      currentState_(false),
      lastDebounceMs_(0),
      pressStartMs_(0),
      longPressTriggered_(false),
      onPressCb_(nullptr),
      onLongPressCb_(nullptr),
      longPressDurationMs_(1000),
      actionCount_(0),
      cycleCount_(0),
      shortPressCb_(nullptr),
      actionCb_(nullptr),
      previewCb_(nullptr) {
  for (uint8_t i = 0; i < MAX_ACTIONS; i++) {
    actionTriggered_[i] = false;
  }
}

void Button::begin() {
  if (activeLow_) {
    pinMode(pin_, INPUT_PULLUP);
  } else {
    pinMode(pin_, INPUT);
  }
  lastState_ = readRaw();
  currentState_ = lastState_;
}

void Button::update() {
  bool reading = readRaw();
  unsigned long now = millis();

  if (reading != lastState_) {
    lastDebounceMs_ = now;
  }

  bool justPressed = false;
  bool justReleased = false;

  if ((now - lastDebounceMs_) > DEBOUNCE_MS) {
    if (reading != currentState_) {
      currentState_ = reading;
      if (currentState_) {
        justPressed = true;
        pressStartMs_ = now;
        longPressTriggered_ = false;
        cycleCount_ = 0;
        for (uint8_t i = 0; i < actionCount_; i++) {
          actionTriggered_[i] = false;
        }
      } else {
        justReleased = true;
      }
    }
  }

  lastState_ = reading;

  if (actionCount_ > 0) {
    updateActions(now, justPressed, justReleased);
  } else {
    updateSimple(now);
  }
}

bool Button::isPressed() const { return currentState_; }

// ── Einfaches API ──

void Button::onPress(SimpleCallback cb) { onPressCb_ = cb; }

void Button::onLongPress(SimpleCallback cb, uint16_t durationMs) {
  onLongPressCb_ = cb;
  longPressDurationMs_ = durationMs;
}

// ── Action-System ──

void Button::setActions(const ButtonAction* actions, uint8_t count) {
  actionCount_ = (count > MAX_ACTIONS) ? MAX_ACTIONS : count;
  for (uint8_t i = 0; i < actionCount_; i++) {
    actions_[i] = actions[i];
    actionTriggered_[i] = false;
  }
}

void Button::onShortPress(SimpleCallback cb) { shortPressCb_ = cb; }
void Button::onAction(ActionCallback cb) { actionCb_ = cb; }
void Button::onPreview(PreviewCallback cb) { previewCb_ = cb; }

unsigned long Button::holdDurationMs() const {
  if (!currentState_) return 0;
  return millis() - pressStartMs_;
}

bool Button::readRaw() const {
  bool pinHigh = digitalRead(pin_) == HIGH;
  return activeLow_ ? !pinHigh : pinHigh;
}

// ── Einfache Logik (kein Action-System) ──

void Button::updateSimple(unsigned long now) {
  if (!currentState_ && lastState_) {
    // Gerade losgelassen
    if (!longPressTriggered_ && onPressCb_) {
      onPressCb_();
    }
  }

  if (currentState_ && !longPressTriggered_ &&
      (now - pressStartMs_) >= longPressDurationMs_) {
    longPressTriggered_ = true;
    if (onLongPressCb_) {
      onLongPressCb_();
    }
  }
}

// ── Multi-Level Action-Logik ──

void Button::updateActions(unsigned long now, bool /*justPressed*/,
                           bool justReleased) {
  if (currentState_) {
    unsigned long duration = now - pressStartMs_;

    // Preview-Callback: Cycle-Count basierend auf erster Action-Schwelle
    if (actionCount_ > 0 && duration >= actions_[0].durationMs && previewCb_) {
      uint8_t newCycle =
          static_cast<uint8_t>(duration / actions_[0].durationMs);
      if (newCycle != cycleCount_) {
        cycleCount_ = newCycle;
        uint8_t r = 255, g = 200, b = 0;
        previewCb_(actions_[0].actionType, cycleCount_, &r, &g, &b);
      }
    }

    // OnPress-Actions prüfen
    for (uint8_t i = 0; i < actionCount_; i++) {
      if (duration >= actions_[i].durationMs &&
          actions_[i].trigger == ButtonTrigger::OnPress &&
          !actionTriggered_[i]) {
        actionTriggered_[i] = true;
        if (actionCb_) {
          actionCb_(actions_[i].actionType, cycleCount_);
        }
        return;
      }
    }
  }

  if (justReleased) {
    unsigned long duration = now - pressStartMs_;

    // Höchste erreichte OnRelease-Action finden
    for (int i = actionCount_ - 1; i >= 0; i--) {
      if (duration >= actions_[i].durationMs &&
          actions_[i].trigger == ButtonTrigger::OnRelease &&
          !actionTriggered_[i]) {
        if (actionCb_) {
          actionCb_(actions_[i].actionType, cycleCount_);
        }
        return;
      }
    }

    // Kürzer als erste Schwelle → Short Press
    if (actionCount_ > 0 && duration < actions_[0].durationMs) {
      if (shortPressCb_) {
        shortPressCb_();
      }
    }
  }
}

}  // namespace adora
