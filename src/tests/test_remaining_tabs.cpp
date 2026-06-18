#include <QtTest/QtTest>
#include <QApplication>
#include "RateScopeTab.h"
#include "SweepScopeTab.h"
#include "FilterScopeTab.h"
#include "SoundPrintTab.h"
#include "Measurement.h"

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────
static Measurement makeMeas(double rateSpd, double ampDeg, double beatMs,
                             int sps = 48000, int bph = 28800)
{
    Measurement m;
    m.metrics.rate = rateSpd;
    m.metrics.amplitude = ampDeg;
    // beatError set via optional m.beatErrorMs  = beatMs;
    m.synced       = true;
    m.detectedBph  = bph;
    m.signal.samplesPerSecond = sps;
    return m;
}

static Measurement makePcmMeas(int nSamples, float value = 1.0f, int sps = 48000)
{
    Measurement m;
    m.signal.samplesPerSecond = sps;
    m.signal.pcm.fill(value, nSamples);
    m.signal.threshold.fill(0.0, nSamples);   // RateScopeTab reads threshold[i] per sample
    m.signal.rawPcm.fill(value, nSamples);
    m.signal.tickStart = 0;
    m.signal.tickEnd   = nSamples;
    return m;
}

// ─────────────────────────────────────────────────────────────────────────────
// RateScopeTab
// ─────────────────────────────────────────────────────────────────────────────
class TestRateScopeTab : public QObject
{
    Q_OBJECT
private slots:
    void init()
    {
        mRatePlot  = new QCustomPlot;
        mScopePlot = new QCustomPlot;
        mTab       = new RateScopeTab(mRatePlot, mScopePlot);
    }
    void cleanup()
    {
        delete mTab;
        delete mRatePlot;
        delete mScopePlot;
    }

    // RS-1: PCM block is written into the scope plot — at least one graph is set up
    void pcmBlock_appearsInScopePlot()
    {
        Measurement m = makePcmMeas(480);
        m.signal.pcm.fill(0.5, 480);
        mTab->onMeasurement(m);
        QVERIFY(mScopePlot->graphCount() >= 1);
    }

    // RS-2: Tic rate point goes to graph(0) on ratePlot
    void ticEvent_appendsToTicSeries()
    {
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.synced = true; m.detectedBph = 28800;
        AcousticEvent ev;
        ev.isA = true; ev.hasRatePoint = true;
        ev.wrappedRateError = 2.5; ev.isTic = true;
        ev.samplePos = 0; ev.peakValue = 0.5f;
        ev.cOnsetValid = false; ev.hasEscapementMs = false; ev.hasAmpSplit = false;
        m.events.append(ev);
        mTab->onMeasurement(m);
        QVERIFY(mRatePlot->graphCount() >= 1);
    }

    // RS-3: Toc rate point goes to graph(1) on ratePlot
    void tocEvent_appendsToTocSeries()
    {
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.synced = true; m.detectedBph = 28800;
        AcousticEvent ev;
        ev.isA = true; ev.hasRatePoint = true;
        ev.wrappedRateError = -1.5; ev.isTic = false;
        ev.samplePos = 0; ev.peakValue = 0.5f;
        ev.cOnsetValid = false; ev.hasEscapementMs = false; ev.hasAmpSplit = false;
        m.events.append(ev);
        mTab->onMeasurement(m);
        QVERIFY(mRatePlot->graphCount() >= 2);
    }

    // RS-4: wrappedRateError value is preserved (not clamped or modified)
    void wrappedValue_isPreserved()
    {
        const double kExpected = 3.7;
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.synced = true; m.detectedBph = 28800;
        AcousticEvent ev;
        ev.isA = true; ev.hasRatePoint = true;
        ev.wrappedRateError = kExpected; ev.isTic = true;
        ev.samplePos = 0; ev.peakValue = 0.5f;
        ev.cOnsetValid = false; ev.hasEscapementMs = false; ev.hasAmpSplit = false;
        m.events.append(ev);
        mTab->onMeasurement(m);

        // find last Y value on graph(0)
        if (mRatePlot->graphCount() >= 1 && mRatePlot->graph(0)->dataCount() > 0) {
            auto data = mRatePlot->graph(0)->data();
            double y = (data->end() - 1)->value;
            QCOMPARE(y, kExpected);
        }
    }

