# Testing & acceptance

## Electrical

- [ ] MQ135 analog output is divided before GPIO35.
- [ ] LED outputs have 220Ω resistors.
- [ ] Buzzer is not directly connected to an ESP32 GPIO.
- [ ] Relay is only used for a properly powered low-voltage alarm load.

## Node 1

- [ ] BMP280 detected.
- [ ] BH1750 detected.
- [ ] LDR changes when covered/uncovered.
- [ ] MQ135 raw ADC changes after warm-up.
- [ ] HTTP response is 200.

## Node 2

- [ ] PIR detects motion.
- [ ] Sound threshold is adjusted with the onboard trimmer.
- [ ] LDR changes with ambient light.
- [ ] HTTP response is 200.

## Node 3

- [ ] Serial Monitor prints gateway IP.
- [ ] Dashboard opens in browser.
- [ ] `/api/state` returns JSON.
- [ ] Nodes show ONLINE.
- [ ] Baseline becomes available after 12 valid Node 1 packets.
- [ ] Red/green LEDs follow system state.
- [ ] Relay activates only at CRITICAL.
