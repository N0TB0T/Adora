# Adora Board Specification Sheet

> **Status:** Entwurf — Pinbelegung und Details bitte mit Hardware abgleichen und ergänzen.  
> **Letzte Änderung:** 2026-03-25  
> **Hinweis:** "Board" ≠ "Fahrzeug". Ein Board kann in verschiedenen Fahrzeugtypen verbaut werden.

---

## Naming-Konvention

| Ebene | Beispiel | Beschreibung |
|---|---|---|
| **Board** | `AdoraOrig`, `Adora26`, `Adora26Pro` | PCB-Variante mit fester Pin-/Feature-Ausstattung |
| **Fahrzeug** | `Adora Mini`, `Adora Mini Monsta`, `Adora Kipper`, `Adora Bagger` | Mechanische/kosmetische Variante, verwendet eines der Boards |

Ein Fahrzeugtyp bestimmt, welches Board verbaut ist. Board-Wahl → Pins & Features stehen fest.

---

## Board-Übersicht

| Eigenschaft | AdoraOrig | Adora26 | Adora26Pro |
|---|---|---|---|
| **Status** | ✅ In Produktion | 🔧 In Entwicklung | ✅ Produktionsbereit |
| **MCU** | ESP8266 (D1 Mini) | ESP8266 (D1 Mini) | ESP8266 (D1 Mini) |
| **Motor-Treiber** | DRV8833 | DRV8833 | DRV8830 (Dual I2C) |
| **LEDs onboard** | 1× WS2812 | 3× WS2812 | 3× WS2812 |
| **LED-Layout** | Center | 01 Center, 02 Links, 03 Rechts | 01 Center, 02 Links, 03 Rechts |
| **Ext. LED-Anschluss** | ✅ (nach Onboard, variabel) | ✅ (nach Onboard, variabel) | ✅ (nach Onboard, variabel) |
| **Sound** | ❌ | ❌ | ✅ NoDac (I2S) |
| **Servo-Ausgänge** | ❌ | ❌ | 2× PWM |
| **Button** | ✅ (D8, Pull-up) | ✅ (D8, Pull-up) | ✅ (D3, active LOW) |
| **Batterie-ADC** | ✅ (A0) | ✅ (A0) | ✅ (A0) |
| **I2C** | ✅ (D4/D3) | ✅ (D4/D3) | ✅ (D1/D2) |
| **VL53L0X (ToF)** | Optional (I2C, 0x29) | Optional (I2C, 0x29) | Optional (I2C, 0x29) |
| **Sleep/Enable** | ✅ (D0) | ✅ (D0) | — (D0 frei, empfohlen für XSHUT) |

> **VL53L0X:** Optionaler Distanzsensor, wird auf den 4-poligen I2C-Stecker der 26er Boards aufgesteckt. Adresse 0x29.

> **NotBotBot-Hardware:** Dieses Spec-Sheet deckt nur die Adora-Boards (ESP8266) ab. Die
> NotBotBot-Profile — inkl. D1-R32-Pinout (Legacy), Servo-Kalibrierung und
> DY-SV17F-Audio-Spezifikation — leben in
> [`firmware/notbotbot/docs/HARDWARE_PROFILES_V1.md`](../../../firmware/notbotbot/docs/HARDWARE_PROFILES_V1.md).

---

## Pinbelegung

> **⚠️ Bitte mit Hardware-Schaltplan abgleichen und fehlende Werte eintragen.**  
> Spalten mit `?` sind noch offen. Bekannte Werte aus Firmware-Code als Ausgangspunkt.

### AdoraOrig

> Identische Pinbelegung wie Adora26. Unterschied: nur 1× LED onboard.

| Funktion | ESP8266 Pin | GPIO | Anmerkung |
|---|---|---|---|
| Motor A1 (links) | D1 | GPIO5 | DRV8833 AIN1 |
| Motor A2 (links) | D2 | GPIO4 | DRV8833 AIN2 |
| Motor B1 (rechts) | D6 | GPIO12 | DRV8833 BIN1 |
| Motor B2 (rechts) | D5 | GPIO14 | DRV8833 BIN2 |
| DRV8833 Sleep | D0 | GPIO16 | Active-low enable |
| WS2812 Data | D7 | GPIO13 | 1× LED (Center) |
| Button | D8 | GPIO15 | Braucht Pull-up |
| Battery ADC | A0 | ADC0 | Spannungsteiler, Ratio: ? |
| I2C SCL | D4 | GPIO2 | 4-pol. I2C-Stecker — optional VL53L0X etc. |
| I2C SDA | D3 | GPIO0 | 4-pol. I2C-Stecker |

