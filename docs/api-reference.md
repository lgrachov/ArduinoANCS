# API Reference

Everything you can call, with descriptions. Include the header with:

```cpp
#include <ArduinoANCS.h>
```

This declares a single global instance named `ANCS`.

← Back to [documentation index](index.md)

## `ANCSClass` (the `ANCS` object)

### `bool begin(const char* deviceName = "Arduino-ANCS", bool manageBLE = true)`

Starts the library and begins advertising.

- `deviceName` — the name iOS shows in the Bluetooth list.
- `manageBLE` — when `true` (default), the library calls `BLE.begin()`, adds an
  encrypted helper characteristic, enables pairing, and starts advertising for
  you. Pass `false` if your sketch already called `BLE.begin()` /
  `BLE.advertise()` and manages its own services — the library then only handles
  ANCS discovery on the connected iPhone.

Returns `false` only if it tried and failed to start the BLE stack.

```cpp
if (!ANCS.begin()) {
  Serial.println("BLE failed to start");
  while (1);
}
```

### `void poll()`

Pumps the internal state machine (connect → discover → subscribe → receive).
**Call this on every `loop()` iteration.** It is non-blocking.

### `void end()`

Stops advertising and releases resources. If the library started BLE, it also
calls `BLE.end()`.

### `bool connected() const`

`true` while an iPhone is connected at the link layer.

### `bool ready() const`

`true` once an iPhone is connected **and** ANCS is subscribed — i.e.
notifications will now flow.

### Callback registration

| Method | Callback type | Fires when |
| --- | --- | --- |
| `onNotification(cb)` | `void cb(const ANCSNotification&)` | a new/modified notification's full metadata has arrived |
| `onRemoved(cb)` | `void cb(const ANCSNotification&)` | a notification was dismissed (header only; text fields are empty) |
| `onConnect(cb)` | `void cb()` | an iPhone connects at the link layer |
| `onDisconnect(cb)` | `void cb()` | the iPhone disconnects |

Callbacks run inside `poll()`. Keep them short and non-blocking; don't call
`delay()` for long periods inside them.

### `bool performAction(uint32_t uid, uint8_t actionId)`

Triggers a notification action on the iPhone. `actionId` is
`ANCS_ACTION_POSITIVE` or `ANCS_ACTION_NEGATIVE`. Only valid when the
notification advertised the matching action flag. Returns `false` if not ready
or the write fails. See [Notification Actions](actions.md).

## `struct ANCSNotification`

Passed to your notification callbacks.

| Field | Type | Description |
| --- | --- | --- |
| `uid` | `uint32_t` | Unique id for this notification; pass to `performAction()`. |
| `event` | `uint8_t` | One of `ANCS_EVENT_ADDED`, `ANCS_EVENT_MODIFIED`, `ANCS_EVENT_REMOVED`. |
| `flags` | `uint8_t` | Bitmask of `ANCS_FLAG_*`. |
| `category` | `uint8_t` | One of `ANCS_CATEGORY_*`. |
| `categoryCount` | `uint8_t` | Number of notifications in this category. |
| `appIdentifier` | `char[]` | Bundle id, e.g. `com.apple.MobileSMS`. |
| `title` | `char[]` | Notification title. |
| `subtitle` | `char[]` | Notification subtitle (may be empty). |
| `message` | `char[]` | Notification body text. |
| `date` | `char[]` | Timestamp, format `YYYYMMDDTHHMMSS`. |

Buffer lengths are configurable — see [Configuration & Memory](configuration.md).

### Methods

| Method | Returns | Description |
| --- | --- | --- |
| `categoryName()` | `const char*` | Human-readable category name (never null). |
| `hasPositiveAction()` | `bool` | Whether a positive action is available. |
| `hasNegativeAction()` | `bool` | Whether a negative action is available. |

## Enumerations

### `ANCSEvent`
| Constant | Value |
| --- | --- |
| `ANCS_EVENT_ADDED` | 0 |
| `ANCS_EVENT_MODIFIED` | 1 |
| `ANCS_EVENT_REMOVED` | 2 |

### `ANCSFlags` (bitmask)
| Constant | Bit |
| --- | --- |
| `ANCS_FLAG_SILENT` | 0 |
| `ANCS_FLAG_IMPORTANT` | 1 |
| `ANCS_FLAG_PRE_EXISTING` | 2 |
| `ANCS_FLAG_POSITIVE_ACTION` | 3 |
| `ANCS_FLAG_NEGATIVE_ACTION` | 4 |

### `ANCSCategory`
| Constant | Value |
| --- | --- |
| `ANCS_CATEGORY_OTHER` | 0 |
| `ANCS_CATEGORY_INCOMING_CALL` | 1 |
| `ANCS_CATEGORY_MISSED_CALL` | 2 |
| `ANCS_CATEGORY_VOICEMAIL` | 3 |
| `ANCS_CATEGORY_SOCIAL` | 4 |
| `ANCS_CATEGORY_SCHEDULE` | 5 |
| `ANCS_CATEGORY_EMAIL` | 6 |
| `ANCS_CATEGORY_NEWS` | 7 |
| `ANCS_CATEGORY_HEALTH_AND_FITNESS` | 8 |
| `ANCS_CATEGORY_BUSINESS_FINANCE` | 9 |
| `ANCS_CATEGORY_LOCATION` | 10 |
| `ANCS_CATEGORY_ENTERTAINMENT` | 11 |

### `ANCSActionID`
| Constant | Value |
| --- | --- |
| `ANCS_ACTION_POSITIVE` | 0 |
| `ANCS_ACTION_NEGATIVE` | 1 |

## See also

- [Handling Notifications](handling-notifications.md) — practical use of the struct.
- [Configuration & Memory](configuration.md) — buffer size macros.
