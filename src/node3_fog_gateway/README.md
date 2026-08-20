# Node 3 — Fog / Edge Gateway

## Responsibilities
- Receive Node 1 and Node 2 telemetry.
- Learn temperature and MQ135 baselines.
- Perform context-aware anomaly scoring.
- Mark nodes offline when packets are stale.
- Control LEDs and relay.
- Serve the browser dashboard.

## Pins
- Red LED: GPIO14 through 220 Ω
- Green LED: GPIO12 through 220 Ω
- Relay IN: GPIO26

## Endpoints
- `/` — web dashboard
- `/api/state` — JSON current state
- `/api/node1` — Node 1 telemetry receiver
- `/api/node2` — Node 2 telemetry receiver
