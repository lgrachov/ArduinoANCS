# Handling Notifications

How to work with the `ANCSNotification` struct your callbacks receive.

← Back to [documentation index](index.md)

## The callback

Register a handler and read the fields you care about:

```cpp
void onNotification(const ANCSNotification& n) {
  Serial.print(n.appIdentifier); Serial.print(" | ");
  Serial.print(n.title);         Serial.print(": ");
  Serial.println(n.message);
}

void setup() {
  Serial.begin(9600);
  ANCS.onNotification(onNotification);
  ANCS.begin();
}

void loop() { ANCS.poll(); }
```

`onNotification` fires only after the full metadata (title, message, etc.) has
been fetched from the iPhone.

## Added, modified, removed

Every notification carries an `event`:

```cpp
switch (n.event) {
  case ANCS_EVENT_ADDED:    /* brand new notification */    break;
  case ANCS_EVENT_MODIFIED: /* updated (e.g. call ended) */ break;
  case ANCS_EVENT_REMOVED:  /* dismissed */                 break;
}
```

Added and modified events go to `onNotification` (with metadata). Removed events
go to `onRemoved` and only carry the header fields (`uid`, `category`, etc.) —
the text fields are empty because iOS provides no metadata for a dismissal.

```cpp
void onRemoved(const ANCSNotification& n) {
  Serial.print("dismissed uid="); Serial.println(n.uid);
}
ANCS.onRemoved(onRemoved);
```

## Categories

`category` tells you the kind of notification, and `categoryName()` gives a
readable string:

```cpp
Serial.println(n.categoryName());   // e.g. "Social", "Email", "IncomingCall"

if (n.category == ANCS_CATEGORY_INCOMING_CALL) {
  // ring a buzzer, light an LED, etc.
}
```

See the full list in the [API Reference](api-reference.md#anscategory).

## Flags

`flags` is a bitmask. Test individual bits, or use the helper methods:

```cpp
if (n.flags & ANCS_FLAG_IMPORTANT) Serial.println("(important)");
if (n.flags & ANCS_FLAG_SILENT)    Serial.println("(silent)");

if (n.hasPositiveAction()) Serial.println("has an Accept action");
if (n.hasNegativeAction()) Serial.println("has a Decline action");
```

`ANCS_FLAG_PRE_EXISTING` marks notifications that were already on the phone when
you connected (a backlog), versus ones that arrived live.

## Filtering by app

`appIdentifier` is the app's bundle id. Match on it to react to specific apps:

```cpp
if (strcmp(n.appIdentifier, "com.apple.MobileSMS") == 0) {
  // it's a Messages notification
}
```

Common bundle ids: `com.apple.MobileSMS` (Messages),
`com.apple.mobilecal` (Calendar), `com.apple.mobilephone` (Phone),
`net.whatsapp.WhatsApp`, `com.facebook.Facebook`.

## Lifecycle callbacks

Track connection state to drive a status LED or reset your own logic:

```cpp
void onConnect()    { Serial.println("iPhone connected"); }
void onDisconnect() { Serial.println("iPhone gone"); }

ANCS.onConnect(onConnect);
ANCS.onDisconnect(onDisconnect);
```

Or poll it directly:

```cpp
if (ANCS.ready()) { /* notifications are flowing */ }
```

## Keep callbacks short

Callbacks run inside `poll()`. Avoid long `delay()`s or blocking work in them —
copy what you need into your own variables and act on it from `loop()` instead.

## See also

- [Notification Actions](actions.md) — respond to calls and prompts.
- [API Reference](api-reference.md) — every field and enum.