### Adora26

> Wie AdoraOrig, aber 3× LED. Kein Sound. Hardwareänderungen noch nicht durchgeführt.

| Funktion | ESP8266 Pin | GPIO | Anmerkung |
|---|---|---|---|
| Motor A1 (links) | D1 | GPIO5 | DRV8833 AIN1 |
| Motor A2 (links) | D2 | GPIO4 | DRV8833 AIN2 |
| Motor B1 (rechts) | D6 | GPIO12 | DRV8833 BIN1 |
| Motor B2 (rechts) | D5 | GPIO14 | DRV8833 BIN2 |
| DRV8833 Sleep | D0 | GPIO16 | Active-low enable |
| WS2812 Data | D7 | GPIO13 | 3× LED (Center, Links, Rechts) |
| Button | D8 | GPIO15 | Braucht Pull-up |
| Battery ADC | A0 | ADC0 | Spannungsteiler, Ratio: ? |
| I2C SCL | D4 | GPIO2 | 4-pol. I2C-Stecker — optional VL53L0X etc. |
| I2C SDA | D3 | GPIO0 | 4-pol. I2C-Stecker |

### Adora26Pro

> Verwendet DRV8830 (I2C) statt DRV8833 — Motoren werden über I2C-Bus gesteuert, keine separaten Motor-PWM-Pins.
> Dadurch werden GPIO-Pins für Servo-Ausgänge frei.
>
> **I2S-Pins (hardware-fixed):** GPIO3=Data, GPIO15(D8)=BCK, GPIO2(D4)=WS.
> D4 und D8 sind während Audio-Wiedergabe von I2S belegt → nicht anderweitig nutzbar.
> Siehe [I2S_PIN_CONFLICTS.md](I2S_PIN_CONFLICTS.md) für Details.

| Funktion | ESP8266 Pin | GPIO | Anmerkung |
|---|---|---|---|
| I2C SCL | D1 | GPIO5 | Shared: DRV8830 ×2 + 4-pol. I2C-Stecker (VL53L0X etc.) |
| I2C SDA | D2 | GPIO4 | Shared: DRV8830 ×2 + 4-pol. I2C-Stecker |
| Button | D3 | GPIO0 | Active LOW, INPUT_PULLUP. ⚠️ Muss HIGH beim Boot |
| ⚠️ I2S WS | D4 | GPIO2 | Reserviert — I2S hardware-fixed. Nicht verwenden |
| Servo 1 | D5 | GPIO14 | PWM-Ausgang |
| Servo 2 | D6 | GPIO12 | PWM-Ausgang |
| WS2812 Data | D7 | GPIO13 | 3× LED (Center, Links, Rechts) |
| Sound (I2S NoDac) | RX | GPIO3 | ESP8266Audio NoDac → NPN+1K → 2W 8Ω Speaker |
| ⚠️ I2S BCK | D8 | GPIO15 | Reserviert — I2S hardware-fixed. Nicht verwenden |
| Motor A (links) | — | — | DRV8830 via I2C (Addr per I2C-Scan) |
| Motor B (rechts) | — | — | DRV8830 via I2C (Addr per I2C-Scan) |
| VL53L0X XSHUT | D0 | GPIO16 | Optional — ermöglicht Sensor-Reset ohne Power-Cycle |
| Battery ADC | A0 | ADC0 | Spannungsteiler, Ratio: ? |

---

## Fahrzeugtyp → Board-Zuordnung

| Fahrzeugtyp | Board | Protocol Device-ID | Anmerkung |
|---|---|---|---|
| Adora Mini | AdoraOrig | `0x02` | Aktuell in Produktion |
| Adora Mini Monsta | AdoraOrig | `0x04` | Andere Reifen/Chassis, selbes Board |
| Adora Mini (2026) | Adora26 | ? | Nachfolger |
| Adora Kipper | ? | ? | Platzhalter |
| Adora Bagger | ? | ? | Platzhalter |
| Adora Mini Pro | Adora26Pro | ? | Mit Servo-Ausgängen |

---

## Feature-Matrix (Capability Flags)

| Capability | Flag | AdoraOrig | Adora26 | Adora26Pro |
|---|---|---|---|---|
| LED | `0x0002` | ✅ (1×) | ✅ (3×) | ✅ (3×) |
| Headlight | `0x0004` | ✅ | ✅ | ✅ |
| Sound | `0x0001` | ❌ | ❌ | ✅ |
| Settings | `0x0008` | ✅ | ✅ | ✅ |
| I2C Motor Driver | `0x0010` | ❌ | ❌ | ✅ (DRV8830 Dual) |
| Servo | — | ❌ | ❌ | ✅ (2×) |
| VL53L0X (ToF) | — | Optional | Optional | Optional |

