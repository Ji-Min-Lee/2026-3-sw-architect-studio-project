#include <QtTest>
#include <QApplication>
#include "EscapementTab.h"
#include "LongTermTab.h"
#include "SequenceTab.h"
#include "BeatNoiseScopeTab.h"
#include "SpectrogramTab.h"
#include "WaveformCompTab.h"
#include "SoundPrintTab.h"
#include "RateScopeTab.h"
#include "Measurement.h"
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Measurement 빌더 헬퍼
// ─────────────────────────────────────────────────────────────────────────────

// PCM 블록만 있는 기본 Measurement
static Measurement makeBase(int numSamples = 4096, int sps = 48000)
{
    Measurement m;
    m.samplesPerSecond = sps;
    m.graphTickStart   = 0;
    m.graphTickEnd     = numSamples;
    m.rawPcm.fill(0.0f, numSamples);
    m.pcm.fill(0.0, numSamples);
    m.threshold.fill(0.0, numSamples);
    return m;
}

// C 이벤트 (hasEscapementMs 선택)
static AcousticEvent makeCEvent(double samplePos, bool hasEsc = true,
                                double escapementMs = 9.0)
{
    AcousticEvent ev{};
    ev.isA              = false;
    ev.samplePos        = samplePos;
    ev.hasEscapementMs  = hasEsc;
    ev.escapementMs     = escapementMs;
    ev.hasRatePoint     = false;
    ev.hasAmpSplit      = false;
    return ev;
}

// A 이벤트
static AcousticEvent makeAEvent(double samplePos, bool hasRatePoint = false,
                                double wrappedRateError = 0.0, bool isTic = true)
{
    AcousticEvent ev{};
    ev.isA              = true;
    ev.samplePos        = samplePos;
    ev.hasRatePoint     = hasRatePoint;
    ev.wrappedRateError = wrappedRateError;
    ev.isTic            = isTic;
    ev.hasEscapementMs  = false;
    ev.hasAmpSplit      = false;
    return ev;
}

// rawPcm 크기 = sps (1초 분량) + rateValid/ampValid/beatErrorValid 설정
static Measurement makeOneSec(double rateSpd, double ampDeg, double beatMs,
                               int sps = 48000)
{
    Measurement m   = makeBase(sps, sps);   // rawPcm.size() == sps → 1 s
    m.rateValid     = true;
    m.rateErrorSpd  = rateSpd;
    m.amplitudeValid = true;
    m.amplitudeDeg  = ampDeg;
    m.beatErrorValid = true;
    m.beatErrorMs   = beatMs;
    m.synced        = true;
    return m;
}

// ─────────────────────────────────────────────────────────────────────────────
class TestRemainingTabs : public QObject
{
    Q_OBJECT

private slots:

    // ══════════════════════════════════════════════════════════════════════════
    // EscapementTab
    // ══════════════════════════════════════════════════════════════════════════

