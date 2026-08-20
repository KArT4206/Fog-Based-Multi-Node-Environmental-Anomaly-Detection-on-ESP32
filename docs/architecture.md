# Architecture

The final deployed topology contains three ESP32 boards. Nodes 1 and 2 are distributed edge sensing nodes; Node 3 is the local fog/edge gateway that aggregates, scores and serves the dashboard.

```text
ESP32 #1 (Environmental) ---- HTTP/Wi-Fi ----+
                                             |
ESP32 #2 (Safety) ----------- HTTP/Wi-Fi ----+--> ESP32 #3
                                                  Fog/Edge Gateway
                                                  - aggregation
                                                  - baselines
                                                  - context fusion
                                                  - anomaly score
                                                  - alerts
                                                  - web server
```

### Why this qualifies for the course prototype

The decision loop is moved close to the data sources. The gateway can act without an external cloud service, receives independent edge-node streams, fuses them and performs local control.

### Application layer

A normal browser on the same LAN reads `/api/state` and renders the dashboard. No external CDN is required.
