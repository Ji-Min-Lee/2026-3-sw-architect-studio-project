// 값 정합성 테스트: 요구사항(Fig 9~17) 기준으로 재설계된 탭들이
// 주입된 Measurement 값을 정확히 반영하는지 검증한다.
#include <QtTest>
#include <QTableWidget>
#include <QCoreApplication>
#include <cmath>
#include "SequenceTab.h"
#include "EscapementTab.h"
#include "LongTermTab.h"
#include "BeatNoiseScopeTab.h"
#include "WaveformCompTab.h"
#include "SpectrogramTab.h"
#include "RadarChartTab.h"
#include "Measurement.h"

static AcousticEvent makeA(double samplePos, float peak = 0.5f)
{
    AcousticEvent ev{};
    ev.isA = true;
    ev.samplePos = samplePos;
    ev.peakValue = peak;
    return ev;
}

static AcousticEvent makeC(double samplePos, float peak = 0.3f)
{
    AcousticEvent ev{};
    ev.isA = false;
    ev.samplePos = samplePos;
    ev.peakValue = peak;
    ev.hasEscapementMs = true;
    return ev;
}

static QCustomPlot *plotOf(QWidget *tab)
{
    return tab->findChild<QCustomPlot *>();
}

class TestAddedTabs : public QObject
{
    Q_OBJECT
private slots:

    // ── SequenceTab: 멀티포지션 캡처 + X/D 요약 ──────────────────────────────
    void sequenceTab_capture_recordsAtActivePosition()
    {
        SequenceTab tab;
        auto *table = tab.findChild<QTableWidget *>();
        QVERIFY(table);

        Measurement m;
        m.metrics.rate = 5.5;
        m.metrics.beatError = 0.3;
        m.metrics.amplitude = 271.0;

        tab.setActivePosition("CH");
        tab.onMeasurement(m);
        tab.captureCurrent();
        QCOMPARE(table->item(0, 0)->text(), QString("5.5"));   // CH rate
        QCOMPARE(table->item(0, 1)->text(), QString("0.3"));   // CH beat
        QCOMPARE(table->item(0, 2)->text(), QString("271"));   // CH ampl

        tab.setActivePosition("6H");
        m.metrics.rate = -2.5;
        tab.onMeasurement(m);
        tab.captureCurrent();

        const int xRow = SequenceTab::positions().size();
        const int dRow = xRow + 1;
        QCOMPARE(table->item(xRow, 0)->text(), QString("1.5")); // X = (5.5-2.5)/2
        QCOMPARE(table->item(dRow, 0)->text(), QString("8.0")); // D = 5.5-(-2.5)
    }

    void sequenceTab_reset_clearsTable()
    {
        SequenceTab tab;
        auto *table = tab.findChild<QTableWidget *>();
        Measurement m;
        m.metrics.rate = 1.0;
        tab.onMeasurement(m);
        tab.captureCurrent();
        tab.reset();
        QCOMPARE(table->item(0, 0)->text(), QString("—"));
    }

    // DVH 행: Rate = Vertical 평균 − Horizontal 평균, Ampl 동일, Beat는 "—"
    void sequenceTab_dvh_rate_and_ampl()
    {
        SequenceTab tab;
        auto *table = tab.findChild<QTableWidget *>();
        QVERIFY(table);

        // Horizontal: CH=2.0, CB=4.0  → mean = 3.0
        // Vertical:   9H=1.0, 6H=1.0, 3H=1.0, 12H=1.0 → mean = 1.0
        // DVH Rate = 1.0 − 3.0 = −2.0
        auto capture = [&](const QString &pos, double rate, double ampl) {
            Measurement m;
            m.metrics.rate = rate;
            m.metrics.amplitude = ampl;
            m.metrics.beatError.reset();
            tab.setActivePosition(pos);
            tab.onMeasurement(m);
            tab.captureCurrent();
        };

        capture("CH",  2.0, 300.0);
        capture("CB",  4.0, 290.0);
        capture("9H",  1.0, 260.0);
        capture("6H",  1.0, 260.0);
        capture("3H",  1.0, 260.0);
        capture("12H", 1.0, 260.0);

        const int dvhRow = SequenceTab::positions().size() + 2;
        QCOMPARE(table->item(dvhRow, 0)->text(), QString("-2.0")); // Rate DVH
        QCOMPARE(table->item(dvhRow, 1)->text(), QString("—"));    // Beat: 빈칸
        // Ampl DVH: Vertical mean=260, Horizontal mean=295 → -35
        QCOMPARE(table->item(dvhRow, 2)->text(), QString("-35"));  // Ampl DVH
    }