    void escapement_cEventWithEscapement_appearsInPlot()
    {
        EscapementTab tab;
        Measurement m = makeBase();
        m.events.append(makeCEvent(432.0, true, 9.0));
        tab.onMeasurement(m);

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 1);
        QVERIFY(qAbs(tab.plot()->graph(0)->data()->at(0)->value - 9.0) < 1e-9);
    }

    void escapement_cEventWithoutEscapement_notDrawn()
    {
        EscapementTab tab;
        Measurement m = makeBase();
        m.events.append(makeCEvent(432.0, false));
        tab.onMeasurement(m);

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
    }

    void escapement_aEvent_ignored()
    {
        EscapementTab tab;
        Measurement m = makeBase();
        m.events.append(makeAEvent(100.0));
        tab.onMeasurement(m);

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
    }

    void escapement_multipleEvents_xAxisIncrementsSequentially()
    {
        EscapementTab tab;
        Measurement m = makeBase();
        m.events.append(makeCEvent(432.0,  true, 9.0));
        m.events.append(makeCEvent(864.0,  true, 18.0));
        m.events.append(makeCEvent(1296.0, true, 27.0));
        tab.onMeasurement(m);

        auto data = tab.plot()->graph(0)->data();
        QCOMPARE(data->size(), 3);
        QCOMPARE(data->at(0)->key, 0.0);
        QCOMPARE(data->at(1)->key, 1.0);
        QCOMPARE(data->at(2)->key, 2.0);
    }

    void escapement_reset_clearsData()
    {
        EscapementTab tab;
        Measurement m = makeBase();
        m.events.append(makeCEvent(432.0, true, 9.0));
        tab.onMeasurement(m);
        tab.reset();

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // LongTermTab — DownsampledSeries 버킷 커밋 동작 검증
    // rawPcm.size() == sps → 1 s 진행. 2회 feed → 1 s 버킷 커밋.
    // ══════════════════════════════════════════════════════════════════════════

    void longterm_rate_appearsAfterBucketCommit()
    {
        LongTermTab tab;
        tab.onMeasurement(makeOneSec(+5.0, 250.0, 0.5));  // t=1 s, bucketStart=1
        tab.onMeasurement(makeOneSec(+6.0, 260.0, 0.6));  // t=2 s, commit!

        QCOMPARE(tab.ratePlot()->graph(0)->data()->size(), 1);
    }

    void longterm_amplitude_appearsAfterBucketCommit()
    {
        LongTermTab tab;
        tab.onMeasurement(makeOneSec(5.0, 250.0, 0.5));
        tab.onMeasurement(makeOneSec(6.0, 260.0, 0.6));

        QCOMPARE(tab.ampPlot()->graph(0)->data()->size(), 1);
    }

    void longterm_beatError_appearsAfterBucketCommit()
    {
        LongTermTab tab;
        tab.onMeasurement(makeOneSec(5.0, 250.0, 0.5));
        tab.onMeasurement(makeOneSec(6.0, 260.0, 0.6));

        QCOMPARE(tab.beatPlot()->graph(0)->data()->size(), 1);
    }

    void longterm_bucketAverage_isCorrect()
    {
        // 2회 평균: rate 5.0, 6.0 → bucket avg = 5.5 (1번째 값은 anchor, 2번째에서 커밋)
        LongTermTab tab;
        tab.onMeasurement(makeOneSec(5.0, 0, 0));
        tab.onMeasurement(makeOneSec(6.0, 0, 0));

        double committed = tab.ratePlot()->graph(0)->data()->at(0)->value;
        // bucketSum = 5.0 (첫 feed), commit 시 bucketSum/bucketN = 5.0/1 = 5.0
        QVERIFY(qAbs(committed - 5.0) < 1e-9);
    }

    void longterm_invalidRate_notAccumulated()
    {
        LongTermTab tab;
        Measurement m1 = makeOneSec(5.0, 250.0, 0.5);
        m1.rateValid = false;
        Measurement m2 = makeOneSec(5.0, 250.0, 0.5);
        m2.rateValid = false;
        tab.onMeasurement(m1);
        tab.onMeasurement(m2);

        QCOMPARE(tab.ratePlot()->graph(0)->data()->size(), 0);
    }

    void longterm_reset_clearsAllThreePlots()
    {
        LongTermTab tab;
        tab.onMeasurement(makeOneSec(5.0, 250.0, 0.5));
        tab.onMeasurement(makeOneSec(6.0, 260.0, 0.6));
        tab.reset();

        QCOMPARE(tab.ratePlot()->graph(0)->data()->size(),  0);
        QCOMPARE(tab.ampPlot()->graph(0)->data()->size(),   0);
        QCOMPARE(tab.beatPlot()->graph(0)->data()->size(),  0);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SequenceTab
    // ══════════════════════════════════════════════════════════════════════════

    void sequence_singleAEvent_noDataPlotted()
    {
        SequenceTab tab;
        Measurement m = makeBase();
        m.events.append(makeAEvent(1000.0));
        tab.onMeasurement(m);

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
    }

    void sequence_twoAEvents_oneIntervalPlotted()
    {
        SequenceTab tab;
        // 두 A 이벤트를 같은 블록에 넣음
        Measurement m = makeBase();
        m.events.append(makeAEvent(1000.0));
        m.events.append(makeAEvent(7000.0));  // +6000 샘플 = 125ms @48000Hz
        tab.onMeasurement(m);

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 1);
    }

    void sequence_intervalMs_isCorrect()
    {
        // 6000 샘플 / 48000 Hz = 0.125 s = 125 ms
        SequenceTab tab;
        Measurement m = makeBase();
        m.events.append(makeAEvent(1000.0));
        m.events.append(makeAEvent(7000.0));
        tab.onMeasurement(m);

        double val = tab.plot()->graph(0)->data()->at(0)->value;
        QVERIFY(qAbs(val - 125.0) < 1e-6);
    }

    void sequence_multipleBlocks_indexAccumulates()
    {
        SequenceTab tab;
        Measurement m1 = makeBase();
        m1.events.append(makeAEvent(1000.0));
        m1.events.append(makeAEvent(7000.0));
        tab.onMeasurement(m1);

        // m2: lastA=7000(m1에서 인계), 13000 하나만 → interval 7000→13000 추가
        Measurement m2 = makeBase();
        m2.events.append(makeAEvent(13000.0));
        tab.onMeasurement(m2);

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 2);
        QCOMPARE(tab.plot()->graph(0)->data()->at(1)->key, 1.0);
    }

    void sequence_reset_clearsData()
    {
        SequenceTab tab;
        Measurement m = makeBase();
        m.events.append(makeAEvent(1000.0));
        m.events.append(makeAEvent(7000.0));
        tab.onMeasurement(m);
        tab.reset();

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
    }

    void sequence_cEvent_ignored()
    {
        SequenceTab tab;
        Measurement m = makeBase();
        m.events.append(makeCEvent(1000.0, false));
        tab.onMeasurement(m);

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // BeatNoiseScopeTab
    // ══════════════════════════════════════════════════════════════════════════

    void beatNoise_emptyRawPcm_ignored()
    {
        BeatNoiseScopeTab tab;
        Measurement m;
        m.samplesPerSecond = 48000;
        // rawPcm 비어있음 → onMeasurement 첫 줄에서 리턴
        tab.onMeasurement(m);  // 크래시 없어야 함
        QCOMPARE(tab.scope1Plot()->graph(0)->data()->size(), 0);
    }

    void beatNoise_aEventAlone_scope1NoData()
    {
        BeatNoiseScopeTab tab;
        Measurement m = makeBase(48000);
        m.events.append(makeAEvent(1000.0));
        tab.onMeasurement(m);

        // A만 있고 C 없음 → scope1 미업데이트
        QCOMPARE(tab.scope1Plot()->graph(0)->data()->size(), 0);
    }

    void beatNoise_acPair_scope1DataAppears()
    {
        BeatNoiseScopeTab tab;
        // rawPcm이 충분히 커야 extractWindow 작동
        Measurement m = makeBase(48000);
        m.events.append(makeAEvent(1000.0));
        m.events.append(makeCEvent(1432.0, true, 9.0));
        tab.onMeasurement(m);

        // scope1 default range=20ms → halfSamples = 48000*20/1000 = 960
        QCOMPARE(tab.scope1Plot()->graph(0)->data()->size(), 960);
    }

    void beatNoise_reset_clearsScope1AndScope2()
    {
        BeatNoiseScopeTab tab;
        Measurement m = makeBase(48000);
        m.events.append(makeAEvent(1000.0));
        m.events.append(makeCEvent(1432.0, true, 9.0));
        tab.onMeasurement(m);
        tab.reset();

        QCOMPARE(tab.scope1Plot()->graph(0)->data()->size(), 0);
        QCOMPARE(tab.scope2Plot()->graph(0)->data()->size(), 0);
        QCOMPARE(tab.scope2Plot()->graph(1)->data()->size(), 0);
    }

    void beatNoise_scope2_ticAndTacSeparated()
    {
        BeatNoiseScopeTab tab;
        Measurement m = makeBase(48000);
        m.events.append(makeAEvent(1000.0));
        m.events.append(makeCEvent(1432.0, true, 9.0));
        tab.onMeasurement(m);

        // scope2: graph(0)=tic, graph(1)=tac → 둘 다 데이터 있어야 함
        QVERIFY(tab.scope2Plot()->graph(0)->data()->size() > 0);
        QVERIFY(tab.scope2Plot()->graph(1)->data()->size() > 0);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SpectrogramTab — computeSpectrum 직접 검증 (isVisible() 우회)
    // ══════════════════════════════════════════════════════════════════════════

    void spectrogram_emptyInput_returnsEmpty()
    {
        SpectrogramTab tab;
        double freqStep;
        auto result = tab.computeSpectrum({}, 48000, freqStep);
        QVERIFY(result.isEmpty());
    }

    void spectrogram_outputSize_isHalfNfft()
    {
        // 4096 샘플 → nfft=4096 → output = 2048
        SpectrogramTab tab;
        QVector<float> pcm(4096, 0.0f);
        double freqStep;
        auto result = tab.computeSpectrum(pcm, 48000, freqStep);
        QCOMPARE(result.size(), 2048);
    }

    void spectrogram_magnitudeNormalized_maxIs1()
    {
        // 임의 nonzero 신호 → 최대값 = 1.0
        SpectrogramTab tab;
        QVector<float> pcm(4096);
        const int sps = 48000;
        for (int i = 0; i < 4096; i++)
            pcm[i] = (float)std::sin(2.0 * M_PI * 440.0 * i / sps);

        double freqStep;
        auto mag = tab.computeSpectrum(pcm, sps, freqStep);

        double maxVal = *std::max_element(mag.begin(), mag.end());
        QVERIFY(qAbs(maxVal - 1.0) < 1e-9);
    }

    void spectrogram_sineWave_peakAtCorrectBin()
    {
        // 440 Hz 사인파 → 피크 bin ≈ 440 / freqStep
        SpectrogramTab tab;
        const int sps = 48000;
        const int n   = 4096;
        QVector<float> pcm(n);
        for (int i = 0; i < n; i++)
            pcm[i] = (float)std::sin(2.0 * M_PI * 440.0 * i / sps);

        double freqStep;
        auto mag = tab.computeSpectrum(pcm, sps, freqStep);

        int peakBin = (int)(std::max_element(mag.begin(), mag.end()) - mag.begin());
        double peakFreq = peakBin * freqStep;
        // freqStep = 48000/4096 ≈ 11.72 Hz → 440/11.72 ≈ 37.5 → bin 37 or 38
        QVERIFY(qAbs(peakFreq - 440.0) < 2.0 * freqStep);
    }

    void spectrogram_freqStep_matchesSampleRate()
    {
        // freqStep = sps / nfft = 48000 / 4096 ≈ 11.718 Hz
        SpectrogramTab tab;
        QVector<float> pcm(4096, 1.0f);
        double freqStep;
        tab.computeSpectrum(pcm, 48000, freqStep);
        QVERIFY(qAbs(freqStep - 48000.0 / 4096.0) < 1e-6);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // WaveformCompTab
    // ══════════════════════════════════════════════════════════════════════════

    void waveform_noRawPcm_ignored()
    {
        WaveformCompTab tab;
        Measurement m;
        m.samplesPerSecond = 48000;
        // rawPcm 비어있음 → 첫 줄에서 리턴
        tab.onMeasurement(m);
        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
    }

    void waveform_aEvent_ticGraphUpdated()
    {
        WaveformCompTab tab;
        // rawPcm을 충분히 크게 (center ± kWindowSamples/2 = 256)
        Measurement m = makeBase(4096);
        m.events.append(makeAEvent(512.0));  // center=512, window=[-256,256]
        tab.onMeasurement(m);

        QVERIFY(tab.plot()->graph(0)->data()->size() > 0);
    }

    void waveform_acPair_bothGraphsHaveData()
    {
        WaveformCompTab tab;
        Measurement m = makeBase(4096);
        m.events.append(makeAEvent(512.0));
        m.events.append(makeCEvent(1024.0, false));  // C event in same block
        tab.onMeasurement(m);

        QVERIFY(tab.plot()->graph(0)->data()->size() > 0);  // Tic
        QVERIFY(tab.plot()->graph(1)->data()->size() > 0);  // Toc
    }

    void waveform_cWithoutA_tocNotDrawn()
    {
        WaveformCompTab tab;
        Measurement m = makeBase(4096);
        m.events.append(makeCEvent(1000.0, false));
        tab.onMeasurement(m);

        QCOMPARE(tab.plot()->graph(1)->data()->size(), 0);
    }

    void waveform_reset_clearsBothGraphs()
    {
        WaveformCompTab tab;
        Measurement m = makeBase(4096);
        m.events.append(makeAEvent(512.0));
        m.events.append(makeCEvent(1024.0, false));
        tab.onMeasurement(m);
        tab.reset();

        QCOMPARE(tab.plot()->graph(0)->data()->size(), 0);
        QCOMPARE(tab.plot()->graph(1)->data()->size(), 0);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // SoundPrintTab — null widget 안전성 검증
    // ══════════════════════════════════════════════════════════════════════════

    void soundprint_nullWidget_constructionAndMeasurementNoCrash()
    {
        SoundPrintTab tab(nullptr, 48000);
        Measurement m = makeBase();
        m.events.append(makeAEvent(100.0));
        tab.onMeasurement(m);  // mWidget==nullptr → 즉시 리턴, 크래시 없어야 함
        QVERIFY(true);
    }

    void soundprint_nullWidget_resetNoCrash()
    {
        SoundPrintTab tab(nullptr, 48000);
        tab.reset();
        QVERIFY(true);
    }

    void soundprint_setSampleRate_accepted()
    {
        SoundPrintTab tab(nullptr, 48000);
        tab.setSampleRate(44100);  // 크래시 없어야 함
        QVERIFY(true);
    }

    void soundprint_setBph_accepted()
    {
        SoundPrintTab tab(nullptr, 48000);
        tab.setBph(28800);
        QVERIFY(true);
    }

    // ══════════════════════════════════════════════════════════════════════════
    // RateScopeTab
    // ══════════════════════════════════════════════════════════════════════════

    void rateScope_pcmData_addedToScopePlot()
    {
        QCustomPlot ratePlot, scopePlot;
        RateScopeTab tab(&ratePlot, &scopePlot);

        Measurement m = makeBase(256);
        tab.onMeasurement(m);

        QVERIFY(scopePlot.graph(0)->data()->size() > 0);
    }

    void rateScope_ticRatePoint_addedToGraph0()
    {
        QCustomPlot ratePlot, scopePlot;
        RateScopeTab tab(&ratePlot, &scopePlot);

        Measurement m = makeBase();
        m.events.append(makeAEvent(100.0, true, +3.5, true));  // Tic
        tab.onMeasurement(m);

        QCOMPARE(ratePlot.graph(0)->data()->size(), 1);  // Tic → graph(0)
        QCOMPARE(ratePlot.graph(1)->data()->size(), 0);  // Toc → graph(1) 비어있어야 함
    }

    void rateScope_tocRatePoint_addedToGraph1()
    {
        QCustomPlot ratePlot, scopePlot;
        RateScopeTab tab(&ratePlot, &scopePlot);

        Measurement m = makeBase();
        m.events.append(makeAEvent(100.0, true, -2.1, false));  // Toc
        tab.onMeasurement(m);

        QCOMPARE(ratePlot.graph(0)->data()->size(), 0);  // Tic 비어있어야 함
        QCOMPARE(ratePlot.graph(1)->data()->size(), 1);  // Toc → graph(1)
    }

    void rateScope_wrappedValue_isPreserved()
    {
        QCustomPlot ratePlot, scopePlot;
        RateScopeTab tab(&ratePlot, &scopePlot);

        Measurement m = makeBase();
        m.events.append(makeAEvent(100.0, true, +7.3, true));
        tab.onMeasurement(m);

        double val = ratePlot.graph(0)->data()->at(0)->value;
        QVERIFY(qAbs(val - 7.3) < 1e-9);
    }

    void rateScope_reset_clearsRatePlotAndScope()
    {
        QCustomPlot ratePlot, scopePlot;
        RateScopeTab tab(&ratePlot, &scopePlot);

        Measurement m = makeBase();
        m.events.append(makeAEvent(100.0, true, +3.5, true));
        tab.onMeasurement(m);
        tab.reset();

        QCOMPARE(ratePlot.graph(0)->data()->size(), 0);
        QCOMPARE(ratePlot.graph(1)->data()->size(), 0);
    }
};

QTEST_MAIN(TestRemainingTabs)
#include "test_remaining_tabs.moc"
