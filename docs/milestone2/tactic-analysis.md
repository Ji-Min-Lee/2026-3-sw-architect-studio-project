# EXP-02 전술 분석 — ADD 설계 결정 및 QA 연관 / EXP-02 Tactic Analysis — ADD Design Decisions and QA Mapping

> **작성일 / Date**: 2026-06-15  
> **출처 / Source**: EXP-02 macOS 실험 결과 (`exp-02-baseline-results.md`), Bass/Clements/Kazman *Software Architecture in Practice*

---

## 1. 개요 / Overview

**한국어**

EXP-02에서 적용한 두 가지 개선 전술(T2, R1)을 SW Architecture 프레임워크 기준으로 분류하고,
TimeGrapher 프로젝트의 Quality Attribute와의 연관 관계를 기록한다.

**English**

This document classifies the two tactics applied in EXP-02 (T2, R1) using the SW Architecture framework
and records their relationship to the TimeGrapher project's Quality Attributes.

---

## 2. 전술 분류 (Bass/Clements/Kazman 기준) / Tactic Classification

**한국어**

두 전술 모두 **Performance Tactic** 범주에 속하되, 하위 전략이 다르다.

**English**

Both tactics belong to the **Performance Tactic** category, but with different sub-strategies.

| 항목 / Item | T2: DSP Offload Thread | R1: Lazy Rendering |
|------------|------------------------|-------------------|
| **상위 범주 / Category** | Performance Tactic | Performance Tactic |
| **하위 전략 / Sub-strategy** | Manage Resources → **Introduce Concurrency** | Control Resource Demand → **Reduce Computational Overhead** |
| **핵심 원리 / Principle** | 독립 스레드로 DSP 분리, Audio·UI 스레드 경합 제거 / Separate DSP onto a dedicated thread, eliminate audio–UI thread contention | 숨겨진 탭 replot 조건부 skip / Conditionally skip replot() for non-visible tabs |
| **실측 결과 / Measured Result** | wait_ms 420ms → 0.013ms, backlog 47% → 0% | replot_count 8.22 → 2.08 (↓75%, macOS) |

---

## 3. ADD 설계 결정 기록 / ADD Design Decision Records

### 3.1 ADD-2-01: T2 DSP Offload Thread

**한국어**

```
결정 ID   : ADD-2-01
결정 내용  : Introduce Concurrency — DSPWorker 별도 스레드 분리 (T2)
해결 QA   : Real-Time Performance, Low Latency
근거      : Audio 스레드 → Main 스레드 Qt 큐 대기(wait_ms 420ms)가 backlog(47%)의 원인.
            DSP를 독립 스레드로 분리하면 Qt 이벤트 루프 병목이 제거됨.
대안      :
  - T1 (SCHED_RR + CPU Affinity): Linux 전용, RPi에서는 유효하나 macOS 검증 불가
  - T3 (Full Pipeline Split): 복잡도 과도하게 증가, MVP 범위 초과
트레이드오프: 스레드 간 데이터 공유 동기화 복잡도 증가 (Qt::QueuedConnection으로 완화)
결과      : backlog 0%, wait_ms ×32,000 감소 — QAS-1(Real-Time) + QAS-2(Low Latency) 동시 해결
```

**English**

```
Decision ID : ADD-2-01
Decision    : Introduce Concurrency — offload DSP to a dedicated DSPWorker thread (T2)
QA resolved : Real-Time Performance, Low Latency
Rationale   : Qt queue wait from Audio thread → Main thread (wait_ms 420ms) was the root cause
              of 47% backlog. Moving DSP to its own thread eliminates the event-loop bottleneck.
Alternatives:
  - T1 (SCHED_RR + CPU Affinity): Linux-only; valid on RPi but cannot validate on macOS
  - T3 (Full Pipeline Split): excessive complexity, beyond MVP scope
Trade-off   : Increased synchronization complexity across threads (mitigated by Qt::QueuedConnection)
Consequence : backlog 0%, wait_ms ×32,000 reduction — resolves QAS-1 (Real-Time) + QAS-2 (Low Latency)
```

---

### 3.2 ADD-2-02: R1 Lazy Rendering

**한국어**

```
결정 ID   : ADD-2-02
결정 내용  : Reduce Computational Overhead — isVisible() 가드 + showEvent catch-up (R1)
해결 QA   : Real-Time Performance (RPi), Usability
근거      : 숨겨진 탭도 매 beat마다 replot()을 호출 → 불필요한 GPU/CPU 소비.
            visible 탭만 렌더링하고, 탭 전환 시 showEvent()에서 catch-up replot으로 UX 유지.
대안      :
  - R2 (Timer-Decoupled 20FPS): macOS에서 spike 미관찰 → 현재 불필요.
            RPi R5 측정 후 spike 기준 충족 시 재검토 (§ RPi R2 판단 기준 참조)
  - R3 (Double-Buffer Async): 구현 복잡도 높음, MVP 범위 초과
트레이드오프: 탭 전환 시 일시적 stale view 가능 (QTimer::singleShot(0)으로 완화)
결과      : replot 75~85% 감소 (macOS 실측). RPi에서 plot_ms ~14ms 절감 기대
```

