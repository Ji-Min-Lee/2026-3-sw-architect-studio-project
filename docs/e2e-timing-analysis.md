# E2E Timing Analysis — TimeGrapher Audio Pipeline

## 1. 목적

오디오 파이프라인의 구간별 latency를 측정하여 real-time performance를 정량적으로 파악한다.

---

## 2. 시스템 구조

```
[마이크]
   │
   │ OS WASAPI (10ms 버퍼링)
   ▼
[BG Thread: TAudioWorker]
   QIODevice::readyRead
     → ProcessAudioInput()
         → 링버퍼(mRawAudio->Samples)에 PCM 저장
         → emit AudioDataReady(bg_event_audio_ready)
   │
   │ Qt Signal (값 복사로 큐 삽입)
   ▼
[FG Thread: Main Thread / Qt Event Loop]
   HandleAudioInput(bg_event_audio_ready)
     → HandleInputData()
         → ProcessSamples()
             → tg_process(), replot(), DrawImage()
   │
   ▼
[화면 출력]
```

---

## 3. 타임스탬프 정의

| 변수 | 정의 | 측정 위치 |
|------|------|-----------|
| `bg_event_audio_ready` | BG가 emit 직전 시각 | AudioWorker.cpp |
| `fg_handler_start` | FG HandleInputData 진입 시각 | MainWindow.cpp |
| `fg_handler_end` | FG HandleInputData 완료 시각 | MainWindow.cpp |

```
bg_event_audio_ready
        │
        │←── wait_us ───────────►│
                                 fg_handler_start
                                        │
                                        │←── exec_us ──────────►│
                                                                 fg_handler_end
        │←────────────── total (wait + exec) ──────────────────►│
```

---

## 4. 측정 지표

### wait_us
- **정의**: `fg_handler_start - bg_event_audio_ready`
- **의미**: BG emit 후 FG가 처리 시작하기까지 대기 시간 (Qt 이벤트 큐 대기 + OS 스케줄링)
- **정상**: 50~300 us (OS 스케줄링 오버헤드만)
- **이상**: > 1ms → 이전 프레임 exec이 길었거나 Main Thread가 다른 이벤트 처리 중

### exec_us
- **정의**: `fg_handler_end - fg_handler_start`
- **의미**: FG 실제 처리 시간 (ProcessSamples 전체)
- **정상**: < 5ms
- **경계**: < 10ms (BG 주기 이내)
- **위험**: > 10ms → 다음 프레임 wait_us 증가 원인

### exec 세부 구간
| 구간 | 내용 | 특성 |
|------|------|------|
| copy | 링버퍼 → mInputBlock memcpy | samples에 선형 비례 |
| sound | SoundImageRenderer.processSamples() | samples에 선형 비례 |
| tg | tg_process() — 박동 검출 알고리즘 | samples에 선형 비례 |
| ui | ScopePlot addData(), A/C 이벤트 마커 | samples에 선형 비례 |
| purge | PurgeHistory() — 오래된 그래프 데이터 삭제 | 간헐적 비용 |
| plot | replot() + DrawImage() | **고정 비용 (samples 무관)** |

### samples
- **정의**: `mLocalTotalSamplesWritten - MainThrd_LastTotalSamplesWritten`
- **의미**: 이번 HandleInputData 호출 시 FG가 처리해야 할 누적 샘플 수
- **정상**: 480 (1프레임치, 48000Hz × 10ms)
- **이상**: 960, 1440... → FG가 BG를 못 따라가고 있음

### total (= wait + exec)
- **의미**: 코드에서 측정 가능한 E2E latency
- **완전한 E2E**: WASAPI 버퍼링(~10ms) + total

---

## 5. Real-time Performance 기준

```
BG 주기 = 10ms (WASAPI 48000Hz → 480샘플/프레임)

exec_us < 10ms   → 정상, 프레임 안 밀림  ✅
exec_us > 10ms   → 다음 프레임 samples 누적 시작  ⚠️
Rollover 발생    → 링버퍼 오버플로우, 샘플 유실  ❌

wait_us ≈ 0      → FG 여유 있음  ✅
wait_us 폭발     → Main Thread가 다른 이벤트에 점유됨  ⚠️
```

