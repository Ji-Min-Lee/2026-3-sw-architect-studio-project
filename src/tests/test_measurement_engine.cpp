#include <QtTest>

#define private public
#include "MeasurementEngine.h"
#undef private

class TestMeasurementEngine : public QObject
{
    Q_OBJECT

private slots:
    void beatError_workedExample_matchesEquation()
    {
        MeasurementEngine engine;
        engine.reset();
        engine.mSamplesPerSecond = 48000;

        engine.computeBeatError(0.0, true, 28800);
        engine.computeBeatError(125.8 * 48.0, true, 28800);
        engine.computeBeatError((125.8 + 124.2) * 48.0, true, 28800);

        QCOMPARE(engine.mBeat.roll->CurrentSize(), 1);
        QVERIFY(qAbs(engine.mBeat.roll->GetAverage() - 0.8) < 1e-3);
    }

    void computeAmplitude_ticAndTocProduceSplitAndAverage()
    {
        MeasurementEngine engine;
        engine.reset();
        engine.mSamplesPerSecond = 48000;
        engine.mLiftAngle = 52.0;
        engine.mRate.bphValid = true;

        Measurement ticMeasurement;
        AcousticEvent ticEvent{};
        engine.mAmp.haveA = true;
        engine.mAmp.lastA = 0.0;
        engine.mRate.beatNumber = 1; // TIC branch
        engine.computeAmplitude(0.009 * engine.mSamplesPerSecond, true, 28800, ticMeasurement, ticEvent);

        QVERIFY(!ticEvent.hasAmpSplit);
        QVERIFY(engine.mAmp.ticValid);
        QVERIFY(engine.mAmp.ticAmp > 0.0);
        QCOMPARE(engine.mAmp.roll->CurrentSize(), 0);

        Measurement tocMeasurement;
        AcousticEvent tocEvent{};
        engine.mAmp.haveA = true;
        engine.mAmp.lastA = 0.0;
        engine.mRate.beatNumber = 2; // TOC branch
        engine.computeAmplitude(0.009 * engine.mSamplesPerSecond, true, 28800, tocMeasurement, tocEvent);

        QVERIFY(tocEvent.hasAmpSplit);
        QVERIFY(qAbs(tocEvent.ticAmpDeg - tocEvent.tocAmpDeg) < 1e-9);
        QCOMPARE(engine.mAmp.roll->CurrentSize(), 1);
        QVERIFY(qAbs(tocMeasurement.amplitudeDeg - tocEvent.ticAmpDeg) < 1e-9);
        QVERIFY(tocMeasurement.amplitudeValid);
    }

    void computeRateError_perfectWatch_setsZeroWrappedPointsAndZeroRate()
    {
        MeasurementEngine engine;
        engine.reset();
        engine.mSamplesPerSecond = 1000;
        engine.mAveragingPeriod = 2;

        AcousticEvent e1{}, e2{}, e3{}, e4{};
        engine.computeRateError(0.0, true, 3600, e1);
        engine.computeRateError(1000.0, true, 3600, e2);
        engine.computeRateError(2000.0, true, 3600, e3);
        engine.computeRateError(3000.0, true, 3600, e4);

        QVERIFY(e1.hasRatePoint);
        QVERIFY(e2.hasRatePoint);
        QVERIFY(e3.hasRatePoint);
        QVERIFY(e4.hasRatePoint);
        QVERIFY(e1.isTic);
        QVERIFY(!e2.isTic);
        QVERIFY(e3.isTic);
        QVERIFY(!e4.isTic);
        QVERIFY(qAbs(e1.wrappedRateError) < 1e-9);
        QVERIFY(qAbs(e2.wrappedRateError) < 1e-9);
        QVERIFY(qAbs(e3.wrappedRateError) < 1e-9);
        QVERIFY(qAbs(e4.wrappedRateError) < 1e-9);
        QVERIFY(engine.mRate.rateValid);
        QVERIFY(qAbs(engine.mRate.rateSpd) < 1e-9);
    }

