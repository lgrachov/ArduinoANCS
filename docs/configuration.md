# Configuration & Memory

ArduinoANCS keeps notification text in fixed-size buffers so it never allocates
on the heap. You can tune those sizes to trade RAM for capacity.

← Back to [documentation index](index.md)

## Overriding buffer sizes

`#define` any of the macros **before** including the header (or pass them as
build flags). Definitions after the `#include` have no effect.

```cpp
#define ANCS_MESSAGE_LEN 128   // shorter messages, less RAM
#define ANCS_MAX_PENDING 8     // more in-flight notifications
#include <ArduinoANCS.h>
```

## Available macros

| Macro | Default | Controls |
| --- | --- | --- |
| `ANCS_APP_ID_LEN` | 48 | Max bytes for `appIdentifier` (incl. NUL). |
| `ANCS_TITLE_LEN` | 48 | Max bytes for `title`. |
| `ANCS_SUBTITLE_LEN` | 48 | Max bytes for `subtitle`. |
| `ANCS_MESSAGE_LEN` | 256 | Max bytes for `message`. |
| `ANCS_DATE_LEN` | 16 | Max bytes for `date`. |
| `ANCS_MAX_PENDING` | 4 | Notifications awaiting metadata at once. |

Each `char[]` field costs its length in RAM **per pending slot**, so total usage
is roughly:

```
(APP_ID + TITLE + SUBTITLE + MESSAGE + DATE + overhead) × MAX_PENDING
```

With defaults that's about `(48+48+48+256+16) × 4 ≈ 1.7 KB`, comfortably within
the Uno R4 WiFi's 32 KB of RAM. On tighter boards, shrink `ANCS_MESSAGE_LEN` and
`ANCS_MAX_PENDING` first.

## What happens at the limits

- **Text longer than a buffer** is truncated to fit (always NUL-terminated). To
  capture long messages, raise `ANCS_MESSAGE_LEN`.
- **More simultaneous notifications than `ANCS_MAX_PENDING`** — extra ones are
  dropped until a slot frees up. A `MODIFIED` event for a notification already
  pending reuses its slot rather than consuming a new one.
- **Responses larger than the reassembly buffer** (sized to `ANCS_MESSAGE_LEN` +
  headroom) are discarded to stay memory-safe rather than overflow.

## Setting the advertised name

The device name iOS shows is the first argument to `begin()`:

```cpp
ANCS.begin("MyWatch");
```

## Managing BLE yourself

If your sketch already sets up BLE and its own services, let it own the stack and
have ArduinoANCS only handle the ANCS side:

```cpp
BLE.begin();
// ... add your own services, set up advertising ...
BLE.advertise();

ANCS.begin("MyDevice", /* manageBLE = */ false);
```

In this mode the library will **not** call `BLE.begin()`, add its helper
characteristic, or start advertising — that's your responsibility. Make sure
your advertising and a bondable/encrypted characteristic are in place so iOS
will pair.

## See also

- [API Reference](api-reference.md) — `begin()` parameters.
- [Troubleshooting](troubleshooting.md) — memory and stability issues.
