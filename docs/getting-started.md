# Getting Started

This guide takes you from zero to seeing your first iPhone notification print
over Serial.

← Back to [documentation index](index.md)

## 1. Install the library

**Option A — manual copy**

1. Download or clone this repository.
2. Copy the whole folder into your Arduino `libraries/` directory:
   - macOS: `~/Documents/Arduino/libraries/`
   - Windows: `Documents\Arduino\libraries\`
   - Linux: `~/Arduino/libraries/`
3. Restart the Arduino IDE. The examples now appear under
   **File → Examples → ArduinoANCS**.

**Option B — arduino-cli**

```bash
arduino-cli lib install ArduinoBLE
# then place this library folder in your sketchbook libraries directory
```

## 2. Install dependencies

ArduinoANCS depends on **ArduinoBLE**. In the IDE:

**Tools → Manage Libraries…** → search `ArduinoBLE` → **Install**.

## 3. Select your board

This library needs a board whose BLE stack supports pairing/bonding. It is
developed and tested on the **Arduino Uno R4 WiFi**.

**Tools → Board → Arduino UNO R4 Boards → Arduino UNO R4 WiFi**

If you don't have the core installed, add it via
**Tools → Board → Boards Manager…** → search `UNO R4` → install.

## 4. Upload the example

Open **File → Examples → ArduinoANCS → NotificationPrinter**, then click
**Upload**. When it finishes, open the **Serial Monitor** (magnifying-glass
icon) and set the baud rate to **9600**.

You should see:

```
ANCS ready. Waiting for iPhone...
```

## 5. Pair your iPhone

1. On the iPhone open **Settings → Bluetooth**.
2. Under *Other Devices* tap **Arduino-ANCS**.
3. Accept the **pairing** prompt.
4. Accept the **"Arduino-ANCS would like to access your notifications"** prompt.

The Serial Monitor should print `iPhone connected - pairing...` and then, once
bonding completes, notifications will start flowing.

> **Important:** iOS only reveals ANCS over a *bonded* link. The pairing prompt
> is triggered automatically the first time the library subscribes. If you don't
> see it, see [Troubleshooting](troubleshooting.md).

## 6. Trigger a notification

Send yourself a text message, set a timer, or wait for any app notification.
You'll see something like:

```
--- Notification ---
  App:      com.apple.MobileSMS
  Category: Social
  Title:    Jane Doe
  Message:  Are we still on for lunch?
  Date:     20260704T120500
  UID:      42
--------------------
```

## Next steps

- Learn what each field means in [Handling Notifications](handling-notifications.md).
- Respond to calls in [Notification Actions](actions.md).
- Understand the machinery in [How ANCS Works](how-ancs-works.md).