    // ════════════════════════════════════════════════════════════════════════
    // computeRateError — Equations p.6 worked example
    // T_tic=249.980ms, T_tac=249.970ms, BPH=28800 → Rate = +8.640 s/day
    //
    // A-event stream (fs=1000, so sample index = ms):
    //   A0=0, A1=125.0, A2=249.980, A3=374.970
    //   T_tic = A2-A0 = 249.980ms ✓   T_tac = A3-A1 = 249.970ms ✓
    // ════════════════════════════════════════════════════════════════════════

    void computeRateError_knownDeviation_rateSpd_8p64()
    {
        MeasurementEngine engine;
        engine.reset();
        engine.mSamplesPerSecond = 1000;  // 1 sample = 1 ms
        engine.mAveragingPeriod  = 1;

        AcousticEvent e0{}, e1{}, e2{}, e3{};
        engine.computeRateError(0.0,     true, 28800, e0);
        engine.computeRateError(125.0,   true, 28800, e1);
        engine.computeRateError(249.980, true, 28800, e2);
        engine.computeRateError(374.970, true, 28800, e3);  // TOC: rate computed here

        QVERIFY(engine.mRate.rateValid);
        QVERIFY(qAbs(engine.mRate.rateSpd - 8.640) < 0.05);
    }

    // ════════════════════════════════════════════════════════════════════════
    // computeRateError — multi-beat averaging (M=20 beats per phase)
    // 동일 조건으로 20쌍 feed 시 수렴 정밀도가 단일 쌍보다 높아야 한다.
    // tolerance: ±0.01 s/day (single-pair는 ±0.05 허용)
    //
    // A-event 패턴 (같은 위상끼리 같은 주기):
    //   A_{2k}   = k × 249.980 ms  (tic, k=0,1,2,...)
    //   A_{2k+1} = 125.0 + k × 249.970 ms  (tac)
    // ════════════════════════════════════════════════════════════════════════

    void computeRateError_multibeat_rateConverges()
    {
        MeasurementEngine engine;
        engine.reset();
        engine.mSamplesPerSecond = 1000;
        engine.mAveragingPeriod  = 1;

        const int N = 20;  // 20 tic + 20 tac A-events
        for (int i = 0; i < N * 2; ++i) {
            double t;
            if (i % 2 == 0)
                t = (i / 2) * 249.980;           // tic: 0, 249.980, 499.960, ...
            else
                t = 125.0 + (i / 2) * 249.970;  // tac: 125.0, 374.970, 624.940, ...
            AcousticEvent ev{};
            engine.computeRateError(t, true, 28800, ev);
        }

        QVERIFY(engine.mRate.rateValid);
        QVERIFY(qAbs(engine.mRate.rateSpd - 8.640) < 0.01);
    }

    // ════════════════════════════════════════════════════════════════════════
    // computeRateError — 느린 시계 (slow watch) → rateSpd < 0
    // T_tic = T_tac = 250.010ms (T_nom=250ms보다 느림)
    // rate_tic = 86400 × (250/250.010 - 1) ≈ -3.456 s/day
    // ════════════════════════════════════════════════════════════════════════

    void computeRateError_slowWatch_rateSpd_negative()
    {
        MeasurementEngine engine;
        engine.reset();
        engine.mSamplesPerSecond = 1000;
        engine.mAveragingPeriod  = 1;

        // T_nom = 250.0ms, T_tic = T_tac = 250.010ms → rate ≈ -3.456 s/day
        const int N = 20;
        for (int i = 0; i < N * 2; ++i) {
            double t;
            if (i % 2 == 0)
                t = (i / 2) * 250.010;
            else
                t = 125.005 + (i / 2) * 250.010;
            AcousticEvent ev{};
            engine.computeRateError(t, true, 28800, ev);
        }

        QVERIFY(engine.mRate.rateValid);
        QVERIFY(engine.mRate.rateSpd < 0.0);
        // rate = 86400 × (0.250 / 0.250010 − 1) = 86400 × (−0.000040) ≈ −3.456 s/day
        QVERIFY(qAbs(engine.mRate.rateSpd - (-3.456)) < 0.05);
    }
};

QTEST_APPLESS_MAIN(TestMeasurementEngine)
#include "test_measurement_engine.moc"
