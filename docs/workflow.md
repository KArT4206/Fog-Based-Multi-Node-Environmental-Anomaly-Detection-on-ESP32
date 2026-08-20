# Workflow

1. ESP32 Node 1 samples BMP280, BH1750, MQ135 and LDR.
2. ESP32 Node 2 samples PIR, sound and a second LDR.
3. Both nodes connect to the same Wi-Fi network.
4. Each node sends a small HTTP GET packet to Node 3.
5. Node 3 stores the newest reading and packet age.
6. Node 3 learns a short-term baseline from the first valid Node 1 packets.
7. Node 3 computes temperature deviation, gas ratio and multi-sensor context.
8. The gateway maps evidence to NORMAL, WARNING or CRITICAL.
9. LED indicators and relay output are updated locally.
10. The browser polls `/api/state` and updates metrics and charts every two seconds.
