# QA Team Consensus Preparation

> **Date**: 2026-06-02  
> **Purpose**: Pre-analysis and agenda items to drive team consensus at the workshop

---

## 1. QA Priority

### Decision criteria

> The QA that most constrains the architecture = top priority  
> This decision becomes the premise for all other design directions

### Proposed priority

| Rank | QA | Rationale |
|------|----|-----------|
| **1** | Real-Time Performance | RPi hardware limits are fixed. Falling below 48k sps means the project fails entirely. This is the prerequisite for every design decision. |
| **2** | Extensibility | Adding 11 graphs + Enhanced features is directly tied to the overall schedule. A poor structure creates schedule risk with every addition. |
| **3** | Measurement Accuracy | The reason the system exists. If the numbers are wrong, there is no value in the TimeGrapher. |
| **4** | Low Latency | Coupled to Real-Time Performance. Less decisive on its own. |
| **5** | Correctness | Follows naturally once Measurement Accuracy is secured. |

---

## 2. QA Trade-offs

### Conflicting relationships

| QA A | QA B | Relationship | Description |
|------|------|-------------|-------------|
| Real-Time Performance | Low Latency | ⚡ Conflict | Increasing throughput grows buffer size → latency increases |
| Real-Time Performance | Extensibility | ⚡ Conflict | Module separation (Extensibility) adds call overhead → potential processing performance degradation |
| Real-Time Performance | Measurement Accuracy | ⚡ Conflict | High-precision detection (high sps) increases computation → higher processing load |
| Measurement Accuracy | Correctness | ✅ Complementary | Accurate T1/T3 detection naturally secures internal consistency |
| Low Latency | Correctness | ✅ Complementary | Lower latency reduces stale data → improved consistency |

### Core trade-off

```
To preserve Real-Time Performance (priority 1):
  → How much module separation (Extensibility) can we allow?
  → How much buffer size (Low Latency) are we willing to sacrifice?
```

---

## 3. Workshop Consensus Agenda

### Agenda 1 — Finalize QA Priority

> Do we agree with the proposed order? Are there any points of disagreement?

- Discussion point: Extensibility vs. Measurement Accuracy ordering
  - If schedule risk is weighted more heavily → keep Extensibility at rank 2
  - If product quality is weighted more heavily → move Measurement Accuracy to rank 2

### Agenda 2 — Finalize Undecided Numbers

| QA | Undecided item | How to finalize |
|----|---------------|----------------|
| Low Latency | Target ms for each of the 3 segments | After Experiment 1·2 results (tentative values can be set today) |
| Correctness | Acceptable deviation range | Team consensus today |
| Measurement Accuracy | Error vs. WeiShi (s/d, ms) | After Experiment 3 results |
| Extensibility | Upper limit on changed files | Team consensus today |

### Agenda 3 — Decide Team Position on the Core Trade-off

> When conflict arises with Real-Time Performance (rank 1), what is the team's default stance?

- **Option A**: Performance first — minimize module separation, accept higher latency
- **Option B**: Structure first — secure Extensibility even at some performance cost
- **Option C**: Decide after experiments — wait for Experiment 1 results before deciding

---

## 4. Consensus Record

> Fill in the table below on the day of the workshop

| Item | Decision | Notes |
|------|----------|-------|
| QA priority | | |
| Low Latency target ms (tentative) | | |
| Correctness acceptable deviation | | |
| Extensibility file count upper limit | | |
| Trade-off default stance | | |