---

## 6. T0-T1 페어링 안전성

`bg_event_audio_ready`는 signal **인자(값 복사)**로 전달된다.

```
emit AudioDataReady(nowUs())
                    ↑
                    Qt가 이벤트 객체에 복사 → 큐에 삽입
                    BG가 다음 프레임에 덮어쓸 수 없음

→ 큐에 이벤트가 쌓여도 각 이벤트가 자신의 T0를 독립적으로 보유
→ wait_us = T1 - T0 >= 0 항상 보장
```

**이전 방식(공유 메모리 저장)의 race condition을 signal 인자로 해결.**

---

## 7. 로그 형식

```
[000100] samples=480   BG: fps=100.0  sps=48000.0  spf=480.0   FG: fps=99.8   sps=47900.0  spf=480.0
[000100] total=2.15ms [wait=0.18 + exec=1.97]  exec=[copy=0.004 sound=0.001 tg=0.390 ui=0.013 purge=0.005 plot=1.450] ms
```

- 첫 번째 줄: BG/FG throughput 통계
- 두 번째 줄: E2E timing (total = wait + exec, exec 세부 구간)
- 100 프레임마다 출력 (≈ 1회/초)
- `samples > 0` 인 프레임만 카운트 (유효 프레임)

---

## 8. 실측 로그 분석 결과

### 정상 구간
```
exec ≈ 0.8~2ms   → 10ms 이내 ✅
wait ≈ 0.1~0.3ms → OS 스케줄링 오버헤드만 ✅
plot이 exec의 ~70~80% 차지 (dominant)
```

### 이상 구간
```
[005000] samples=6240   wait=128ms  → 13프레임 누적
[005900] samples=10560  wait=216ms  → 22프레임 누적
[005700] samples=2880   wait=51ms   →  6프레임 누적
```

### 패턴
```
exec < 3ms (항상 건강)
wait 폭발 → samples 폭발 → FG SPF 폭발

FG SPF 폭발 원인:
  samples=10560을 1번의 HandleInputData로 처리
  → mForegroundFrameCount += 1
  → mForegroundSampleCount += 10560
  → SPF = 10560 / 1 = 10560 (폭발)
```

### 병목 결론
```
exec 자체는 문제없음 (< 3ms)
wait 폭발의 원인:
  Main Thread가 HandleInputData 외 다른 이벤트 처리 중
  유력 후보: replot(rpQueuedReplot)이 큐잉한 QPaintEvent 렌더링
  → 추후 replot 비활성화 실험으로 확인 필요
```

---

## 9. 코드 변경 내역

### SharedAudio.h
- `#include <cstdint>` 추가

### AudioWorker.cpp / PlaybackWorker.cpp / SimWorker.cpp
- `#include <chrono>` 추가
- `nowUs()` 헬퍼 함수 추가
- `emit AudioDataReady()` → `emit AudioDataReady(nowUs())` (T0를 signal 인자로 전달)

### AudioWorker.h / PlaybackWorker.h / SimWorker.h
- signal 시그니처 변경: `void AudioDataReady()` → `void AudioDataReady(int64_t bg_event_audio_ready)`

### MainWindow.h
- 슬롯 시그니처 변경: `HandleAudioInput(int64_t bg_event_audio_ready)` 등
- `TExecBreakdown` 구조체 추가 (exec 세부 구간 전달용)
- `ProcessSamples(TMasterAudioDataRaw*, TExecBreakdown&)` 시그니처 변경

### MainWindow.cpp
- `nowUs()` 헬퍼 함수 추가
- `HandleInputData()`: wait_us, exec_us, total 측정 및 로그 출력
- `ProcessSamples()`: 구간별 타이밍 측정, TExecBreakdown 채움
- `PurgeHistory()` 타이밍 추가 (exec gap 해소)

### CMakeLists.txt
- `WIN32_EXECUTABLE TRUE` → `WIN32_EXECUTABLE FALSE` (콘솔 출력 활성화)
