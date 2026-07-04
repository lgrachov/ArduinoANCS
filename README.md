# ArduinoANCS

Receive **iOS notifications** on your Arduino over Bluetooth Low Energy, with a
simple callback API.

ANCS (Apple Notification Center Service) is the BLE service iPhones expose to
accessories so they can display incoming notifications (app, title, message,
etc.). This library implements the accessory side and hands each notification
to your sketch as a parsed struct.

Built on [ArduinoBLE](https://github.com/arduino-libraries/ArduinoBLE) and
tested on the **Arduino Uno R4 WiFi**.

## Install

1. Copy this folder into your Arduino `libraries/` directory
2. Install the **ArduinoBLE** library (Library Manager → search "ArduinoBLE").
3. Select **Arduino Uno R4 WiFi** as your board.

## Quick start

```cpp
#include <ArduinoANCS.h>

void onNotification(const ANCSNotification& n) {
  Serial.print(n.appIdentifier); Serial.print(": ");
  Serial.print(n.title);         Serial.print(" - ");
  Serial.println(n.message);
}

void setup() {
  Serial.begin(9600);
  ANCS.onNotification(onNotification);
  ANCS.begin();          // starts BLE, advertising, pairing & ANCS discovery
}

void loop() {
  ANCS.poll();           // must be called continuously
}
```

Then on your iPhone: **Settings → Bluetooth → connect to "Arduino-ANCS"**,
accept the pairing prompt and the "allow notifications" prompt, and send
yourself a notification.

## API

### `ANCS.begin(deviceName = "Arduino-ANCS", manageBLE = true)`
Starts the library. By default it owns the BLE stack (calls `BLE.begin()`,
advertises, enables pairing). Pass `manageBLE = false` if your sketch already
called `BLE.begin()`/`BLE.advertise()` and manages its own services — then this
library only handles ANCS discovery on the connected iPhone.

### `ANCS.poll()`
Call every `loop()`. Non-blocking; drives the connect → pair → discover →
subscribe → receive state machine.

### Callbacks
| Method | Fires when |
| --- | --- |
| `onNotification(cb)` | a new/modified notification's metadata has arrived |
| `onRemoved(cb)`      | a notification was dismissed (header only, no metadata) |
| `onConnect(cb)`      | an iPhone connects at the link layer |
| `onDisconnect(cb)`   | the iPhone disconnects |

Callback signature: `void cb(const ANCSNotification& n)` (or `void cb()` for
connect/disconnect).

### `ANCSNotification`
```cpp
uint32_t uid;              // unique id (pass to performAction)
uint8_t  event;            // ANCS_EVENT_ADDED / MODIFIED / REMOVED
uint8_t  flags;            // ANCS_FLAG_* bitmask
uint8_t  category;         // ANCS_CATEGORY_*
uint8_t  categoryCount;
char     appIdentifier[];  // e.g. "com.apple.MobileSMS"
char     title[];
char     subtitle[];
char     message[];
char     date[];           // "YYYYMMDDTHHMMSS"
const char* categoryName() const;
bool hasPositiveAction() const;
bool hasNegativeAction() const;
```

### `ANCS.performAction(uid, actionId)`
Trigger `ANCS_ACTION_POSITIVE` or `ANCS_ACTION_NEGATIVE` on a notification
(e.g. accept/decline a call). Only valid if the notification advertised the
matching action flag.

### `ANCS.ready()` / `ANCS.connected()`
`connected()` is true when an iPhone is linked; `ready()` is true once ANCS is
subscribed and notifications will flow.

## Tuning memory

Buffer sizes default to sensible values but can be overridden by `#define`-ing
them **before** including the header (or via build flags):

```cpp
#define ANCS_MESSAGE_LEN 128   // shorter messages, less RAM
#define ANCS_MAX_PENDING 8     // more in-flight notifications
#include <ArduinoANCS.h>
```

Available: `ANCS_APP_ID_LEN`, `ANCS_TITLE_LEN`, `ANCS_SUBTITLE_LEN`,
`ANCS_MESSAGE_LEN`, `ANCS_DATE_LEN`, `ANCS_MAX_PENDING`.

## Troubleshooting

- **No pairing prompt / never becomes `ready()`** — iOS only exposes ANCS over a
  bonded link. If a previous attempt left a stale bond, go to iOS Settings →
  Bluetooth → ⓘ → **Forget This Device**, then reconnect.
- **Bond lost after reset** — ArduinoBLE does not persist bonds across reboots,
  so you may need to forget + re-pair after reflashing.
- **Long messages truncated** — increase `ANCS_MESSAGE_LEN`. Responses larger
  than the reassembly buffer are dropped to stay memory-safe.

## Examples

- **NotificationPrinter** — prints every notification's metadata to Serial.
- **CallResponder** — auto-declines incoming calls via `performAction()`.

## License

MIT
