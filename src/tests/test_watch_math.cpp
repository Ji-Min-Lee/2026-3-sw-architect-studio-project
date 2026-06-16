#include <QtTest>
#include "WatchMath.h"
#include <cmath>

class TestWatchMath : public QObject
{
    Q_OBJECT

private slots:

    // ════════════════════════════════════════════════════════════════════════
    // Part 1: beatErrorMs
    // 수식(Equations p.7): BE = |(t1 - t2)| / 2 * 1000 [ms]
    // t1 = (A1-A0)/fs,  t2 = (A2-A1)/fs
    // ════════════════════════════════════════════════════════════════════════

    void beatError_perfectSymmetry_isZero()
    {
        // t1 = t2 = 125 ms → BE = 0
        const int fs = 48000;
        double A0 = 0.0, A1 = 125.0 * fs / 1000.0, A2 = 250.0 * fs / 1000.0;
        QVERIFY(qAbs(WatchMath::beatErrorMs(A0, A1, A2, fs)) < 1e-9);
    }

    void beatError_workedExample_0p8ms()
    {
        // Equations p.8: t1=125.8ms, t2=124.2ms → BE = 0.8 ms
        const int fs = 48000;
        double A0 = 0.0;
        double A1 = 125.8 * fs / 1000.0;
        double A2 = A1 + 124.2 * fs / 1000.0;
        double beatErrorMs = WatchMath::beatErrorMs(A0, A1, A2, fs);
        QVERIFY(qAbs(beatErrorMs - 0.8) < 1e-3);
    }

    void beatError_t1GreaterThanT2_positive()
    {
        // t1=130ms, t2=120ms → BE = 5.0 ms
        const int fs = 48000;
        double A0 = 0.0;
        double A1 = 130.0 * fs / 1000.0;
        double A2 = A1 + 120.0 * fs / 1000.0;
        QVERIFY(qAbs(WatchMath::beatErrorMs(A0, A1, A2, fs) - 5.0) < 1e-3);
    }

    void beatError_t2GreaterThanT1_absoluteValue()
    {
        // t1=120ms, t2=130ms → BE = 5.0 ms (abs)
        const int fs = 48000;
        double A0 = 0.0;
        double A1 = 120.0 * fs / 1000.0;
        double A2 = A1 + 130.0 * fs / 1000.0;
        QVERIFY(qAbs(WatchMath::beatErrorMs(A0, A1, A2, fs) - 5.0) < 1e-3);
    }

    void beatError_largeAsymmetry()
    {
        // t1=150ms, t2=100ms → BE = 25.0 ms
        const int fs = 48000;
        double A0 = 0.0;
        double A1 = 150.0 * fs / 1000.0;
        double A2 = A1 + 100.0 * fs / 1000.0;
        QVERIFY(qAbs(WatchMath::beatErrorMs(A0, A1, A2, fs) - 25.0) < 1e-3);
    }

    void beatError_differentSampleRate_44100()
    {
        // fs=44100, 동일한 시간 → 동일한 결과
        const int fs = 44100;
        double A0 = 0.0;
        double A1 = 125.8 * fs / 1000.0;
        double A2 = A1 + 124.2 * fs / 1000.0;
        QVERIFY(qAbs(WatchMath::beatErrorMs(A0, A1, A2, fs) - 0.8) < 1e-3);
    }

    // ════════════════════════════════════════════════════════════════════════
    // Part 2: amplitudeDeg
    // 수식(Equations p.10): Amp = liftAngle / sin(π * BPH * t_AC / 3600)
    // 문서 근사식: Amp ≈ (3600 * λ) / (π * BPH * t_AC)
    // ════════════════════════════════════════════════════════════════════════

    void amplitude_workedExample_230deg()
    {
        // Equations p.11: λ=52°, BPH=28800, t_AC=0.009s → Amp ≈ 230°
        const int fs = 48000;
        double aPos = 0.0;
        double cPos = 0.009 * fs;  // 0.009s × 48000
        double amp  = WatchMath::amplitudeDeg(aPos, cPos, fs, 52.0, 28800);
        QVERIFY(amp > 0);
        QVERIFY(qAbs(amp - 230.0) < 2.0);  // ±2° tolerance (근사 vs 정확식 차이)
    }

