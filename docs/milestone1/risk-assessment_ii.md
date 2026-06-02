# Risk Assessment — TimeGrapher Project

**Milestone**: M1 | **Due**: 2026-06-09 | **Status**: [ ] Draft / [ ] Final

> Probability: H=High / M=Medium / L=Low  
> Impact: H=High / M=Medium / L=Low

---

## 1. Technical Risks

| ID | Risk | Probability | Impact | Score | Rationale |
|----|------|:-----------:|:------:|:-----:|-----------|
| T1 | **Raspberry Pi 5가 96k sps 실시간 처리를 달성하지 못함** | H | H | 🔴 Critical | RPi 5는 embedded ARM 플랫폼. 96k sps 오디오 캡처 + HPF/Envelope DSP + 이벤트 검출 + Qt GUI 렌더링을 동시에 수행할 때 CPU 과부하 가능성이 실험 전까지 미확인 상태 |
| T2 | **Qt 실시간 렌더링이 병목이 됨** | M | H | 🟠 High | QCustomPlot 기반 scatter/line 그래프를 11개 동시 업데이트하면 GUI 스레드가 블로킹될 수 있음. 특히 RPi의 GPU 가속 제한 환경에서 FPS 저하 위험 |
| T3 | **T1/T3 이벤트 검출 정확도가 WeiShi 1000 대비 허용 오차 초과** | M | H | 🟠 High | Silence→Burst 기반 검출 알고리즘은 환경 노이즈, 시계 종류, 진동 특성에 민감. 정확도 검증 전까지 Rate/Amplitude 오차 margin 미확인 |
| T4 | **AGC(Auto Gain Control) 미비활성화로 신호 왜곡 발생** | M | H | 🟠 High | AlsaMixer에서 AGC 비활성화가 아직 미확인. AGC가 활성 상태면 마이크 입력이 자동 조정되어 Beat event 타이밍이 변형됨 → 모든 측정값 신뢰 불가 |
| T5 | **Spectrogram(FFT) 구현 불가** | M | M | 🟡 Medium | 베이스코드에서 FFTW3가 주석 처리되어 있고 Raw PCM→FFT 경로 미구현. 11개 필수 그래프 중 난이도 최상위 항목 (~8h 예상) |
| T6 | **PC ↔ Raspberry Pi 환경 불일치** | M | M | 🟡 Medium | Qt 버전, ALSA 드라이버, 샘플레이트 지원 범위가 PC(macOS/Windows)와 RPi OS 간 다를 수 있음. PC에서 통과한 빌드가 RPi에서 실패 가능 |
| T7 | **per-beat PCM 링 버퍼 미구현 (Waveform Comparison, Beat-Noise Scope)** | L | M | 🟡 Medium | 베이스코드에 beat 슬라이싱 없음. Waveform Comparison Display와 Beat-Noise Scope Σ 평균 기능은 멀티 beat PCM 히스토리 버퍼를 신규 설계해야 함 |
| T8 | **베이스코드 아키텍처 확장성 부족** | L | M | 🟡 Medium | 현재 MainWindow 단일 클래스에 비즈니스 로직이 집중됨. 11개 그래프 추가 시 기존 코드 변경 범위가 넓어지면 Extensibility QA 달성 어려움 |
| T9 | **장시간 운용 시 데이터 누적에 따른 메모리 증가** | M | M | 🟡 Medium | Rate/Amplitude/Beat Error 측정값, PCM 샘플, 그래프 포인트가 시간에 비례해 누적됨. Long-Term Performance Graph 및 Waveform Comparison은 특히 히스토리 버퍼를 무한 보존할 경우 RPi 8GB RAM에서도 장시간 운용 시 OOM(Out of Memory) 또는 성능 저하 위험 |
| T10 | **Pause 후 Resume 시 데이터 손실 또는 왜곡 발생** | M | M | 🟡 Medium | Pause 중에도 오디오 캡처 스레드가 계속 실행되면 버퍼 오버플로우로 데이터 유실 가능. 반대로 캡처를 중단하면 Resume 시점에 불연속 구간이 생겨 Beat Error·Rate 계산값이 일시적으로 왜곡될 수 있음. 특히 BPH PLL 동기 상태가 리셋되면 재동기화까지 수초간 측정 불신뢰 구간 발생 |

