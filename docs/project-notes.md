# Build notes

- Final deployed topology: 3 ESP32 boards.
- Arduino Uno is optional for laboratory sensor testing, not part of the live topology.
- Keep all ESP32 boards on the same Wi-Fi network.
- Gateway IP is printed at 115200 baud.
- MQ135 values are raw/relative until calibrated.
- The MQ135 sensor can take time to stabilize. Do not use its first few readings as a final baseline.
- Change `SOUND_ACTIVE_STATE` in Node 2 if your sound module is inverted.
