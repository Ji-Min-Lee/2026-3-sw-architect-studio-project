#include <QtTest>

#include "MeasurementSummaries.h"

#define private public
#include "VarioTab.h"
#undef private

static Measurement makeAmpMeasurement(double ticAmp, double tocAmp)
{
    Measurement m;
    m.samplesPerSecond = 48000;
    m.rawPcm.fill(0.0f, 4096);

    AcousticEvent ev{};
    ev.isA         = false;
    ev.hasAmpSplit = true;
    ev.ticAmpDeg   = ticAmp;
    ev.tocAmpDeg   = tocAmp;
    m.events.append(ev);
    return m;
}

class TestMeasurementSummaries : public QObject
{
    Q_OBJECT

private slots:
    void runningStats_minMaxMeanStddevCurrent_areCorrect()
    {
        RunningStats stats;
        stats.add(1.0);
        stats.add(2.0);
        stats.add(3.0);

        QCOMPARE(stats.count, 3);
        QCOMPARE(stats.current, 3.0);
        QCOMPARE(stats.min, 1.0);
        QCOMPARE(stats.max, 3.0);
        QVERIFY(qAbs(stats.mean() - 2.0) < 1e-9);
        QVERIFY(qAbs(stats.delta() - 2.0) < 1e-9);
        QVERIFY(qAbs(stats.stddev() - std::sqrt(2.0 / 3.0)) < 1e-9);
    }

    void sequenceSummary_xAndD_followOriginalRequirement()
    {
        SequenceSummary summary;
        summary.addPosition(+1.5, 300.0, 0.4);
        summary.addPosition(+2.0, 295.0, 0.6);
        summary.addPosition(-1.0, 285.0, 0.5);

        QVERIFY(qAbs(summary.rate.mean() - ((1.5 + 2.0 - 1.0) / 3.0)) < 1e-9);
        QVERIFY(qAbs(summary.rate.delta() - 3.0) < 1e-9);

        QVERIFY(qAbs(summary.amplitude.mean() - ((300.0 + 295.0 + 285.0) / 3.0)) < 1e-9);
        QVERIFY(qAbs(summary.amplitude.delta() - 15.0) < 1e-9);

        QVERIFY(qAbs(summary.beatError.mean() - 0.5) < 1e-9);
        QVERIFY(qAbs(summary.beatError.delta() - 0.2) < 1e-9);
    }

    void varioTab_accumulatesStatisticsFromAmplitudeEvents()
    {
        VarioTab tab;
        tab.onMeasurement(makeAmpMeasurement(300.0, 290.0));
        tab.onMeasurement(makeAmpMeasurement(306.0, 294.0));
        tab.onMeasurement(makeAmpMeasurement(294.0, 288.0));

        QCOMPARE(tab.mTicStats.count, 3);
        QCOMPARE(tab.mTocStats.count, 3);
        QVERIFY(qAbs(tab.mTicStats.min - 294.0) < 1e-9);
        QVERIFY(qAbs(tab.mTicStats.max - 306.0) < 1e-9);
        QVERIFY(qAbs(tab.mTicStats.mean() - 300.0) < 1e-9);
        QVERIFY(qAbs(tab.mTocStats.mean() - ((290.0 + 294.0 + 288.0) / 3.0)) < 1e-9);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // VarioTab — plot 값 검증 (graph(0)=Tic, graph(1)=Toc)
    // 통계(mTicStats)와 별개로, 플롯에 찍히는 Y값이 이벤트 값과 일치하는지 확인.
    // ══════════════════════════════════════════════════════════════════════════

    void varioTab_plotValues_matchTicTocEvents()
    {
        VarioTab tab;
        tab.onMeasurement(makeAmpMeasurement(300.0, 290.0));

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 1);
        QCOMPARE(tab.plot()->graph(1)->data()->size(), 1);
        QVERIFY(qAbs(tab.plot()->graph(0)->data()->at(0)->value - 300.0) < 1e-9);
        QVERIFY(qAbs(tab.plot()->graph(1)->data()->at(0)->value - 290.0) < 1e-9);
    }

    void varioTab_plotXAxis_incrementsPerEvent()
    {
        // X축 = beat index: 이벤트마다 0, 1, 2 순으로 증가해야 한다.
        VarioTab tab;
        tab.onMeasurement(makeAmpMeasurement(300.0, 290.0));
        tab.onMeasurement(makeAmpMeasurement(306.0, 294.0));
        tab.onMeasurement(makeAmpMeasurement(294.0, 288.0));

        auto *g0 = tab.plot()->graph(0);
        QCOMPARE(g0->data()->size(), 3);
        QCOMPARE(g0->data()->at(0)->key, 0.0);
        QCOMPARE(g0->data()->at(1)->key, 1.0);
        QCOMPARE(g0->data()->at(2)->key, 2.0);
    }

    void varioTab_plotTicAndTocIndependent_differentValues()
    {
        // Tic과 Toc이 서로 다른 값으로 각각 독립적으로 플롯되는지 확인.
        VarioTab tab;
        tab.onMeasurement(makeAmpMeasurement(310.0, 280.0));

        double ticY = tab.plot()->graph(0)->data()->at(0)->value;
        double tocY = tab.plot()->graph(1)->data()->at(0)->value;
        QVERIFY(qAbs(ticY - 310.0) < 1e-9);
        QVERIFY(qAbs(tocY - 280.0) < 1e-9);
        QVERIFY(qAbs(ticY - tocY - 30.0) < 1e-9);  // 두 값이 동일하지 않음
    }

    void varioTab_reset_clearsBothGraphsAndStats()
    {
        VarioTab tab;
        tab.onMeasurement(makeAmpMeasurement(300.0, 290.0));
        tab.reset();

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
        QCOMPARE(tab.plot()->graph(1)->data()->size(), 0);
        QCOMPARE(tab.mTicStats.count, 0);
        QCOMPARE(tab.mTocStats.count, 0);
    }

    void varioTab_noAmpSplitEvent_nothingPlotted()
    {
        // hasAmpSplit=false 이벤트는 VarioTab이 무시해야 한다.
        VarioTab tab;
        Measurement m;
        m.samplesPerSecond = 48000;
        m.rawPcm.fill(0.0f, 4096);
        AcousticEvent ev{};
        ev.isA         = false;
        ev.hasAmpSplit = false;  // ampSplit 없음
        m.events.append(ev);
        tab.onMeasurement(m);

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
        QCOMPARE(tab.plot()->graph(1)->data()->size(), 0);
    }
};

QTEST_MAIN(TestMeasurementSummaries)
#include "test_measurement_summaries.moc"
