# Node 2 — Safety Edge Node

## Sensors
- PIR: motion event
- KY-037/KY-038-style sound module: digital threshold event
- LDR #2: analog light signal

## Pins
- PIR OUT: GPIO27
- Sound DO: GPIO32
- LDR AO: GPIO33

## Sound polarity
The firmware defaults to `SOUND_ACTIVE_STATE = LOW`. Change it to `HIGH` if the module behaves in the opposite way.