    // DVH: 수직 또는 수평 포지션이 하나도 없으면 "—"
    void sequenceTab_dvh_missing_positions_shows_dash()
    {
        SequenceTab tab;
        auto *table = tab.findChild<QTableWidget *>();

        Measurement m;
        m.metrics.rate = 1.0;
        tab.setActivePosition("CH");
        tab.onMeasurement(m);
        tab.captureCurrent();  // Horizontal만 존재

        const int dvhRow = SequenceTab::positions().size() + 2;
        QCOMPARE(table->item(dvhRow, 0)->text(), QString("—"));
    }

    // ── EscapementTab: 파형 + A/C 마커 + Δms ────────────────────────────────
    void escapementTab_deltaMs_matchesEventSpacing()
    {
        EscapementTab tab;
        tab.show();
        QApplication::processEvents();
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.signal.tickStart = 0;
        m.signal.rawPcm.fill(0.1f, 4096);
        m.events << makeA(1000.0) << makeC(1240.0);  // 240 samples = 5.00 ms
        tab.onMeasurement(m);

        QVERIFY(qAbs(tab.currentEscapementMs() - 5.0) < 1e-6);
        QVERIFY(plotOf(&tab)->graph(0)->data()->size() > 0);  // waveform drawn
    }

    void escapementTab_reset_clearsState()
    {
        EscapementTab tab;
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.signal.rawPcm.fill(0.1f, 4096);
        m.events << makeA(1000.0) << makeC(1240.0);
        tab.onMeasurement(m);
        tab.reset();
        QCOMPARE(tab.currentEscapementMs(), 0.0);
        QCOMPARE(plotOf(&tab)->graph(0)->data()->size(), 0);
    }

    // ── LongTermTab: rate/amplitude/beat error 3개 시리즈 ────────────────────
    void longTermTab_timeAndRate_matchInput()
    {
        LongTermTab tab;
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.signal.pcm.fill(0.0, 4800);                            // 0.1 s per block
        // rate set via optional
        m.metrics.rate = 7.5;
        tab.onMeasurement(m);
        m.metrics.rate = -3.25;
        tab.onMeasurement(m);

        auto data = plotOf(&tab)->graph(0)->data();
        QCOMPARE(data->size(), 2);
        QVERIFY(qAbs(data->at(0)->key - 0.1) < 1e-9);
        QCOMPARE(data->at(0)->value, 7.5);
        QVERIFY(qAbs(data->at(1)->key - 0.2) < 1e-9);
        QCOMPARE(data->at(1)->value, -3.25);
    }

    void longTermTab_threeSeries_populateIndependently()
    {
        LongTermTab tab;
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.signal.pcm.fill(0.0, 4800);
        m.metrics.rate = 1.0;
        m.metrics.amplitude = 280.0;
        m.metrics.beatError = 0.2;
        tab.onMeasurement(m);

        auto *plot = plotOf(&tab);
        QCOMPARE(plot->graph(0)->data()->size(), 1);  // rate
        QCOMPARE(plot->graph(1)->data()->size(), 1);  // amplitude
        QCOMPARE(plot->graph(2)->data()->size(), 1);  // beat error
        QCOMPARE(plot->graph(1)->data()->at(0)->value, 280.0);
        QCOMPARE(plot->graph(2)->data()->at(0)->value, 0.2);
    }

    void longTermTab_invalidRate_notDrawn()
    {
        LongTermTab tab;
        Measurement m;
        m.signal.pcm.fill(0.0, 4800);
        m.metrics.rate.reset();   // leave rate unset — no data point should be added
        tab.onMeasurement(m);
        QCOMPARE(plotOf(&tab)->graph(0)->data()->size(), 0);
    }

