# Notification Actions

Some notifications expose actions — like **Accept**/**Decline** on an incoming
call. You can trigger those from the Arduino with `performAction()`.

← Back to [documentation index](index.md)

## The API

```cpp
bool ANCS.performAction(uint32_t uid, uint8_t actionId);
```

- `uid` — the `uid` from the `ANCSNotification`.
- `actionId` — `ANCS_ACTION_POSITIVE` or `ANCS_ACTION_NEGATIVE`.

Returns `false` if ANCS isn't ready or the write failed.

## Positive vs negative

- **Positive** (`ANCS_ACTION_POSITIVE`) is the affirmative action — e.g. *Answer*
  a call.
- **Negative** (`ANCS_ACTION_NEGATIVE`) is the dismissive action — e.g. *Decline*
  a call or clear the notification.

An action only works if the notification advertised it. Always check first:

```cpp
if (n.hasPositiveAction()) ANCS.performAction(n.uid, ANCS_ACTION_POSITIVE);
if (n.hasNegativeAction()) ANCS.performAction(n.uid, ANCS_ACTION_NEGATIVE);
```

## Example: decline incoming calls

```cpp
#include <ArduinoANCS.h>

void onNotification(const ANCSNotification& n) {
  if (n.category == ANCS_CATEGORY_INCOMING_CALL && n.hasNegativeAction()) {
    Serial.println("Declining call");
    ANCS.performAction(n.uid, ANCS_ACTION_NEGATIVE);
  }
}

void setup() {
  Serial.begin(9600);
  ANCS.onNotification(onNotification);
  ANCS.begin();
}

void loop() { ANCS.poll(); }
```

This is the bundled **CallResponder** example — see [Examples](examples.md).

## Example: a physical answer/decline button

Store the most recent actionable call's `uid`, then act on a button press:

```cpp
uint32_t pendingCall = 0;

void onNotification(const ANCSNotification& n) {
  if (n.category == ANCS_CATEGORY_INCOMING_CALL) pendingCall = n.uid;
}

void loop() {
  ANCS.poll();

  if (pendingCall && digitalRead(ANSWER_PIN) == LOW) {
    ANCS.performAction(pendingCall, ANCS_ACTION_POSITIVE);
    pendingCall = 0;
  }
  if (pendingCall && digitalRead(DECLINE_PIN) == LOW) {
    ANCS.performAction(pendingCall, ANCS_ACTION_NEGATIVE);
    pendingCall = 0;
  }
}
```

## Notes

- Acting on a stale `uid` (one that's already been dismissed) simply fails
  quietly; iOS ignores unknown UIDs.
- Not every notification is actionable. Messages and most app notifications have
  no positive/negative action — only certain categories (notably calls) do.

## See also

- [Handling Notifications](handling-notifications.md) — reading flags and categories.
- [API Reference](api-reference.md#bool-performactionuint32_t-uid-uint8_t-actionid) — method details.
