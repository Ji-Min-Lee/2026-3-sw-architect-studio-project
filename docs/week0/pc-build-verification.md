# PC 빌드 검증 기록

**날짜**: 2026-05-28  
**작업자**: Jimin Lee  
**대상**: TimeGrapher_v10.5_Student

---

## 환경

| 항목 | 버전 |
|------|------|
| OS | macOS 14.6.1 (Sonoma, Build 23G93) |
| 컴파일러 | Apple clang 15.0.0 (clang-1500.3.9.4) |
| Qt | 6.11.1 (macOS, `~/Qt/6.11.1/macos`) |
| CMake | 3.30.5 (Qt 번들, `~/Qt/Tools/CMake`) |
| Qt Creator | `~/Qt/Qt Creator.app` (설치 확인) |

---

## 절차

### 1. 소스 압축 해제

```bash
unzip TimeGrapher_v10.5_Student.zip -d ~/Desktop/2026_architect/TimeGrapher
```

결과: `TimeGrapher/TimeGrapher/` 하위에 소스 파일 일체 압축 해제 완료

### 2. CMake 구성

```bash
mkdir -p ~/Desktop/2026_architect/TimeGrapher/build
cd ~/Desktop/2026_architect/TimeGrapher/build
~/Qt/6.11.1/macos/bin/qt-cmake ../TimeGrapher -DCMAKE_BUILD_TYPE=Release
```

결과:
```
-- The CXX compiler identification is AppleClang 15.0.0.15000309
-- Found Threads: TRUE
-- Found WrapOpenGL: TRUE
-- Found Cups: libcups 2.3.4
-- Configuring done (4.3s)
-- Generating done (1.8s)
-- Build files have been written to: .../TimeGrapher/build
```

### 3. 빌드 (Release)

```bash
cmake --build . --config Release -j$(sysctl -n hw.ncpu)
```

결과:
```
[100%] Built target TimeGrapher
```

- 에러: **없음**
- 경고: `qcustomplot.cpp`의 Qt 6.9/6.13 deprecated API 6건 (서드파티, 무시 가능)
  - `toTimeSpec` → `toTimeZone` 권고
  - `startOfDay` → QTimeZone 파라미터 권고
  - `mirrored` → `flipped(Qt::Orientations)` 권고

### 4. 실행

```bash
open ~/Desktop/2026_architect/TimeGrapher/build/TimeGrapher.app
```

결과: **TimeGrapher GUI 정상 실행 확인**

![TimeGrapher macOS 실행 화면](macos_build.png)

> - Gain: MacBook Pro 마이크 (macOS CoreAudio 자동 인식)
> - Sample Rate: 48000 Hz, Mode: Live
> - Rate/Scope, Sound Print 탭 정상 표시
> - Rate Error 그래프 및 Amplitude 그래프 렌더링 확인

---

## 산출물

| 파일 | 크기 | 경로 |
|------|------|------|
| TimeGrapher (바이너리) | 4.1 MB | `build/TimeGrapher.app/Contents/MacOS/TimeGrapher` |
| TimeGrapher.app (번들) | — | `build/TimeGrapher.app/` |

---

## macOS 특이사항

- `LinuxAudio.cpp`는 `#if defined(Q_OS_LINUX)` 가드로 감싸져 있어 macOS에서 컴파일 시 무해하게 스킵됨
- macOS는 `libasound2-dev` 불필요 (CoreAudio 사용, Qt Multimedia가 자동 처리)
- Windows 전용 링크 라이브러리(`winmm`, `Ole32`, `Uuid`, `Propsys`)도 조건부 컴파일로 스킵됨

---

## 결론

`TimeGrapher_v10.5_Student` macOS 빌드 및 실행 **성공** ✓