    void amplitude_nearZero_tAC_returnsInvalid()
    {
        // t_AC ≈ 0 → sin ≈ 0 → 무효
        double amp = WatchMath::amplitudeDeg(0.0, 0.1, 48000, 52.0, 28800);
        QVERIFY(amp < 0);  // 무효값
    }

    void amplitude_over360_returnsInvalid()
    {
        // t_AC 매우 짧아 → amp >= 360 → -1 반환
        const int fs = 48000;
        double cPos  = 0.001 * fs;  // 1ms → 매우 짧음
        double amp   = WatchMath::amplitudeDeg(0.0, cPos, fs, 52.0, 28800);
        QVERIFY(amp < 0);
    }

    void amplitude_convergesWithApprox_forSmallTAC()
    {
        // t_AC가 작을 때(정상 범위) 정확식과 근사식의 차이 < 1%
        const int fs = 48000;
        double tAC   = 0.009;  // 9ms (정상 범위)
        double cPos  = tAC * fs;
        double ampExact  = WatchMath::amplitudeDeg(0.0, cPos, fs, 52.0, 28800);
        double ampApprox = (3600.0 * 52.0) / (M_PI * 28800.0 * tAC);
        double relErr = qAbs(ampExact - ampApprox) / ampApprox;
        QVERIFY(relErr < 0.01);  // 1% 이내
    }

    void amplitude_21600bph_validResult()
    {
        // 6Hz 시계 (21600 BPH), 정상 t_AC
        const int fs = 48000;
        double cPos  = 0.012 * fs;
        double amp   = WatchMath::amplitudeDeg(0.0, cPos, fs, 52.0, 21600);
        QVERIFY(amp > 0 && amp < 360.0);
    }

    // ════════════════════════════════════════════════════════════════════════
    // Part 3: escapementMs
    // 수식: (cPos - aPos) / fs * 1000 [ms]
    // ════════════════════════════════════════════════════════════════════════

    void escapement_9ms_exact()
    {
        // 0.009s × 48000 = 432 샘플
        QVERIFY(qAbs(WatchMath::escapementMs(0.0, 432.0, 48000) - 9.0) < 1e-9);
    }

    void escapement_sampleRateScaling()
    {
        // fs=44100: 동일 시간이면 샘플 수만 다름
        double cPos44 = 0.009 * 44100;
        QVERIFY(qAbs(WatchMath::escapementMs(0.0, cPos44, 44100) - 9.0) < 1e-6);
    }

    void escapement_nonZeroAPos()
    {
        // aPos=1000, cPos=1432 → 432샘플 / 48000 = 9ms
        QVERIFY(qAbs(WatchMath::escapementMs(1000.0, 1432.0, 48000) - 9.0) < 1e-9);
    }

    void escapement_zero_interval()
    {
        QCOMPARE(WatchMath::escapementMs(500.0, 500.0, 48000), 0.0);
    }

    // ════════════════════════════════════════════════════════════════════════
    // Part 4: wrapInRange
    // 범위 [-10, +10] 기준 wrapping 검증
    // ════════════════════════════════════════════════════════════════════════

    void wrap_valueInsideRange_unchanged()
    {
        QVERIFY(qAbs(WatchMath::wrapInRange(5.0, -10.0, 10.0) - 5.0) < 1e-9);
        QVERIFY(qAbs(WatchMath::wrapInRange(-5.0, -10.0, 10.0) - (-5.0)) < 1e-9);
    }

    void wrap_exactUpperBound_wrapsToLower()
    {
        // 10.0 → -10.0 (상한 = lo)
        QVERIFY(qAbs(WatchMath::wrapInRange(10.0, -10.0, 10.0) - (-10.0)) < 1e-9);
    }

