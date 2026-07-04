# Examples

The library ships with runnable sketches under `examples/`. In the Arduino IDE
they appear at **File → Examples → ArduinoANCS**.

← Back to [documentation index](index.md)

## NotificationPrinter

Prints the full metadata of every incoming notification to Serial. This is the
best first sketch to confirm your setup works.

**Path:** `examples/NotificationPrinter/NotificationPrinter.ino`

It registers all four callbacks (`onNotification`, `onRemoved`, `onConnect`,
`onDisconnect`) and formats each notification like:

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

Use it with the [Getting Started](getting-started.md) walkthrough.

## CallResponder

Demonstrates `performAction()` by automatically **declining incoming calls**
that expose a negative action.

**Path:** `examples/CallResponder/CallResponder.ino`

Core logic:

```cpp
void onNotification(const ANCSNotification& n) {
  if (n.category == ANCS_CATEGORY_INCOMING_CALL && n.hasNegativeAction()) {
    ANCS.performAction(n.uid, ANCS_ACTION_NEGATIVE);
  }
}
```

See [Notification Actions](actions.md) for more on positive/negative actions and
a physical-button variant.

## Writing your own

Every sketch follows the same three-step shape:

```cpp
#include <ArduinoANCS.h>

void onNotification(const ANCSNotification& n) {
  // ...your logic...
}

void setup() {
  Serial.begin(9600);
  ANCS.onNotification(onNotification);   // 1. register callbacks
  ANCS.begin();                          // 2. start the library
}

void loop() {
  ANCS.poll();                           // 3. pump every iteration
}
```

From there, branch on `n.category`, `n.appIdentifier`, or `n.flags` to drive
LEDs, displays, buzzers, or actuators. See
[Handling Notifications](handling-notifications.md) for patterns.

## See also

- [Getting Started](getting-started.md)
- [Handling Notifications](handling-notifications.md)
- [Notification Actions](actions.md)