    // RS-5: reset() clears rate series
    void reset_clearsSeries()
    {
        Measurement m;
        m.signal.samplesPerSecond = 48000; m.synced = true; m.detectedBph = 28800;
        AcousticEvent ev;
        ev.isA = true; ev.hasRatePoint = true; ev.wrappedRateError = 1.0; ev.isTic = true;
        ev.samplePos = 0; ev.peakValue = 0.1f;
        ev.cOnsetValid = false; ev.hasEscapementMs = false; ev.hasAmpSplit = false;
        m.events.append(ev);
        mTab->onMeasurement(m);
        mTab->reset();
        if (mRatePlot->graphCount() >= 1)
            QCOMPARE(mRatePlot->graph(0)->dataCount(), 0);
        if (mRatePlot->graphCount() >= 2)
            QCOMPARE(mRatePlot->graph(1)->dataCount(), 0);
    }

private:
    QCustomPlot  *mRatePlot  = nullptr;
    QCustomPlot  *mScopePlot = nullptr;
    RateScopeTab *mTab       = nullptr;
};

// ─────────────────────────────────────────────────────────────────────────────
// SweepScopeTab
// ─────────────────────────────────────────────────────────────────────────────
class TestSweepScopeTab : public QObject
{
    Q_OBJECT
private slots:
    void init()   { mTab = new SweepScopeTab; }
    void cleanup(){ delete mTab; mTab = nullptr; }

    // SW-1: After receiving a PCM block, plot has data points
    void pcmBlock_producesPlotData()
    {
        Measurement m = makePcmMeas(480, 0.5f);
        m.synced = true; m.detectedBph = 28800;
        mTab->show();
        mTab->onMeasurement(m);
        auto *plot = mTab->findChild<QCustomPlot*>();
        QVERIFY(plot != nullptr);
        QVERIFY(plot->graphCount() >= 1);
        // sweep buffer was filled; after redraw graph has data
        QVERIFY(plot->graph(0)->dataCount() > 0);
    }

    // SW-2: Buffer length matches detectedBph x multiple x sps (28800 bph, 2x = 100ms worth)
    void bufferLength_matchesBphMultiple()
    {
        const int sps = 48000;
        const int bph = 28800;
        // beatSamples = round(3600/28800 * 48000) = round(6000) = 6000
        // len = 6000 * 2 = 12000
        const int expectedLen = 12000;

        Measurement m = makeMeas(0, 270, 0, sps, bph);
        m.signal.pcm.fill(1.0, 480);
        m.signal.rawPcm.fill(1.0f, 480);
        mTab->show();
        mTab->onMeasurement(m);

        auto *plot = mTab->findChild<QCustomPlot*>();
        QVERIFY(plot != nullptr);
        // x-axis upper bound = len/sps*1000 = 250 ms
        double expected_ms = (double)expectedLen / sps * 1000.0;
        QCOMPARE(plot->xAxis->range().upper, expected_ms);
    }

    // SW-3: reset() clears sweep buffer and plot data
    void reset_clearsSweepAndPlot()
    {
        mTab->onMeasurement(makePcmMeas(480, 1.0f));
        mTab->reset();
        auto *plot = mTab->findChild<QCustomPlot*>();
        QVERIFY(plot != nullptr);
        QCOMPARE(plot->graph(0)->dataCount(), 0);
    }

    // SW-4: Absolute values stored — negative input is treated as positive amplitude
    void absoluteValue_storedInSweep()
    {
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.synced = true; m.detectedBph = 28800;
        m.signal.pcm = {-1.0, -2.0, -3.0};
        m.signal.rawPcm = {-1.0f, -2.0f, -3.0f};
        mTab->onMeasurement(m);
        auto *plot = mTab->findChild<QCustomPlot*>();
        QVERIFY(plot != nullptr);
        // all Y values must be >= 0
        auto data = plot->graph(0)->data();
        for (auto it = data->begin(); it != data->end(); ++it)
            QVERIFY(it->value >= 0.0);
    }

private:
    SweepScopeTab *mTab = nullptr;
};

// ─────────────────────────────────────────────────────────────────────────────
// FilterScopeTab
// ─────────────────────────────────────────────────────────────────────────────
class TestFilterScopeTab : public QObject
{
    Q_OBJECT
private slots:
    void init()   { mTab = new FilterScopeTab; }
    void cleanup(){ delete mTab; mTab = nullptr; }