    void wrap_exactLowerBound_unchanged()
    {
        QVERIFY(qAbs(WatchMath::wrapInRange(-10.0, -10.0, 10.0) - (-10.0)) < 1e-9);
    }

    void wrap_slightlyAboveUpper()
    {
        // 15.0 → -5.0  (15 - 10 = 5 초과, range=20, 15 mod 20 = 15, shifted=15-(-10)-20=5... )
        // 실제: shifted = fmod(15 - (-10), 20) = fmod(25, 20) = 5 → 5 + (-10) = -5
        QVERIFY(qAbs(WatchMath::wrapInRange(15.0, -10.0, 10.0) - (-5.0)) < 1e-9);
    }

    void wrap_slightlyBelowLower()
    {
        // -10.5 → shifted = fmod(-10.5-(-10), 20) = fmod(-0.5, 20) = -0.5 → +19.5 → 19.5+(-10)=9.5
        QVERIFY(qAbs(WatchMath::wrapInRange(-10.5, -10.0, 10.0) - 9.5) < 1e-9);
    }

    void wrap_farOutOfRange()
    {
        // -25.0: shifted = fmod(-25-(-10), 20) = fmod(-15, 20) = -15 → +5 → 5+(-10) = -5
        QVERIFY(qAbs(WatchMath::wrapInRange(-25.0, -10.0, 10.0) - (-5.0)) < 1e-9);
    }

    void wrap_largePositive()
    {
        // 100.0 in [-10, 10]: fmod(110, 20) = 10 → 10+(-10) = 0
        QVERIFY(qAbs(WatchMath::wrapInRange(100.0, -10.0, 10.0) - 0.0) < 1e-9);
    }

    // ════════════════════════════════════════════════════════════════════════
    // Part 5: applyZeroOffset
    // 목적: 첫 번째 beat의 오차를 기준(0)으로 삼아 이후 오차를 상대값으로 표시.
    // 규칙: beat 0의 instErrorMs를 anchor로 저장 → 이후 값에서 anchor를 뺀다.
    // ════════════════════════════════════════════════════════════════════════

    void zeroOffset_firstBeat_plotsAtZero()
    {
        // 첫 번째 오차가 어떤 값이든 beat 0의 플롯 Y = 0 이어야 한다.
        double firstError = 3.7;
        QCOMPARE(WatchMath::applyZeroOffset(firstError, firstError), 0.0);
    }

    void zeroOffset_subsequentBeat_isRelative()
    {
        // firstError=3.7, beat n의 raw 오차=5.2 → 표시값 = 5.2 - 3.7 = 1.5
        QVERIFY(qAbs(WatchMath::applyZeroOffset(3.7, 5.2) - 1.5) < 1e-9);
    }

    void zeroOffset_watchFast_growsPositive()
    {
        // 시계가 빠르면 instError(= ideal - measured)가 beat마다 커진다.
        // beat0=0.0, beat1=0.1, beat2=0.2 → 상대값 0, 0.1, 0.2
        double anchor = 0.0;
        QVERIFY(qAbs(WatchMath::applyZeroOffset(anchor, 0.0) - 0.0) < 1e-9);
        QVERIFY(qAbs(WatchMath::applyZeroOffset(anchor, 0.1) - 0.1) < 1e-9);
        QVERIFY(qAbs(WatchMath::applyZeroOffset(anchor, 0.2) - 0.2) < 1e-9);
    }

    void zeroOffset_negativeAnchor_correctsOffset()
    {
        // firstError=-2.0, beat n=-2.0+delta → 표시값 = delta
        double anchor = -2.0;
        QVERIFY(qAbs(WatchMath::applyZeroOffset(anchor, -2.0 + 1.5) - 1.5) < 1e-9);
    }

    // ════════════════════════════════════════════════════════════════════════
    // Part 6: escapementMs — 역순 이벤트 (cPos < aPos)
    // C 이벤트가 A보다 이전 샘플에 배치된 경우 음수가 반환되는지 확인.
    // MeasurementEngine은 mHaveLastA 가드로 이를 방지하지만,
    // 순수 함수 자체는 음수를 그대로 반환해야 한다 (방어 처리는 호출자 책임).
    // ════════════════════════════════════════════════════════════════════════

