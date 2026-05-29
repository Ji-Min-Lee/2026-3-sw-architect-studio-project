# Experiment Results

**Milestone**: M2 | **Due**: 2026-06-22 | **Status**: [ ] Draft / [ ] Final

---

## Results Summary

| ID | Experiment | Status | Question Resolved? | Architecture Impact |
|----|------------|--------|-------------------|---------------------|
| EX-01 | Sample Rate Performance on RPi 5 | | | |
| EX-02 | Beat Event Detection Accuracy | | | |
| EX-03 | Filter Parameter Sweep | | | |
| EX-04 | Cross-Compilation & RPi Deploy | | | |
| EX-05 | Qt Multi-Tab Rendering Performance | | | |

---

## EX-01: Sample Rate Performance on Raspberry Pi 5

**Question**: What is the maximum sustained audio sample rate the RPi 5 can process while running the Qt GUI without dropping blocks?

### Method
```
[Describe what was done]
```

### Data

| Sample Rate | FPS | CPU % | Memory (MB) | Dropped Blocks / min | Assessment |
|-------------|-----|-------|-------------|----------------------|------------|
| 48,000 sps | | | | | |
| 96,000 sps | | | | | |
| 192,000 sps | | | | | |

### Conclusion
- **Recommended sample rate**: ___k sps
- **Rationale**: 
- **Architecture Decision**: 

---

## EX-02: Beat Event Detection Accuracy

**Question**: Which T1 detection reference point (onset vs peak) produces the most stable and accurate measurements?

### Method
```
[Describe what was done — watch used, recording conditions, comparison method]
```

### Data

| Method | Mean Rate Error (vs WeiShi) | Rate StdDev | Mean Beat Error Error | Beat Error StdDev |
|--------|-----------------------------|-------------|----------------------|--------------------|
| Onset  | | | | |
| Peak   | | | | |

### Conclusion
- **Chosen approach**: Onset / Peak *(circle one)*
- **Rationale**:
- **Architecture Decision**:

---

## EX-03: Filter Parameter Sweep

**Question**: What LP/HP cutoff values best preserve beat events while rejecting ambient noise?

### Method
```
[Describe test setup — noise types, sweep range]
```

### Data

| LP Cutoff (Hz) | HP Cutoff (Hz) | T1 SNR | T3 SNR | Detection Rate (%) | Notes |
|----------------|----------------|--------|--------|-------------------|-------|
| | | | | | |
| | | | | | |

### Conclusion
- **Recommended LP cutoff**: ___ Hz
- **Recommended HP cutoff**: ___ Hz
- **Architecture Decision**:

---

## EX-04: Cross-Compilation & RPi Deploy

**Question**: Can we build on macOS/Windows and deploy to RPi, or must we build natively on RPi?

### Method
```
[Steps attempted, tools used]
```

### Result
- **Build path chosen**: Cross-compile / Native on RPi *(circle one)*
- **Build time**: 
- **Issues encountered**:
- **Architecture Decision**: 

---

## EX-05: Qt Multi-Tab Rendering Performance

**Question**: Can Qt sustain target FPS with 3+ active graph tabs on RPi?

### Method
```
[Describe tab stub implementation, measurement method]
```

### Data

| Active Tabs | FPS (UI thread) | CPU % | Notes |
|-------------|-----------------|-------|-------|
| 1 | | | |
| 2 | | | |
| 3 | | | |
| 4 | | | |

### Conclusion
- **Threading model chosen**:
- **Render strategy**:
- **Architecture Decision**:

---

## Remaining Experiments

| ID | Title | Why Not Complete | Plan |
|----|-------|-----------------|------|
| | | | |

---

## Review Checklist

- [ ] All planned experiments have results or documented reason for incompletion
- [ ] Each result clearly resolves (or fails to resolve) the original question
- [ ] Architecture decisions updated based on results
- [ ] Remaining experiments listed if any
- [ ] Results are relevant to overall system goals