---

## 2. Non-Technical Risks

| ID | Risk | Probability | Impact | Score | Rationale |
|----|------|:-----------:|:------:|:-----:|-----------|
| N1 | **5주 일정 내 11개 그래프 + Enhanced Features 완성 불가** | H | H | 🔴 Critical | 7명 × 2h/day = 70h/week. 그래프 구현 추정치 합산 ~70h + Enhanced ~18h + 문서 ~40h + RPi 검증 ~15h = 143h. 일정 압박이 매우 높음 |
| N2 | **팀원 간 코드베이스 이해 수준 불균형** | M | M | 🟡 Medium | C++/Qt 숙련도 차이가 존재할 경우, 코드 리뷰·통합 단계에서 병목 발생. 특히 DSP 파이프라인(HPF, Envelope, Detector)은 도메인 지식 필요 |
| N3 | **Grading Rubric 미수령** | M | M | 🟡 Medium | 타임그래퍼 전용 채점 기준이 Week 2~3에 배포 예정. 수령 전까지 구현 우선순위 판단 기준 불명확 |
| N4 | **하드웨어 장애 (RPi, 센서 스탠드, 시계)** | L | H | 🟠 High | RPi 또는 USB 센서 스탠드 고장 시 실물 검증 전면 중단. 대체 장비 없음 |
| N5 | **도메인 지식 부족 (시계 공학)** | M | L | 🟢 Low | Escapement 동작 원리 이해 없이 Rate/Amplitude/Beat Error 측정값 해석 오류 가능성. 단, 문서(Witschi Training Course)로 보완 가능 |
| N6 | **팀 내 영어 커뮤니케이션 부담** | M | M | 🟡 Medium | 멘토 리뷰, 문서 작성, 팀 회의가 영어로 진행됨. 영어 숙련도 차이로 인해 기술적 의견 전달이 누락되거나 문서 품질이 저하될 위험. 특히 아키텍처 결정 사항을 정확히 표현하지 못하면 멘토 피드백 반영이 지연될 수 있음 |

---

## 3. Open Questions / Issues

> 아래 항목들은 모두 프로젝트 최종 결과에 직접 영향을 미치는 미결 사항입니다.

| # | Open Question / Issue | 관련 Risk | 결과 영향 |
|---|----------------------|-----------|-----------|
| OQ1 | **RPi 5에서 96k sps 실시간 처리가 실제로 가능한가?** | T1 | Real-Time Performance QA 달성 여부 결정 |
| OQ2 | **Qt GUI 렌더링이 RPi에서 목표 FPS를 유지하는가?** | T2 | Low Latency QA 달성 여부 결정 |
| OQ3 | **T1/T3 검출 오차가 WeiShi 1000 대비 허용 범위 이내인가?** | T3 | Measurement Accuracy QA 달성 여부 결정 |
| OQ4 | **AGC가 실제로 비활성화되어 있는가?** | T4 | 모든 측정값의 신뢰성 전제 조건 |
| OQ5 | **11개 그래프를 5주 일정 내 완성 가능한가?** | N1 | 최종 데모 completeness에 직결 |
| OQ6 | **새 그래프 추가 시 변경되는 파일 수가 목표치 이내인가?** | T8 | Extensibility QA 측정 가능 여부 |

---

## 4. Mitigation Actions

