# Troubleshooting

Fixes for the most common issues. If you're not sure what's happening under the
hood, read [How ANCS Works](how-ancs-works.md) first.

← Back to [documentation index](index.md)

## No pairing prompt / never becomes `ready()`

iOS only exposes ANCS over a **bonded** link, and the pairing prompt is
triggered by the library's first subscription attempt.

1. On the iPhone, go to **Settings → Bluetooth**, tap the ⓘ next to
   **Arduino-ANCS**, and choose **Forget This Device**. A stale bond from an
   earlier attempt will block re-pairing.
2. Reset the Arduino and reconnect from the iPhone.
3. Accept **both** prompts: the pairing prompt *and* the "allow notifications"
   prompt.

If you still get no prompt, force bonding from the phone side with a BLE tool
like **nRF Connect**: connect, read the encrypted `2A3D` characteristic, accept
pairing, and iOS will then expose ANCS.

## Bond lost after reset or reflash

ArduinoBLE does not persist bonds across a reboot. After resetting or uploading
new firmware, "Forget This Device" on the iPhone and pair again.

## Connects, then immediately disconnects

- Make sure `ANCS.poll()` is called on **every** `loop()` iteration with no long
  `delay()` blocking it — the BLE stack needs to be serviced continuously.
- Avoid heavy blocking work inside callbacks; they run inside `poll()`.

## Discovery succeeds but subscription keeps failing

This is the bonding step. The repeated subscribe attempts are expected — they're
what raise the pairing prompt. If it loops for more than ~30 seconds with no
prompt on the phone, the bond isn't forming: forget the device and retry as
above. Some ArduinoBLE cores are more reliable at auto-initiating pairing than
others.

## Long messages are cut off

Text is truncated to the buffer size. Increase `ANCS_MESSAGE_LEN` before
including the header — see [Configuration & Memory](configuration.md).

## Some notifications never arrive

- iOS suppresses ANCS delivery for some notifications depending on Focus modes,
  Do Not Disturb, and per-app notification settings. Check the phone's settings.
- Notifications already on screen when you connect are delivered with the
  `ANCS_FLAG_PRE_EXISTING` flag set; brand-new ones arrive live.

## Nothing prints on Serial at all

- Confirm the Serial Monitor baud rate matches the sketch (`9600` in the
  examples).
- The examples call `while (!Serial);` — the sketch waits for the Serial Monitor
  to open before running.

## Wrong board / BLE won't start

`begin()` returns `false` if the BLE stack fails to start. Confirm you selected
**Arduino UNO R4 WiFi** (or another ArduinoBLE board with bonding support) and
that the **ArduinoBLE** library is installed.

## Still stuck?

Re-read [How ANCS Works](how-ancs-works.md) to map the symptom to a step in the
handshake, then check the [API Reference](api-reference.md) to confirm your
calls match.