**English**

```
Decision ID : ADD-2-02
Decision    : Reduce Computational Overhead — isVisible() guard + showEvent catch-up (R1)
QA resolved : Real-Time Performance (RPi), Usability
Rationale   : Non-visible tabs called replot() every beat → unnecessary GPU/CPU consumption.
              Render only the visible tab; catch-up via showEvent() preserves UX on tab switch.
Alternatives:
  - R2 (Timer-Decoupled 20FPS): no spike observed on macOS → not needed now.
              Revisit after RPi R5 measurement if trigger criteria are met (see § R2 Trigger Criteria)
  - R3 (Double-Buffer Async): high implementation complexity, beyond MVP scope
Trade-off   : Temporarily stale view on tab switch (mitigated by QTimer::singleShot(0))
Consequence : 75–85% replot reduction (macOS measured). Expected ~14ms plot_ms saving on RPi
```

---

## 4. Quality Attribute 연관 매핑 / QA Mapping

**한국어**

TimeGrapher 프로젝트 QA 기준으로 각 전술의 연관 강도를 정리한다.
- ✅ 직접: 해당 전술이 이 QA를 직접 개선
- 🔸 간접: 부수적 효과

**English**

Maps each tactic to TimeGrapher QAs by contribution strength.
- ✅ Direct: this tactic directly improves the QA
- 🔸 Indirect: secondary benefit

| QA | 설명 / Description | T2 DSP Offload | R1 Lazy Rendering |
|----|-------------------|:--------------:|:-----------------:|
| **Real-Time Performance** | 96kHz, Dropped Block=0 | ✅ backlog 0% 달성 / achieved | ✅ RPi replot_ms 절감 / saves plot_ms on RPi |
| **Low Latency** | capture→process→GUI E2E | ✅ wait_ms ×32,000 감소 / reduced | 🔸 렌더링 경쟁 감소 / less render contention |
| **Correctness** | 측정값 정확도 / Measurement accuracy | 🔸 deadline miss 0% → 샘플 누락 방지 / no sample drop | 🔸 동일 / same |
| **Usability** | 탭 전환 UX / Tab-switch UX | — | ✅ catch-up replot으로 최신 데이터 즉시 반영 / instant refresh |
| **Extensibility** | 새 탭 추가 / Adding new tabs | — | 🔸 `BaseGraphTab::replotAll()` 표준 인터페이스 / standard interface |

---

## 5. R2 적용 판단 기준 (RPi R5 이후) / R2 Trigger Criteria (after RPi R5)

**한국어**

macOS 실험에서 R1 spike가 관찰되지 않아 R2는 현재 Skip 결정.
RPi R5 측정 후 아래 조건 중 하나 충족 시 R2(Timer-Decoupled 20FPS) 적용을 재검토한다.

**English**

R2 is skipped for now as no R1 spike was observed on macOS.
Revisit R2 (Timer-Decoupled 20FPS) after RPi R5 if any of the following is triggered.

| 관찰 항목 / Observation | 임계값 / Threshold | 비고 / Note |
|------------------------|-------------------|------------|
| replot_count 순간 max / instantaneous max | > 20 (burst) | showEvent catch-up이 한 프레임에 집중되는 경우 / catch-up concentrated in one frame |
| exec_ms spike (탭 전환 직후 / post tab-switch) | > deadline × 2 | catch-up replot이 exec path에 영향 / leaking into exec path |
| UI 프리즈 체감 / UI freeze | 탭 전환 시 >200ms / >200ms on switch | singleShot(0) 지연으로도 해소 안 되는 경우 / singleShot(0) insufficient |
| replot_count avg (RPi) | > 5 / beat | R1 효과가 macOS보다 미달 시 R2의 FPS 고정이 더 유리 / fixed FPS cap preferred |

**한국어**

조건 미충족 시 R2는 Skip하고 T1(SCHED_RR + CPU Affinity) 적용(RPi R6)으로 진행한다.

**English**

If none of the above is triggered, skip R2 and proceed to T1 (SCHED_RR + CPU Affinity) in RPi R6.

---

## 6. 참고 / References

- `docs/milestone2/exp-02-baseline-results.md` — 실험 결과 상세 / Detailed experiment results
- `docs/milestone2/architectural-approaches.md` — 전술 옵션 및 trade-off 분석 / Tactic options and trade-off analysis
- Bass, L., Clements, P., Kazman, R. *Software Architecture in Practice*, 3rd ed. — Performance Tactics (Ch. 8)