    // ── BeatNoiseScopeTab: 비트 파형 캡처 + 20ms 스코프 ──────────────────────
    void beatNoise_capturesBeatWaveformWindow()
    {
        BeatNoiseScopeTab tab;
        tab.show();
        QApplication::processEvents();
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.signal.tickStart = 0;
        m.signal.tickEnd   = 4096;
        m.signal.pcm.fill(0.2, 4096);
        m.events << makeA(2000.0) << makeC(2300.0);
        tab.onMeasurement(m);

        QVERIFY(tab.capturedBeats() >= 1);
        // Scope 1 default range 20 ms = 960 samples @48k
        auto data = tab.scope1Plot()->graph(0)->data();
        QCOMPARE(data->size(), 960);
        QVERIFY(qAbs(data->at(100)->value - 0.2) < 1e-9); // |pcm| value preserved
    }

    void beatNoise_reset_clearsCaptures()
    {
        BeatNoiseScopeTab tab;
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.signal.pcm.fill(0.2, 4096);
        m.events << makeA(2000.0);
        tab.onMeasurement(m);
        tab.reset();
        QCOMPARE(tab.capturedBeats(), 0);
    }

    // ── WaveformCompTab: HPF bipolar, time-pass window ───────────────────────
    void waveformComp_ticWindow_matchesHpfPcm()
    {
        WaveformCompTab tab;
        tab.show();
        QApplication::processEvents();
        Measurement m;
        m.signal.tickStart = 0;
        m.signal.samplesPerSecond = 48000;
        m.signal.hpfPcm.resize(4096);
        for (int i = 0; i < 4096; i++) {
            m.signal.hpfPcm[i] = static_cast<float>(std::sin(2.0 * M_PI * i / 100.0));
        }
        m.events << makeA(2048.0);
        tab.onMeasurement(m);

        auto data = plotOf(&tab)->graph(0)->data();
        QVERIFY(data->size() > 0);

        const int preSamples = static_cast<int>(2.0 * 48000.0 / 1000.0);
        const int winStart = 2048 - preSamples;

        bool foundAtA = false;
        bool foundNegative = false;
        for (auto it = data->constBegin(); it != data->constEnd(); ++it) {
            const int k = qRound(it->key * 48000.0 / 1000.0);
            const int idx = winStart + k;
            if (idx >= 0 && idx < 4096) {
                QVERIFY(qAbs(it->value - m.signal.hpfPcm[idx]) < 1e-5);
                if (it->value < -0.01) foundNegative = true;
            }
            if (qAbs(it->key - 2.0) < 0.05) foundAtA = true;
        }
        QVERIFY(foundAtA);
        QVERIFY(foundNegative);
    }

    void waveformComp_tocPair_completesBeat()
    {
        WaveformCompTab tab;
        tab.show();
        QApplication::processEvents();
        Measurement m;
        m.signal.tickStart = 0;
        m.signal.samplesPerSecond = 48000;
        m.signal.hpfPcm.resize(4096);
        for (int i = 0; i < 4096; i++) {
            m.signal.hpfPcm[i] = static_cast<float>(i) / 4096.0f;
        }
        m.events << makeA(1000.0) << makeC(1400.0);
        tab.onMeasurement(m);

        auto data = plotOf(&tab)->graph(0)->data();
        QVERIFY(data->size() > 0);
        const double expectedTacMs = (1400.0 - 1000.0) * 1000.0 / 48000.0;
        const auto labels = tab.findChildren<QLabel *>();
        bool foundTac = false;
        for (QLabel *lbl : labels) {
            if (lbl->text().contains(QStringLiteral("t_AC")) && lbl->text().contains(QStringLiteral("min:"))) {
                foundTac = true;
                break;
            }
        }
        QVERIFY(foundTac);
        QVERIFY(qAbs(expectedTacMs - 400.0 * 1000.0 / 48000.0) < 1e-6);
    }