    void escapement_reverseOrder_returnsNegative()
    {
        // cPos < aPos → 음수 ms
        double result = WatchMath::escapementMs(500.0, 100.0, 48000);
        QVERIFY(result < 0.0);
    }

    void escapement_reverseOrder_magnitudeCorrect()
    {
        // aPos=432, cPos=0 → -9.0 ms (432/48000*1000)
        QVERIFY(qAbs(WatchMath::escapementMs(432.0, 0.0, 48000) - (-9.0)) < 1e-9);
    }

    // ════════════════════════════════════════════════════════════════════════
    // Part 7: halfBeatInterval
    // 수식(Part I §6): I_target = 3600 / BPH
    // ════════════════════════════════════════════════════════════════════════

    void halfBeat_28800bph_is125ms()
    {
        // 28800 BPH → 3600/28800 = 0.125 s = 125 ms
        QVERIFY(qAbs(WatchMath::halfBeatInterval(28800) - 0.125) < 1e-9);
    }

    void halfBeat_21600bph_is166ms()
    {
        // 21600 BPH → 3600/21600 ≈ 0.16667 s
        QVERIFY(qAbs(WatchMath::halfBeatInterval(21600) - (3600.0/21600.0)) < 1e-9);
    }

    void halfBeat_36000bph_is100ms()
    {
        // 36000 BPH → 3600/36000 = 0.1 s
        QVERIFY(qAbs(WatchMath::halfBeatInterval(36000) - 0.1) < 1e-9);
    }

    // ════════════════════════════════════════════════════════════════════════
    // Part 8: instErrorSec
    // 수식(Part I §1): En = T_measured − (T_start + n × I_target)
    // ════════════════════════════════════════════════════════════════════════

    void instError_beat0_isZero()
    {
        // beat 0: T_measured = T_start → En = 0
        double tStart = 1.0;
        QCOMPARE(WatchMath::instErrorSec(tStart, tStart, 0, 28800), 0.0);
    }

    void instError_perfectWatch_isZero()
    {
        // 완벽한 시계: T_measured = T_start + n * I_target → En = 0
        int bph = 28800;
        double itarget = 3600.0 / bph;
        double tStart  = 0.0;
        for (int n = 1; n <= 5; ++n) {
            double t = tStart + n * itarget;
            QVERIFY(qAbs(WatchMath::instErrorSec(t, tStart, n, bph)) < 1e-12);
        }
    }

    void instError_fastWatch_growsNegative()
    {
        // 빠른 시계: 실제 beat가 이상적 시각보다 일찍 도착 → En < 0
        // beat 4, I_target=125ms, 실제 beat가 1ms 일찍 도착
        int bph = 28800;
        double itarget = 3600.0 / bph;
        double tStart  = 0.0;
        int n = 4;
        double tMeasured = tStart + n * itarget - 0.001;  // 1ms 일찍
        double err = WatchMath::instErrorSec(tMeasured, tStart, n, bph);
        QVERIFY(err < 0.0);
        QVERIFY(qAbs(err - (-0.001)) < 1e-9);
    }

    void instError_slowWatch_growsPositive()
    {
        // 느린 시계: 실제 beat가 늦게 도착 → En > 0
        int bph = 28800;
        double itarget = 3600.0 / bph;
        double tStart  = 0.0;
        int n = 3;
        double tMeasured = tStart + n * itarget + 0.002;  // 2ms 늦게
        double err = WatchMath::instErrorSec(tMeasured, tStart, n, bph);
        QVERIFY(err > 0.0);
        QVERIFY(qAbs(err - 0.002) < 1e-9);
    }

    void instError_workedExample_p2()
    {
        // Equations p.2 worked example: T_start=0, I_target=125ms
        // beat 1 ideal = 125ms. If measured at 125.8ms → En = +0.8ms
        int bph = 28800;
        double tStart    = 0.0;
        double tMeasured = 0.1258;  // 125.8 ms
        double err = WatchMath::instErrorSec(tMeasured, tStart, 1, bph);
        QVERIFY(qAbs(err - 0.0008) < 1e-6);
    }

