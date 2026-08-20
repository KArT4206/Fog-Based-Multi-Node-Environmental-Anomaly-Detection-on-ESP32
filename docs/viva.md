# Viva / panel explanation

### What is the problem?
Traditional IoT monitoring often sends data to centralized cloud services or uses independent fixed thresholds. That can add dependency and cause false positives when a single sensor is noisy.

### What is the solution?
The system distributes sensing across two edge nodes and performs local correlation at a third gateway. The gateway learns a short-term baseline and combines multiple signals before issuing an alert.

### Why three ESP32 boards?
Two boards represent physically distributed sensing locations or zones. The third board is the fog/edge processing point. This makes the multi-node architecture explicit rather than simulated.

### Why not call MQ135 output ppm?
Because a raw gas-sensor ADC value is not automatically a calibrated ppm measurement. The prototype therefore uses a relative signal and baseline ratio.

### Is machine learning implemented?
No. The current prototype uses an explainable rule-based anomaly score. ML can be added after collecting a labeled dataset.

### Why context fusion?
Motion alone may be harmless; sound alone may be harmless. Their co-occurrence is stronger evidence. Likewise, gas elevation combined with temperature rise is more meaningful than a single noisy gas reading.

### Is the system patent-level?
It is technically structured like a research prototype, but patentability cannot be assumed. A formal prior-art and novelty search is required before making patent claims.
