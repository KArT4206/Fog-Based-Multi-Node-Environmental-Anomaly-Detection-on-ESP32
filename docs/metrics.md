# Metrics & experimental evaluation

These are **metrics to measure**, not claimed results.

| Metric | Formula / definition | Collection method |
|---|---|---|
| End-to-end latency | dashboard update time - sensor sample time | timestamps or controlled event video |
| Packet delivery ratio | received / sent | compare node HTTP 200 count with gateway packet count |
| Node availability | online time / experiment time | gateway packet-age timeout |
| Detection rate (TPR) | true positives / total events | labeled controlled trials |
| False-positive rate | false alarms / normal trials | repeated normal environment trials |
| Mean alert response | average event-to-alert time | GPIO timestamp or high-frame-rate video |
| Sensor stability | standard deviation / coefficient of variation | steady-state sample window |
| Network overhead | bytes per packet × packets per second | request-size estimate |
| Power | voltage × current | USB meter / multimeter |

## Suggested benchmark

Run each scenario at least 20 times and record raw results before calculating percentages. Do not report a single successful demo as an accuracy claim.

### Example result sheet

```text
Scenario: motion + sound
Trials: 20
True detections: ____
False positives: ____
Misses: ____
Mean alert latency: ____ ms
P95 alert latency: ____ ms
```
