# Node 1 — Environmental Edge Node

## Sensors
- BMP280: temperature + pressure
- BH1750: digital illuminance
- MQ135: relative gas/air-quality signal
- LDR #1: analog light signal

## Pins
- I2C SDA: GPIO21
- I2C SCL: GPIO22
- LDR AO: GPIO34
- MQ135 divider output: GPIO35

## Upload
1. Copy `config.h.example` to `config.h`.
2. Set Wi-Fi credentials and `GATEWAY_HTTP_BASE`.
3. Select ESP32 Dev Module.
4. Upload.
5. Open Serial Monitor at 115200.

## Output
The node prints sensor values and an HTTP response code. `200` means the gateway accepted the packet.