---

## LED-Modell

Alle Boards verwenden eine einzige serielle WS2812-Kette. LED-Index = Position in der Kette.

### Architektur

```
[Onboard LEDs] → [Externe LEDs]
     |                 |
     |                 └── variabel, Anzahl nicht begrenzt
     |
     ├── LED 0: Center (mandatory, immer bestückt) — SK6812 (GRB)
     ├── LED 1: Links  (optional, Jumper-überbrückbar) — WS2812B-2020 (R/G invertiert!)
     └── LED 2: Rechts (optional, Jumper-überbrückbar) — WS2812B-2020 (R/G invertiert!)
```

### LED-Typen

| Position | Typ | Farb-Reihenfolge | Anmerkung |
|---|---|---|---|
| LED 0 (Center) | SK6812 | GRB (Standard) | Baugleich mit WS2812B |
| LED 1 (Links) | WS2812B-2020 | **GRB invertiert** → effektiv RBG | Noname-Variante, R und G vertauscht |
| LED 2 (Rechts) | WS2812B-2020 | **GRB invertiert** → effektiv RBG | Noname-Variante, R und G vertauscht |
| Externe LEDs | variabel | abhängig vom angeschlossenen Strip | Muss konfigurierbar sein |

> **Achtung:** Die Library (`LedController`) muss R und G für LED 1+2 softwareseitig tauschen,
> damit `setColor(255, 0, 0)` auf allen LEDs tatsächlich Rot ergibt.
> Getestet und bestätigt am 2026-03-03.

### Regeln

1. **LED 0 (Center) ist immer vorhanden** — auf allen Boards bestückt und mandatory.
2. **LED 1 + 2 (Links/Rechts) sind optional** — nur auf Adora26/Adora26Pro bestückt.
   Nicht bestückte oder nicht benötigte LEDs werden per Lötjumper überbrückt.
   In diesem Fall rücken alle nachfolgenden Pixel auf.
3. **Externe LEDs** werden in Reihe hinter den Onboard-LEDs angeschlossen.
   Sie starten ab dem nächsten freien Index (typ. ab Index 1 bei AdoraOrig, ab Index 3 bei Adora26).
4. **Anzahl externer LEDs ist nicht begrenzt** — Architektur ist variabel.

### Index-Beispiele

| Konfiguration | Idx 0 | Idx 1 | Idx 2 | Idx 3+ |
|---|---|---|---|---|
| AdoraOrig, keine externen | Center | — | — | — |
| AdoraOrig + 4 externe | Center | Ext 1 | Ext 2 | Ext 3, 4 |
| Adora26, alle bestückt | Center | Links | Rechts | — |
| Adora26, LED 2+3 gejumpert + 3 ext. | Center | Ext 1 | Ext 2 | Ext 3 |
| Adora26 + 2 externe | Center | Links | Rechts | Ext 1, 2 |

### Board-Defaults

| Board | Onboard bestückt | Onboard max | Ext. Anschluss |
|---|---|---|---|
| AdoraOrig | 1 (Center) | 1 | ✅ variabel |
| Adora26 | 3 (C/L/R) | 3 | ✅ variabel |
| Adora26Pro | 3 (C/L/R) | 3 | ✅ variabel |

---

## Offene Fragen

1. ~~**AdoraOrig Pinbelegung**~~ — ✅ Bestätigt: identisch mit Adora26.
2. **Sleep-Pin** — Wird der DRV8833-Sleep-Pin auf AdoraOrig/Adora26 aktiv genutzt? (Adora26Pro: entfällt, I2C-gesteuert)
3. ~~**Sound-Pin**~~ — ✅ Bestätigt: RX/GPIO3, NPN-Transistor + 1K Widerstand auf allen 26er Boards verbaut.
4. **Servo-Pins (Adora26Pro)** — Welche GPIOs?
5. **Battery Voltage Divider** — Ratio pro Board?
6. ~~**Externer LED-Strip**~~ — ✅ Auf allen Boards möglich, variabel, keine Begrenzung.
7. **I2C-Peripherie** — VL53L0X (ToF-Sensor) auf Adora26/Adora26Pro bestätigt. Weitere Sensoren/Displays vorgesehen?
