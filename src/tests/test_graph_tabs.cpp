#include <QtTest>
#include <QApplication>
#include "TraceTab.h"
#include "BeatErrorTab.h"
#include "Measurement.h"

static QCustomPlot *findPlotBySeriesName(const TraceTab &tab, const QString &name)
{
    const auto plots = tab.findChildren<QCustomPlot *>();
    for (QCustomPlot *plot : plots) {
        if (plot->graphCount() > 0 && plot->graph(0)->name() == name)
            return plot;
    }
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper: 원하는 값으로 채운 Measurement 생성
// ─────────────────────────────────────────────────────────────────────────────
static Measurement makeMeasurement(double rateSpd,
                                    double beatMs,
                                    bool   hasAmpSplit,
                                    double ticAmp,
                                    double tocAmp)
{
    Measurement m;
    m.samplesPerSecond = 48000;

    // pcm/rawPcm 블록이 있어야 시간 계산이 진행됨 (TraceTab은 rawPcm.size() 사용)
    m.pcm.fill(0.0, 4096);
    m.rawPcm.fill(0.0f, 4096);

    m.rateValid      = true;
    m.rateErrorSpd   = rateSpd;

    m.beatErrorValid = true;
    m.beatErrorMs    = beatMs;

    m.amplitudeValid = true;
    m.amplitudeDeg   = (hasAmpSplit ? (ticAmp + tocAmp) / 2.0 : 0.0);

    if (hasAmpSplit) {
        AcousticEvent ev{};
        ev.isA             = false;
        ev.hasAmpSplit     = true;
        ev.ticAmpDeg       = ticAmp;
        ev.tocAmpDeg       = tocAmp;
        ev.hasEscapementMs = false;
        ev.hasRatePoint    = false;
        m.events.append(ev);
    }
    return m;
}

// ─────────────────────────────────────────────────────────────────────────────
class TestGraphTabs : public QObject
{
    Q_OBJECT

private slots:

    // ── TraceTab ──────────────────────────────────────────────────────────────

    void traceTab_rateValue_appearsInPlot()
    {
        TraceTab tab;
        tab.onMeasurement(makeMeasurement(+5.3, 0.0, false, 0, 0));

        auto data = tab.plot()->graph(0)->data();
        QCOMPARE(data->size(), 1);
        QCOMPARE(data->at(0)->value, +5.3);
    }

    void traceTab_negativeRate_appearsInPlot()
    {
        TraceTab tab;
        tab.onMeasurement(makeMeasurement(-12.7, 0.0, false, 0, 0));

        QCOMPARE(tab.plot()->graph(0)->data()->at(0)->value, -12.7);
    }

    void traceTab_amplitudeValue_appearsInSecondPlot()
    {
        TraceTab tab;
        tab.onMeasurement(makeMeasurement(0.0, 0.0, true, 270.0, 266.0));

        QCustomPlot *ampPlot = findPlotBySeriesName(tab, "Amplitude (°)");
        QVERIFY(ampPlot != nullptr);
        QCOMPARE(ampPlot->graph(0)->data()->size(), 1);
        QCOMPARE(ampPlot->graph(0)->data()->at(0)->value, 268.0);
    }

    void traceTab_multiplePoints_accumulateInOrder()
    {
        TraceTab tab;
        tab.onMeasurement(makeMeasurement(+2.0, 0, false, 0, 0));
        tab.onMeasurement(makeMeasurement(-1.5, 0, false, 0, 0));
        tab.onMeasurement(makeMeasurement(+4.0, 0, false, 0, 0));

        auto data = tab.plot()->graph(0)->data();
        QCOMPARE(data->size(), 3);
        QCOMPARE(data->at(1)->value, -1.5);
    }

    void traceTab_invalidRate_notDrawn()
    {
        TraceTab tab;
        Measurement m = makeMeasurement(+5.0, 0, false, 0, 0);
        m.rateValid = false;
        tab.onMeasurement(m);

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
    }

    void traceTab_invalidAmplitude_notDrawn()
    {
        TraceTab tab;
        Measurement m = makeMeasurement(0.0, 0.0, true, 270.0, 266.0);
        m.amplitudeValid = false;
        tab.onMeasurement(m);

        QCustomPlot *ampPlot = findPlotBySeriesName(tab, "Amplitude (°)");
        QVERIFY(ampPlot != nullptr);
        QCOMPARE(ampPlot->graph(0)->data()->size(), 0);
    }

    void traceTab_reset_clearsData()
    {
        TraceTab tab;
        tab.onMeasurement(makeMeasurement(+3.0, 0, false, 0, 0));
        tab.reset();
        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
        QCustomPlot *ampPlot = findPlotBySeriesName(tab, "Amplitude (°)");
        QVERIFY(ampPlot != nullptr);
        QCOMPARE(ampPlot->graph(0)->data()->size(), 0);
    }

    void traceTab_xAxis_advancesWithBlockSize()
    {
        TraceTab tab;
        // 블록 4096 샘플 @ 48000 Hz → Δt = 4096/48000 ≈ 0.0853 s
        tab.onMeasurement(makeMeasurement(1.0, 0, false, 0, 0));
        tab.onMeasurement(makeMeasurement(2.0, 0, false, 0, 0));

        auto data = tab.plot()->graph(0)->data();
        double dt = data->at(1)->key - data->at(0)->key;
        double expected = 4096.0 / 48000.0;
        QVERIFY(qAbs(dt - expected) < 1e-6);
    }

    // ── BeatErrorTab ─────────────────────────────────────────────────────────

    void beatErrorTab_value_appearsInPlot()
    {
        BeatErrorTab tab;
        tab.onMeasurement(makeMeasurement(0, 1.23, false, 0, 0));

        auto data = tab.plot()->graph(0)->data();
        QCOMPARE(data->size(), 1);
        QCOMPARE(data->at(0)->value, 1.23);
    }

    void beatErrorTab_multipleValues_inOrder()
    {
        BeatErrorTab tab;
        tab.onMeasurement(makeMeasurement(0, 0.5, false, 0, 0));
        tab.onMeasurement(makeMeasurement(0, 2.1, false, 0, 0));

        auto data = tab.plot()->graph(0)->data();
        QCOMPARE(data->size(), 2);
        QCOMPARE(data->at(0)->value, 0.5);
        QCOMPARE(data->at(1)->value, 2.1);
    }

    void beatErrorTab_invalidBeatError_notDrawn()
    {
        BeatErrorTab tab;
        Measurement m = makeMeasurement(0, 1.0, false, 0, 0);
        m.beatErrorValid = false;
        tab.onMeasurement(m);

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
    }

    void beatErrorTab_reset_clearsData()
    {
        BeatErrorTab tab;
        tab.onMeasurement(makeMeasurement(0, 1.0, false, 0, 0));
        tab.reset();
        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
    }

};

QTEST_MAIN(TestGraphTabs)
#include "test_graph_tabs.moc"
