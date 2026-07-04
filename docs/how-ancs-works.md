# How ANCS Works

This page explains the concepts behind the library. You don't need to read it to
use ArduinoANCS, but it helps a lot when debugging.

← Back to [documentation index](index.md)

## The "backwards" BLE roles

ANCS looks upside-down at first because two independent sets of roles are in
play:

- **Link-layer roles** — the Arduino *advertises* and the iPhone *connects*, so
  the Arduino is the **peripheral** and the iPhone is the **central**.
- **GATT roles** — the iPhone is the **server** that hosts the ANCS service, and
  the Arduino is the **client** that reads it.

So the Arduino is a *peripheral that acts as a GATT client on its central*. That
is exactly what `ANCS.begin()` sets up for you.

## Bonding is mandatory

iOS only exposes the ANCS characteristics over an **encrypted, bonded** link.
Until the two devices bond, discovery may succeed but subscribing to the ANCS
characteristics is rejected with *insufficient authentication*.

The library uses that rejection on purpose: the first `subscribe()` attempt is
what makes iOS raise its pairing prompt. Once you accept, the link becomes
encrypted and the next attempt succeeds. This is why `poll()` retries
subscription rather than giving up.

> ArduinoBLE does not persist bonds across a reset. After reflashing you may need
> to "Forget This Device" on the iPhone and pair again.

## The ANCS service

ANCS defines one service with three characteristics:

| Characteristic | UUID (short) | Direction | Purpose |
| --- | --- | --- | --- |
| Notification Source | `9FBF120D…` | notify | announces that a notification was added/modified/removed |
| Control Point | `69D1D8F3…` | write | request details, or perform an action |
| Data Source | `22EAC6E9…` | notify | streams back the requested details |

## The handshake, step by step

1. **Advertise & bond.** The Arduino advertises; the iPhone connects and bonds.
2. **Discover.** The Arduino discovers the ANCS service and its three
   characteristics on the iPhone.
3. **Subscribe.** It subscribes to *Notification Source* and *Data Source*.
4. **Notification arrives.** The iPhone sends an 8-byte *Notification Source*
   event: EventID, EventFlags, CategoryID, CategoryCount, and a 4-byte
   NotificationUID.
5. **Request details.** For added/modified events, the Arduino writes a
   *Get Notification Attributes* command to the *Control Point*, listing the
   attributes it wants (app id, title, subtitle, message, date).
6. **Receive details.** The iPhone streams the answer over *Data Source*. Large
   answers span several packets, so the library reassembles them before parsing.
7. **Deliver.** Once every requested attribute has arrived, the library fills an
   `ANCSNotification` and calls your `onNotification` callback.

Removed events skip steps 5–6 and are delivered straight to `onRemoved`.

## What the library does for you

- Owns the BLE stack, advertising, and the encrypted characteristic that nudges
  iOS to bond (unless you pass `manageBLE = false`).
- Runs the connect → discover → subscribe → receive state machine inside
  `poll()`, with ret/throttle logic for the bonding dance.
- Tracks in-flight notifications so replies match the right request.
- Reassembles fragmented Data Source responses and parses the attribute tuples.

## See also

- [API Reference](api-reference.md) — the surface you actually call.
- [Troubleshooting](troubleshooting.md) — when the handshake stalls.