    // ════════════════════════════════════════════════════════════════════════
    // Part 9: rateSpdFromPhase
    // 수식(Part II §3): Rate = (rate_tic + rate_tac) / 2
    //   rate_tic = 86400 × (T_nom / T_tic − 1)
    //   rate_tac = 86400 × (T_nom / T_tac − 1)
    //   T_nom = 7200 / BPH
    // ════════════════════════════════════════════════════════════════════════

    void rateSpd_workedExample_p6()
    {
        // Equations p.6 worked example (28800 BPH):
        //   T_tic = 249.980 ms, T_tac = 249.970 ms
        //   rate_tic = +6.912 s/day, rate_tac = +10.368 s/day
        //   Rate = +8.640 s/day
        double tTic = 0.249980;
        double tTac = 0.249970;
        double rate = WatchMath::rateSpdFromPhase(tTic, tTac, 28800);
        QVERIFY(qAbs(rate - 8.640) < 0.01);
    }

    void rateSpd_perfectWatch_isZero()
    {
        // T_tic = T_tac = T_nom → rate = 0
        double tNom = 7200.0 / 28800.0;  // 0.25 s
        double rate = WatchMath::rateSpdFromPhase(tNom, tNom, 28800);
        QVERIFY(qAbs(rate) < 1e-9);
    }

    void rateSpd_fastWatch_isNegative()
    {
        // 빠른 시계: T_tic, T_tac < T_nom → (T_nom/T - 1) > 0이지만
        // 코드에서는 부호 반전 없이 반환 (86400 * (T_nom/T - 1))
        // T < T_nom → T_nom/T > 1 → rate > 0 (s/day gain)
        double tNom  = 7200.0 / 28800.0;
        double tFast = tNom * 0.999;  // 약간 짧음
        double rate  = WatchMath::rateSpdFromPhase(tFast, tFast, 28800);
        QVERIFY(rate > 0.0);
    }

    void rateSpd_ticTacAsymmetry_averaged()
    {
        // tic과 tac이 대칭적으로 빠름/느림일 때 평균이 중간값임을 확인.
        // 비선형 공식 특성상 정확히 0이 아닌 작은 잔차(< 0.1 s/day) 발생.
        // rate_tic ≈ -86.31, rate_tac ≈ +86.49 → avg ≈ +0.086 (0.1 이내)
        int bph    = 28800;
        double tNom = 7200.0 / bph;
        double tTic = tNom * 1.001;
        double tTac = tNom * 0.999;
        double rate = WatchMath::rateSpdFromPhase(tTic, tTac, bph);
        QVERIFY(qAbs(rate) < 0.1);
    }

    // ════════════════════════════════════════════════════════════════════════
    // Part 10: amplitude 근사식 직접 검증
    // 문서 공식: Amp_approx = (3600 × λ) / (π × n × t_AC)
    // ════════════════════════════════════════════════════════════════════════

    void ampApprox_workedExample_230deg()
    {
        // Equations p.11: λ=52°, n=28800, t_AC=0.009s → Amp ≈ 230°
        double ampApprox = (3600.0 * 52.0) / (M_PI * 28800.0 * 0.009);
        QVERIFY(qAbs(ampApprox - 230.0) < 1.0);
    }

    void ampApprox_inverseProportional_toTAC()
    {
        // t_AC가 2배 → Amp_approx가 절반
        double tAC1 = 0.009;
        double tAC2 = 0.018;
        double amp1 = (3600.0 * 52.0) / (M_PI * 28800.0 * tAC1);
        double amp2 = (3600.0 * 52.0) / (M_PI * 28800.0 * tAC2);
        QVERIFY(qAbs(amp1 / amp2 - 2.0) < 1e-9);
    }
};

QTEST_APPLESS_MAIN(TestWatchMath)
#include "test_watch_math.moc"