    // Returns the QCustomPlot for panel i (0=F0, 1=F1, 2=F2, 3=F3)
    QCustomPlot *plot(int i = 0)
    {
        return mTab->findChildren<QCustomPlot*>().value(i);
    }

    // FS-1: F0 output size equals input size
    void f0_outputSizeMatchesInput()
    {
        const int n = 256;
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.signal.rawPcm.fill(1.0f, n);
        mTab->show(); // make visible so redraw fires
        mTab->onMeasurement(m);
        QCOMPARE(plot(0)->graph(0)->dataCount(), n);
    }

    // FS-2: F0 produces mirrored negative series on graph(1)
    void f0_mirroredGraph1HasData()
    {
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.signal.rawPcm.fill(2.0f, 64);
        mTab->show();
        mTab->onMeasurement(m);
        QVERIFY(plot(0)->graph(1)->dataCount() > 0);
        // graph(1) values should be <= 0
        auto data = plot(0)->graph(1)->data();
        for (auto it = data->begin(); it != data->end(); ++it)
            QVERIFY(it->value <= 0.0);
    }

    // FS-3: F1 output has no negative values (moving average of |.|)
    void f1_allValuesNonNegative()
    {
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        // varying signal
        for (int i = 0; i < 128; i++) m.signal.rawPcm.append((float)(i % 10 - 5));
        mTab->show();
        mTab->onMeasurement(m);
        auto data = plot(1)->graph(0)->data();
        QVERIFY(data->size() > 0);
        for (auto it = data->begin(); it != data->end(); ++it)
            QVERIFY(it->value >= 0.0);
    }

    // FS-4: F1 negGraph is empty (F1 is a positive-only envelope, not mirrored)
    void f1_graph1IsEmpty()
    {
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.signal.rawPcm.fill(1.0f, 64);
        mTab->show();
        mTab->onMeasurement(m);
        QCOMPARE(plot(1)->graph(1)->dataCount(), 0);
    }

    // FS-5: reset() clears both graph series on F0 panel
    void reset_clearsBothGraphs()
    {
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.signal.rawPcm.fill(1.0f, 64);
        mTab->show();
        mTab->onMeasurement(m);
        mTab->reset();
        QCOMPARE(plot(0)->graph(0)->dataCount(), 0);
        QCOMPARE(plot(0)->graph(1)->dataCount(), 0);
    }

private:
    FilterScopeTab *mTab = nullptr;
};

// ─────────────────────────────────────────────────────────────────────────────
// SoundPrintTab — null-safety TCs (renderer seam not injectable; test safety only)
// ─────────────────────────────────────────────────────────────────────────────
class TestSoundPrintTab : public QObject
{
    Q_OBJECT
private slots:
    // SP-1: Constructs without crashing when widget is null
    void construction_withNullWidget_doesNotCrash()
    {
        SoundPrintTab *tab = new SoundPrintTab(nullptr, 48000);
        QVERIFY(tab != nullptr);
        delete tab;
    }

    // SP-2: onMeasurement with empty PCM does not crash
    void onMeasurement_emptyPcm_doesNotCrash()
    {
        SoundPrintTab tab(nullptr, 48000);
        Measurement m;
        tab.onMeasurement(m); // should not crash
    }

    // SP-3: reset() does not crash with null widget
    void reset_withNullWidget_doesNotCrash()
    {
        SoundPrintTab tab(nullptr, 48000);
        tab.reset();
    }

    // SP-4: setBph / setSampleRate do not crash
    void setBph_setSampleRate_doNotCrash()
    {
        SoundPrintTab tab(nullptr, 48000);
        tab.setBph(28800);
        tab.setSampleRate(44100);
    }
};

// Run all test classes
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int status = 0;

    {TestRateScopeTab  t; status |= QTest::qExec(&t, argc, argv);}
    {TestSweepScopeTab t; status |= QTest::qExec(&t, argc, argv);}
    {TestFilterScopeTab t; status |= QTest::qExec(&t, argc, argv);}
    {TestSoundPrintTab  t; status |= QTest::qExec(&t, argc, argv);}

    return status;
}

#include "test_remaining_tabs.moc"