| Risk ID | Action | 완료 기준 | 기한 |
|---------|--------|-----------|------|
| T1, OQ1 | **Experiment 1 수행**: RPi에서 96k / 48k / 192k sps 처리 시간 실측 측정 | sps별 처리 시간 수치 확보 | 06/11 |
| T2, OQ2 | **Experiment 2 수행**: Qt 그래프 업데이트 FPS vs CPU 사용률 측정. Double-buffering 또는 별도 렌더링 스레드 적용 여부 판단 | 렌더링 병목 여부 판정 + 허용 FPS 범위 확정 | 06/11 |
| T3, OQ3 | **Experiment 3 수행**: 동일 시계에서 타임그래퍼 vs WeiShi 1000 Rate/Amplitude 비교. 허용 오차 기준(±5 s/d) 충족 여부 확인 | 오차 margin 수치 확보 | 06/12 |
| T4, OQ4 | **AGC 비활성화 즉시 확인**: AlsaMixer에서 AGC 항목 OFF 설정 후 스크린샷으로 기록 | AlsaMixer 확인 완료 체크 | 06/09 (즉시) |
| T5 | Spectrogram 구현 시 FFTW3 대신 Qt 또는 kiss_fft 경량 라이브러리 검토. 일정 압박 시 스펙트로그램 구현 순위를 11개 중 후순위로 조정 | 라이브러리 선택 결정 | 06/15 |
| T6 | PC에서 빌드 성공 즉시 RPi에서 병렬 빌드 검증. "PC 완료 → RPi 빌드" 루틴 유지 | 각 그래프 완료마다 RPi 빌드 확인 | Rolling |
| T7 | Beat-Noise Scope 구현 시 ring buffer 설계를 Waveform Comparison과 공유하도록 모듈 설계 선행 | 공유 ring buffer 인터페이스 정의 | 06/16 |
| T8, OQ6 | 아키텍처 설계 시 그래프 추가를 위한 Plugin/Observer 패턴 적용. Extensibility QA: "새 그래프 추가 시 변경 파일 ≤ 3개" 목표 설정 | Module View에 extensibility 포인트 명시 | 06/18 |
| T9 | 그래프별 히스토리 버퍼에 **최대 보존 개수(cap)** 설정 — Ring buffer 또는 sliding window 방식으로 오래된 데이터 자동 폐기. Long-Term Graph는 다운샘플링(1분 평균 등) 적용 검토 | 버퍼 cap 정책 정의 + 장시간 운용 메모리 사용량 측정 | 06/18 |
| T10 | Pause 시 오디오 캡처 스레드 처리 정책 명확화 — Pause 중 캡처 지속 시 버퍼 드레인 처리, 중단 시 Resume 후 BPH PLL 재동기화 대기 구간을 UI에 표시. Resume 직후 일정 구간(예: 3 beat) 측정값을 신뢰 불가 상태로 마킹하여 그래프에 반영 | Pause/Resume 시나리오 테스트 통과 + 불연속 구간 마킹 확인 | 06/22 |
| N1, OQ5 | 11개 그래프를 Priority로 분류: **Core(1~4)** → **Required(5~8)** → **Stretch(9~11)**. 일정 지연 시 Stretch는 데모에서 "설계만 완료"로 대응 | 우선순위 목록 확정 | 06/09 |
| N3 | 채점 기준 수령 전까지 Project Plan p.25-26의 QA 정의를 기준으로 구현 우선순위 유지. 수령 즉시 계획 재검토 | Rubric 수령 후 1일 내 계획 업데이트 | 수령 즉시 |
| N6 | 회의 전 핵심 안건을 한국어로 정리 후 영어 요약본 준비. 문서 초안은 한국어로 작성 후 영어로 번역하는 2-pass 방식 적용. 멘토 리뷰 전 영어 표현 교차 검토 | 팀 내 영어 커뮤니케이션 가이드 합의 | 06/09 |
| N4 | RPi 고장에 대비해 주요 WAV 녹음 파일을 PC에 백업. Sim 모드 활용으로 하드웨어 없이도 일부 개발 계속 가능하도록 유지 | Sim 모드 정상 동작 확인 | 06/09 |

---

## 5. Risk Summary Matrix

```
Impact
  H │  T4        T1        N4
    │  T3        T2
  M │  N2  T5   T7  T8   N3  T6
    │  N5
  L │            N1 (prob H)
    └─────────────────────────────
         L        M        H    Probability
```

> 🔴 **즉시 조치 필요**: T1(RPi sps), N1(일정), T4(AGC)  
> 🟠 **실험으로 조기 검증 필요**: T2(Qt 렌더링), T3(검출 정확도), N4(하드웨어 장애)  
> 🟡 **아키텍처 설계 시 반영**: T5(FFT), T6(환경 불일치), T7(ring buffer), T8(확장성)

---

## 6. Review Checklist

- [ ] Technical and non-technical risks distinguished
- [ ] Probability and impact assessed (H/M/L)
- [ ] Actions defined for each risk
- [ ] Open issues listed with owners and target resolution dates
