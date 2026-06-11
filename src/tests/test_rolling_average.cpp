#include <QtTest>
#include "RollingAverage.h"

class TestRollingAverage : public QObject
{
    Q_OBJECT

private slots:

    // ── 기본 동작 ──────────────────────────────────────────────────────────────

    void singleValue_returnsItself()
    {
        RollingAverage ra(5);
        ra.Add(7.0);
        QCOMPARE(ra.GetAverage(), 7.0);
        QCOMPARE(ra.CurrentSize(), 1);
    }

    void twoValues_returnsArithmicMean()
    {
        RollingAverage ra(5);
        ra.Add(4.0);
        ra.Add(6.0);
        QCOMPARE(ra.GetAverage(), 5.0);
    }

    void constantStream_returnsConstant()
    {
        RollingAverage ra(3);
        for (int i = 0; i < 10; i++) ra.Add(7.0);
        QCOMPARE(ra.GetAverage(), 7.0);
    }

    // ── 윈도우 슬라이딩 ────────────────────────────────────────────────────────

    void window3_slidesCorrectly()
    {
        // window=3, 값 [1,2,3,4] 추가 → 마지막 평균 = (2+3+4)/3
        RollingAverage ra(3);
        ra.Add(1.0); ra.Add(2.0); ra.Add(3.0); ra.Add(4.0);
        QCOMPARE(ra.CurrentSize(), 3);
        QVERIFY(qAbs(ra.GetAverage() - 3.0) < 1e-10);
    }

    void window_doesNotExceedCapacity()
    {
        RollingAverage ra(5);
        for (int i = 1; i <= 10; i++) ra.Add(static_cast<double>(i));
        QCOMPARE(ra.CurrentSize(), 5);
        // 마지막 5개: 6,7,8,9,10 → avg = 8.0
        QVERIFY(qAbs(ra.GetAverage() - 8.0) < 1e-10);
    }

    // ── Reset ─────────────────────────────────────────────────────────────────

    void reset_clearsAllState()
    {
        RollingAverage ra(5);
        ra.Add(10.0); ra.Add(20.0);
        ra.Reset();
        QCOMPARE(ra.CurrentSize(), 0);
        QCOMPARE(ra.GetAverage(), 0.0);
    }

    void afterReset_acceptsNewValues()
    {
        RollingAverage ra(5);
        ra.Add(100.0);
        ra.Reset();
        ra.Add(3.0);
        QCOMPARE(ra.GetAverage(), 3.0);
    }

    // ── Resize ────────────────────────────────────────────────────────────────

    void resize_shrink_trimsOldestValues()
    {
        // window=5, 값 [1,2,3,4,5], resize→2 → 최근 2개 [4,5] avg = 4.5
        RollingAverage ra(5);
        for (int i = 1; i <= 5; i++) ra.Add(static_cast<double>(i));
        ra.Resize(2);
        QCOMPARE(ra.CurrentSize(), 2);
        QVERIFY(qAbs(ra.GetAverage() - 4.5) < 1e-10);
    }

    void resize_grow_keepsExistingValues()
    {
        RollingAverage ra(3);
        ra.Add(1.0); ra.Add(2.0); ra.Add(3.0);
        ra.Resize(10);
        QCOMPARE(ra.CurrentSize(), 3);
        QVERIFY(qAbs(ra.GetAverage() - 2.0) < 1e-10);
    }

    // ── 빈 상태 ───────────────────────────────────────────────────────────────

    void emptyWindow_returns0()
    {
        RollingAverage ra(5);
        QCOMPARE(ra.GetAverage(), 0.0);
        QCOMPARE(ra.CurrentSize(), 0);
    }

    // ── 음수 값 ───────────────────────────────────────────────────────────────

    void negativeValues_averagedCorrectly()
    {
        RollingAverage ra(3);
        ra.Add(-6.0); ra.Add(-3.0); ra.Add(-3.0);
        QVERIFY(qAbs(ra.GetAverage() - (-4.0)) < 1e-10);
    }

    // ── 실제 사용 시나리오: Beat Error 평균 ────────────────────────────────────
    // window=10, 동일한 beat error 0.8 ms를 10회 → 평균 0.8

    void beatErrorScenario_constantError()
    {
        RollingAverage ra(10);
        for (int i = 0; i < 10; i++) ra.Add(0.8);
        QVERIFY(qAbs(ra.GetAverage() - 0.8) < 1e-10);
    }
};

QTEST_APPLESS_MAIN(TestRollingAverage)
#include "test_rolling_average.moc"
