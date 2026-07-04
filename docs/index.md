# ArduinoANCS Documentation

Receive **iOS notifications** on your Arduino over Bluetooth Low Energy, with a
simple callback API.

ANCS (Apple Notification Center Service) is the BLE service every iPhone exposes
to accessories so they can display incoming notifications — the app, title,
subtitle, message, and date. This library implements the accessory side and
hands each notification to your sketch as a parsed `ANCSNotification` struct.

Built on [ArduinoBLE](https://github.com/arduino-libraries/ArduinoBLE) and
tested on the **Arduino Uno R4 WiFi**.

```cpp
#include <ArduinoANCS.h>

void onNotification(const ANCSNotification& n) {
  Serial.print(n.appIdentifier); Serial.print(": ");
  Serial.println(n.message);
}

void setup() {
  Serial.begin(9600);
  ANCS.onNotification(onNotification);
  ANCS.begin();
}

void loop() { ANCS.poll(); }
```

## Table of contents

| Page | What's inside |
| --- | --- |
| [Getting Started](getting-started.md) | Install the library, upload your first sketch, and pair your iPhone step by step. |
| [How ANCS Works](how-ancs-works.md) | The concepts behind the library: BLE roles, bonding, and the notification handshake. |
| [API Reference](api-reference.md) | Every class, method, struct field, and enum, with descriptions. |
| [Handling Notifications](handling-notifications.md) | Working with the `ANCSNotification` struct, categories, flags, and events. |
| [Notification Actions](actions.md) | Accept/decline calls and other actions with `performAction()`. |
| [Configuration & Memory](configuration.md) | Tune buffer sizes and RAM usage with compile-time `#define`s. |
| [Examples](examples.md) | Walkthroughs of the bundled example sketches. |
| [Troubleshooting](troubleshooting.md) | Fixes for pairing, bonding, and common runtime issues. |

## Quick links

- **New here?** Start with [Getting Started](getting-started.md).
- **Curious how it works?** Read [How ANCS Works](how-ancs-works.md).
- **Looking up a method?** Jump to the [API Reference](api-reference.md).
- **Something not working?** See [Troubleshooting](troubleshooting.md).

## Requirements

- A BLE-capable Arduino board with an ArduinoBLE core that supports pairing/
  bonding. Developed and tested on the **Arduino Uno R4 WiFi**.
- The **ArduinoBLE** library.
- An iPhone (ANCS is an Apple service; Android does not expose it).

## License

MIT
