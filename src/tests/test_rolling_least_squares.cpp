#include <QtTest>
#include "RollingLeastSquares.h"

class TestRollingLeastSquares : public QObject
{
    Q_OBJECT

private slots:

    // ── 기본 slope 계산 ────────────────────────────────────────────────────────

    void twoPoints_perfectLine_exactSlope()
    {
        // y = 3x → slope = 3
        RollingLeastSquares rls(10);
        rls.AddPoint(0.0, 0.0);
        rls.AddPoint(1.0, 3.0);
        double slope;
        QVERIFY(rls.GetRate(slope));
        QVERIFY(qAbs(slope - 3.0) < 1e-9);
    }

    void fivePoints_perfectLine_exactSlope()
    {
        // y = 2x + 1 → slope = 2
        RollingLeastSquares rls(10);
        for (int i = 0; i < 5; i++) rls.AddPoint(i, 2.0 * i + 1.0);
        double slope;
        QVERIFY(rls.GetRate(slope));
        QVERIFY(qAbs(slope - 2.0) < 1e-9);
    }

    void negativeSlope_detectedCorrectly()
    {
        // y = -5x → slope = -5
        RollingLeastSquares rls(10);
        for (int i = 0; i < 4; i++) rls.AddPoint(i, -5.0 * i);
        double slope;
        QVERIFY(rls.GetRate(slope));
        QVERIFY(qAbs(slope - (-5.0)) < 1e-9);
    }

    // ── 롤링 윈도우 ────────────────────────────────────────────────────────────

    void rollingWindow_dropsOldestPoint()
    {
        // capacity=3. 점 (0,0),(1,1),(2,2) 추가 후 slope=1.
        // (3,10) 추가 → window: (1,1),(2,2),(3,10) → slope 변화해야 함
        RollingLeastSquares rls(3);
        rls.AddPoint(0.0, 0.0);
        rls.AddPoint(1.0, 1.0);
        rls.AddPoint(2.0, 2.0);
        double slope1;
        rls.GetRate(slope1);
        QVERIFY(qAbs(slope1 - 1.0) < 1e-9);

        rls.AddPoint(3.0, 10.0);  // (0,0) 제거됨
        double slope2;
        rls.GetRate(slope2);
        // 새 slope는 1이 아니어야 함
        QVERIFY(qAbs(slope2 - 1.0) > 0.1);
    }

    void window_sizeCapped_atCapacity()
    {
        // capacity=3, 점 5개 추가 → 최근 3개만 반영
        RollingLeastSquares rls(3);
        // 처음 2개: (0,0),(1,100) → slope≈100
        // 이후 3개: (2,2),(3,3),(4,4) → window=(2,2),(3,3),(4,4) → slope=1
        rls.AddPoint(0.0, 0.0);
        rls.AddPoint(1.0, 100.0);
        rls.AddPoint(2.0, 2.0);
        rls.AddPoint(3.0, 3.0);
        rls.AddPoint(4.0, 4.0);
        double slope;
        QVERIFY(rls.GetRate(slope));
        QVERIFY(qAbs(slope - 1.0) < 1e-9);
    }

    // ── 에지 케이스 ────────────────────────────────────────────────────────────

    void onePoint_returnsFlase()
    {
        RollingLeastSquares rls(5);
        rls.AddPoint(1.0, 1.0);
        double slope;
        QVERIFY(!rls.GetRate(slope));
    }

    void noPoints_returnsFalse()
    {
        RollingLeastSquares rls(5);
        double slope;
        QVERIFY(!rls.GetRate(slope));
    }

    void allSameX_singular_returnsFalse()
    {
        // 모든 x가 같으면 분모 = 0 → false
        RollingLeastSquares rls(5);
        rls.AddPoint(1.0, 0.0);
        rls.AddPoint(1.0, 1.0);
        rls.AddPoint(1.0, 2.0);
        double slope;
        QVERIFY(!rls.GetRate(slope));
    }

    // ── Reset ─────────────────────────────────────────────────────────────────

    void reset_clearsAllPoints()
    {
        RollingLeastSquares rls(5);
        rls.AddPoint(0.0, 0.0);
        rls.AddPoint(1.0, 1.0);
        rls.Reset();
        double slope;
        QVERIFY(!rls.GetRate(slope));
    }

    void afterReset_acceptsNewPoints()
    {
        RollingLeastSquares rls(5);
        rls.AddPoint(0.0, 100.0);
        rls.AddPoint(1.0, 200.0);
        rls.Reset();
        rls.AddPoint(0.0, 0.0);
        rls.AddPoint(1.0, 5.0);
        double slope;
        QVERIFY(rls.GetRate(slope));
        QVERIFY(qAbs(slope - 5.0) < 1e-9);
    }

    // ── 실제 사용 시나리오: Rate s/day 계산 ───────────────────────────────────
    // 28800 BPH 시계, slope of error = 1e-4 s/s → rate = 1e-4 * 86400 = 8.64 s/day

    void rateScenario_slopeToSecondsPerDay()
    {
        RollingLeastSquares rls(50);
        // 시뮬레이션: 오차가 1e-4 s/s 속도로 누적
        for (int i = 0; i < 20; i++)
            rls.AddPoint(static_cast<double>(i), 1e-4 * i);
        double slope;
        QVERIFY(rls.GetRate(slope));
        double rateSpd = slope * 86400.0;  // s/day
        QVERIFY(qAbs(rateSpd - 8.64) < 0.01);
    }
};

QTEST_APPLESS_MAIN(TestRollingLeastSquares)
#include "test_rolling_least_squares.moc"
