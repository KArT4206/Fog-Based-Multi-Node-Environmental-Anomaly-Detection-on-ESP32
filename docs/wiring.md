# Wiring

## Node 1

**BMP280**: VCC->3.3V, GND->GND, SDA->GPIO21, SCL->GPIO22.

**BH1750**: VCC->3.3V, GND->GND, SDA->GPIO21, SCL->GPIO22. Both share the I2C bus.

**LDR #1**: VCC->3.3V, GND->GND, AO->GPIO34.

**MQ135**: VCC->5V, GND->GND, AO through a divider -> GPIO35. Use 10k top and two 10k resistors in series for the lower 20k leg.

## Node 2

**PIR**: VCC->5V, GND->GND, OUT->GPIO27.

**Sound**: +->3.3V, G->GND, DO->GPIO32. AO is unused.

**LDR #2**: VCC->3.3V, GND->GND, AO->GPIO33.

## Node 3

**Red LED**: GPIO14 -> 220Ω -> anode; cathode -> GND.

**Green LED**: GPIO12 -> 220Ω -> anode; cathode -> GND.

**Relay**: VCC->5V, GND->GND, IN->GPIO26.

**Buzzer**: do not drive the photographed 6–12V buzzer directly from an ESP32 GPIO. Use the relay contacts to switch an appropriate external low-voltage supply.
