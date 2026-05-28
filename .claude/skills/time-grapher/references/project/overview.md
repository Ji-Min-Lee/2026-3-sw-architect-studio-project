# TimeGrapher — 프로젝트 개요

## 배경

기계식 시계(mechanical watch)의 이스케이프먼트 진동 시 발생하는 음향 신호(beat noise)를 분석해
시계 성능을 진단하는 장비를 **Timegrapher**라 한다.
마이크에 시계를 올려두면 tick-tock 소리를 분석해 아래 측정값을 실시간 출력한다.

| 측정값 | 설명 | 정상 기준 |
|--------|------|-----------|
| Rate (s/d) | 하루 기준 빠름/느림 편차 | ±5 s/d 이내 권장 |
| Amplitude (°) | 밸런스 휠 각도 스윙 | 270~310° (강함), 220~250° (허용) |
| Beat Error (ms) | tick-tock 비대칭 | 0.6 ms 이하 |
| BPH | Beats Per Hour | 21,600 / 28,800 / 36,000 등 |

## 음향 이벤트 구조 (Swiss Lever Escapement)

1비트당 3개의 음향 이벤트가 발생한다.

```
T1 (A): Impulse pin → Pallet fork    ← 가장 정밀 → Rate / Beat Error 계산
T2 (B): Escape wheel → Pallet stone  ← 불규칙    → 측정 미사용
T3 (C): Escape wheel locks + Fork → Banking pin  ← T1과 함께 → Amplitude 계산
```

## 프로젝트 목표

베이스라인 `TimeGrapher_v10.5` GUI를 확장해 **Raspberry Pi 5** 위에서 동작하는
실시간 진단·시각화 시스템을 설계·구현한다.

- **Signal Detection** — beat event(T1, T3) 정밀 감지
- **Signal Processing** — 실시간 필터링 (Low-pass / High-pass)
- **Real-Time Analysis** — Rate·Amplitude·Beat Error 계산
- **Real-Time Graphing** — 다양한 디스플레이 모드 구현

아키텍처 관점에서의 핵심 목표:
> 모듈형·확장 가능·유지보수 가능한 구조로 설계하여 새 분석·그래프를 기존 코드 대규모 수정 없이 추가한다.

## 시스템 구성

```
[Watch]
  ↓ 진동 (음향 신호)
[USB Sensor Stand + Converter Box]
  ↓ USB (analog → digital)
[Raspberry Pi 5]
  ↓ Qt GUI (TimeGrapher_v10.5 기반)
[8" Touchscreen (1280×800)]

[개발 환경: macOS/Windows PC]  ← Qt Creator + 원격 개발/테스트
[참조 기기: WeiShi 1000 Standalone Timegrapher] ← 측정값 비교 기준
```

## 하드웨어 스펙

| 항목 | 스펙 |
|------|------|
| Raspberry Pi | Pi 5, 8GB RAM, 128GB microSD |
| Touchscreen | 8인치 IPS HD, 1280×800 |
| 마이크 | Weishi-style contact/vibration pickup |
| 참조 기기 | WeiShi No.1000 Standalone Timegrapher |

## 기술 스택

| 항목 | 내용 |
|------|------|
| 언어 | C++ |
| GUI 프레임워크 | Qt (Qt Creator IDE) |
| 플랫폼 | Raspberry Pi OS (Pi 5) + Windows/macOS (개발) |
| 코드베이스 | `TimeGrapher_v10.5_Student.zip` |
| AGC 설정 | **반드시 비활성화** (AlsaMixer에서 확인) |

## 동작 모드

| 모드 | 설명 |
|------|------|
| **Live** | 마이크에서 실시간 신호 캡처 |
| **Playback** | 사전 녹음된 신호 재생 (디버깅·비교) |
| **Sim** | 합성 신호 생성 (하드웨어 없이 테스트) |

## 품질 속성 (Quality Attributes)

| QA | 목표 | 측정 방법 |
|----|------|-----------|
| **Real-Time Performance** | 96,000 sps 목표 / 48,000 sps 최소 / 192,000 sps stretch | FPS + 처리 시간 측정 |
| **Low Latency** | 캡처 → 처리 → GUI 표시 end-to-end 최소화 | 3-point latency 측정 |
| **Correctness** | 표시값 ↔ 실제 음향 이벤트 일치 | WeiShi 1000 기준 비교 |
| **Measurement Accuracy** | T1 onset/peak 정밀 감지 | beat event 정확도 검증 |
| **Extensibility** | 새 그래프·필터 추가 시 기존 코드 대규모 수정 없이 가능 | Module view 기반 영향 분석 |

## 구현 대상 그래프 (Required)

| 그래프 | 설명 |
|--------|------|
| Trace Display | rate deviation + amplitude 실시간 연속 기록 (smoothing 포함) |
| Rate & Amplitude Stability (Vario) | Min/Max/Avg/σ 통계 표시, acceptable range 구분 |
| Multi-Position Sequence Display | 최대 10 포지션 측정 결과 비교 (X·D 요약값 포함) |
| Beat-Noise Scope (Scope 1 & 2) | 개별 beat noise 파형 + Σ 평균화 |
| Beat Error Display & Diagnostic Trace | rate/amplitude/beat error 수치 + trace 그래프 |
| Long-Term Performance Graph | 장시간 rate/amplitude/beat error 변화 |
| Escapement Analyzer & Marker-Line Display | A/C 이벤트 마커 + ms 레이블 |
| Time-Frequency Spectrogram | 시간-주파수 에너지 분포 (색상 강도 = 신호 세기) |
| Waveform Comparison Display | 연속 beat 파형 정렬 비교 + timing marker |
| Scope Mode (Synchronized Sweep) | oscilloscope 스타일 고정 sweep window |
| Scope Function (F0/F1/F2/F3 Filter Views) | 4가지 필터 처리 뷰 동시 제공 |

## 참고 자료

| 레퍼런스 | 용도 |
|----------|------|
| Witschi Training Course (pp.14-15) | 그래프 읽기·해석 |
| Witschi Training Course (pp.16-19) | Scope function 오류 감지 |
| Witschi Chronoscope X1 G3 Manual | 디스플레이 모드 UI 참고 |
| TimeGrapher Equations v0 | Rate/Amplitude/Beat Error 계산 공식 |
| Watch-O-Scope Manual | GUI·scope 디스플레이·필터링 참고 |
| eTimer Website | Escapement Analyzer 참고 |