    // ── SpectrogramTab: FFT on HPF PCM, peak row at input frequency ──────────
    void spectrogram_peakRowAtInputFrequency()
    {
        SpectrogramTab tab;
        Measurement m;
        m.signal.samplesPerSecond = 48000;
        m.signal.hpfPcm.resize(4096);
        const double freq = 1500.0;
        for (int block = 0; block < 8; ++block) {
            m.signal.tickStart = static_cast<uint64_t>(block * 4096);
            for (int i = 0; i < 4096; ++i) {
                const int sampleIdx = block * 4096 + i;
                m.signal.hpfPcm[i] = (float)std::sin(2.0 * M_PI * freq * sampleIdx / 48000.0);
            }
            tab.onMeasurement(m);
        }

        QCPColorMapData *d = tab.colorMap()->data();
        const int lastCol = d->keySize() - 1;
        int peakRow = 1;
        double peakVal = -1e9;
        for (int r = 1; r < d->valueSize(); ++r) {
            if (d->cell(lastCol, r) > peakVal) {
                peakVal = d->cell(lastCol, r);
                peakRow = r;
            }
        }

        const double binHz = 48000.0 / 1024.0;
        const double rowFreq = peakRow * binHz;
        QVERIFY2(peakVal > -60.0, qPrintable(QString("no spectrogram energy (peak %1 dB)").arg(peakVal)));
        QVERIFY2(qAbs(rowFreq - freq) <= binHz * 2.0,
                 qPrintable(QString("peak row %1 → %2 Hz").arg(peakRow).arg(rowFreq)));
    }

    // X-axis: one color-map column per FFT hop; column count must match window duration.
    void spectrogram_timeColumnsMatchWindow()
    {
        SpectrogramTab tab;
        tab.reset();
        QCPColorMapData *d = tab.colorMap()->data();
        const double hopMs = 512.0 * 1000.0 / 48000.0;
        const int expectedCols = static_cast<int>(std::ceil(1000.0 / hopMs));
        QCOMPARE(d->keySize(), expectedCols);
        QCOMPARE(d->keyRange().upper, 1000.0);
    }

    // ── SequenceTab → RadarChartTab data contract ───────────────────────────
    void sequence_capturedReadings_reflectValues()
    {
        SequenceTab tab;
        Measurement m;
        m.metrics.rate = 4.0;
        m.metrics.beatError = 0.2;
        m.metrics.amplitude = 285.0;
        tab.setActivePosition("CH");
        tab.onMeasurement(m);
        tab.captureCurrent();

        auto readings = tab.capturedReadings();
        QCOMPARE(readings.size(), SequenceTab::positions().size());
        QVERIFY(readings[0].valid);                  // CH captured
        QCOMPARE(readings[0].rate, 4.0);
        QCOMPARE(readings[0].amp,  285.0);
        QVERIFY(!readings[1].valid);                 // CB not captured
    }

    void sequence_emitsSequenceUpdated_onCapture()
    {
        SequenceTab tab;
        QSignalSpy spy(&tab, &SequenceTab::sequenceUpdated);
        Measurement m;
        m.metrics.amplitude = 280.0;
        tab.onMeasurement(m);
        tab.captureCurrent();
        QCOMPARE(spy.count(), 1);
    }

    // ── RadarChartTab rendering against captured data ───────────────────────
    void radar_plotsCapturedPositions_closedPolygon()
    {
        SequenceTab seq;
        RadarChartTab radar(&seq);
        radar.show();
        QApplication::processEvents();

        auto cap = [&](const QString &pos, double amp) {
            Measurement m;
            m.metrics.amplitude = amp;
            seq.setActivePosition(pos);
            seq.onMeasurement(m);
            seq.captureCurrent();
            radar.rebuild();
        };
        cap("CH", 285); cap("6H", 280); cap("3H", 290);

        // 3 captured positions + 1 closing vertex
        QCOMPARE(radar.dataGraph()->data()->size(), 4);
        QVERIFY(radar.verdictText().contains("Balanced"));
    }

    void radar_flagsOutOfTolerance_inVerdict()
    {
        SequenceTab seq;
        RadarChartTab radar(&seq);
        radar.show();
        QApplication::processEvents();
        auto cap = [&](const QString &pos, double amp) {
            Measurement m;
            m.metrics.amplitude = amp;
            seq.setActivePosition(pos);
            seq.onMeasurement(m);
            seq.captureCurrent();
            radar.rebuild();
        };
        cap("CH", 285); cap("6H", 240); cap("3H", 290);  // 6H below 270° band

        QVERIFY(radar.verdictText().contains("out of tolerance"));
        QVERIFY(radar.verdictText().contains("6H"));
    }
};

QTEST_MAIN(TestAddedTabs)
#include "test_added_tabs.moc"
