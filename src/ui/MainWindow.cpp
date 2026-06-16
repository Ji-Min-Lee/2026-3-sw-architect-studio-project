#include <QtGlobal>
#include "MainWindow.h"
#include "ReplotCounter.h"
#include "ui_MainWindow.h"
#include "WaveHeader.h"
#include "SharedAudio.h"
#include "Timegrapher.h"
#include "Bph.h"
#include <QDateTime>
#include <QCoreApplication>

#if defined(Q_OS_LINUX)
#include "LinuxAudio.h"
#elif defined(Q_OS_WIN)
#include "WindowsAudio.h"
#endif

#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QtEndian>
#include <QDebug>
#include <QTextStream>
#include <QtMath>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QLabel>
#include <stdexcept>

#define  LIVE     0
#define  PLAYBACK 1
#define  SIM      2


#define PLAYBACK_OR_SIM_PCM             "Playback/Sim"

#define PREF_NAME_WELSHI                "Welshi USB"
#define PREF_NAME_CHINESE_GENERIC       "Chinese Generic USB"

#define LINUX_SOUND_CARD_NAME           "USB PnP Sound Device"
#define LINUX_SOUND_MIC_NAME            "Mic Capture Volume"
#define LINUX_SOUND_AGC_NAME            "Auto Gain Control"
#define LINUX_SOUND_MIC_PERCENT_VOLUME  50

#define WINDOWS_SOUND_ENDPOINT_NAME      "USB PnP Sound Device"
#define WINDOWS_SOUND_MIC_NAME           "USB PnP Sound Device"
#define WINDOWS_SOUND_MIC_PERCENT_VOLUME 50

#define SND_PIXEL_SIZE 3

static QString RenameAudioDevices[][2] =
{
  {"USB PnP Sound Device",        PREF_NAME_WELSHI},
  {"C-Media USB Headphone Set",   PREF_NAME_CHINESE_GENERIC},
  {"CM108 Audio Controller Mono", PREF_NAME_WELSHI},
  {"Audio Adapter Mono",          PREF_NAME_CHINESE_GENERIC}
};

static QString PreferredAudioDevices[] =
{
 PREF_NAME_WELSHI,
 PREF_NAME_CHINESE_GENERIC,
 "Cubilux HA-3",
 "CUBILUX CA7"
};

static QString ModeStrings[] =
{
        "Live",
        "Playback",
        "Sim",
};

static int ManualAutoBPH[]={0, //Auto
                    3600,  6000,  7200,  7380,  7440,  7800,  9000,  9100, 10800, 11880,
                    12000, 12342, 12480, 12600, 13320, 13440, 13500, 14000, 14040, 14160,
                    14200, 14280, 14400, 14520, 14580, 14760, 14850, 15000, 15360, 15600,
                    16200, 16320, 16800, 17196, 11258, 17280, 17186, 17897, 18000, 18049,
                    18514, 19332, 19440, 19800, 20160, 20222, 20944, 21000, 21031, 21306,
                    21600, 25200, 28800, 32400, 36000, 43200};

static int SimBPH[]={3600,  6000,  7200,  7380,  7440,  7800,  9000,  9100, 10800, 11880,
                     12000, 12342, 12480, 12600, 13320, 13440, 13500, 14000, 14040, 14160,
                     14200, 14280, 14400, 14520, 14580, 14760, 14850, 15000, 15360, 15600,
                     16200, 16320, 16800, 17196, 11258, 17280, 17186, 17897, 18000, 18049,
                     18514, 19332, 19440, 19800, 20160, 20222, 20944, 21000, 21031, 21306,
                     21600, 25200, 28800, 32400, 36000, 43200};

static int AveragingPeriodList[]={2,4,8,10,12,20,20,30,40,50,60,120,240};

// ─────────────────────────────────────────────────────────────────────────────
// Constructor — wire Domain layer → Presentation layer (AP-4 Observer)
// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QDir testDir;
    mCurrentDir = QDir::current();
    testDir = mCurrentDir;
    testDir.cd("../../TimeGrapherTestFilesWeishiMic");
    if (testDir.exists()) mCurrentDir = testDir;

    mCurrentSamplesPerSecond = 48000;
    mLiftAngle               = 52;

    ui->setupUi(this);
    this->setWindowTitle("TimeGrapher");
    ui->StopPushButton->setEnabled(false);
    ui->PausePushButton->setEnabled(false);
    ui->LiftAngleSpinBox->setFocusPolicy(Qt::NoFocus);
    ui->Results->setAlignment(Qt::AlignHCenter);
    ui->LiftAngleSpinBox->setValue(mLiftAngle);
    ui->SoundImage->CreateImage();

    // Domain layer (T2): MeasurementEngine lives in DSPWorker (created per-session in StartXxxThread)

    // ── Presentation layer: tabs (MVC View / Observer ConcreteObserver) ───────
    // AP-3: RateScopeTab wraps the existing QCustomPlot widgets from .ui
    mRateScopeTab = new RateScopeTab(ui->RatePlot, ui->ScopePlot, this);

    // AP-3: remaining 10 tabs — each ≤3-file change, added as new tab pages
    mTraceTab          = new TraceTab(this);
    mSoundPrintTab     = new SoundPrintTab(ui->SoundImage, mCurrentSamplesPerSecond, this);
    mBeatErrorTab      = new BeatErrorTab(this);
    mVarioTab          = new VarioTab(this);
    mSequenceTab       = new SequenceTab(this);
    mBeatNoiseScopeTab = new BeatNoiseScopeTab(this);
    mLongTermTab       = new LongTermTab(this);
    mEscapementTab     = new EscapementTab(this);
    mSpectrogramTab    = new SpectrogramTab(this);
    mWaveformCompTab   = new WaveformCompTab(this);
    mSweepScopeTab     = new SweepScopeTab(this);
    mFilterScopeTab    = new FilterScopeTab(this);

    mBeatNoiseScopeTab->setLiftAngle(mLiftAngle);
    mWaveformCompTab->setLiftAngle(mLiftAngle);

    ui->GraphicsTabWidget->addTab(mTraceTab,          "Trace");
    ui->GraphicsTabWidget->addTab(mBeatErrorTab,      "Beat Error");
    ui->GraphicsTabWidget->addTab(mVarioTab,          "Vario");
    ui->GraphicsTabWidget->addTab(mSequenceTab,       "Sequence");
    ui->GraphicsTabWidget->addTab(mBeatNoiseScopeTab, "Beat Scope");
    ui->GraphicsTabWidget->addTab(mLongTermTab,       "Long Term");
    ui->GraphicsTabWidget->addTab(mEscapementTab,     "Escapement");
    ui->GraphicsTabWidget->addTab(mSpectrogramTab,    "Spectrogram");
    ui->GraphicsTabWidget->addTab(mWaveformCompTab,   "Waveform");
    ui->GraphicsTabWidget->addTab(mSweepScopeTab,     "Sweep");
    ui->GraphicsTabWidget->addTab(mFilterScopeTab,    "Filters");

    // ── Observer: register() — connect Model → Views (AP-4) ──────────────────
    mAllTabs = {mRateScopeTab, mTraceTab, mSoundPrintTab, mBeatErrorTab,
                mVarioTab, mSequenceTab, mBeatNoiseScopeTab, mLongTermTab,
                mEscapementTab, mSpectrogramTab, mWaveformCompTab,
                mSweepScopeTab, mFilterScopeTab};

    QObject::connect(mSequenceTab, &SequenceTab::positionChanged,
                     this, [this](const QString &pos) { mActivePosition = pos; });

    // Results label subscription wired per-session in wireEngineToTabs() (T2)

    LoadBPH();
    LoadSimBPH();
    LoadAudioDevices();
    LoadAverageingPeriod();
    DisplayResults(Measurement{}); // show blank results
}

MainWindow::~MainWindow()
{
    delete mLogger; mLogger = nullptr;   // flushes CSV before UI teardown
    delete ui;
}

// ─────────────────────────────────────────────────────────────────────────────
// Controller slot: update Results label from Measurement
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onMeasurementReady(const Measurement &m)
{
    mLastReplotCount = g_replotCount.exchange(0);
    mLastPlotUs      = g_plotUs.exchange(0);
    checkWatchDetached(m);  // update detached state before formatting the label
    DisplayResults(m);
}

// Live-mode only: if a watch was being measured and the signal is then lost
// (watch removed from the microphone), raise a one-shot alarm. Re-arms when
// the signal returns, so re-attaching and removing again alarms again.
void MainWindow::checkWatchDetached(const Measurement &m)
{
    const bool liveRunning = ui->StopPushButton->isEnabled()
                           && ui->ModeComboBox->currentText() == ModeStrings[LIVE];
    if (!liveRunning) {
        mHadWatchSignal = false;
        mWatchDetached  = false;
        if (mDetachAlarm) mDetachAlarm->hide();
        return;
    }

    if (m.synced) {                 // a watch is currently being measured
        mHadWatchSignal = true;
        if (mWatchDetached && mDetachAlarm) mDetachAlarm->hide();  // reattached
        mWatchDetached  = false;    // re-arm
        return;
    }

    // No A-event for the no-signal threshold while we had a watch → detached
    if (m.noSignal && mHadWatchSignal && !mWatchDetached) {
        mWatchDetached = true;
        raiseWatchDetachedAlarm();
    }
}

void MainWindow::raiseWatchDetachedAlarm(void)
{
    QApplication::beep();           // audible alarm

    statusBar()->showMessage("Watch detached", 5000);

    if (!mDetachAlarm) {
        mDetachAlarm = new QMessageBox(this);
        mDetachAlarm->setIcon(QMessageBox::Warning);
        mDetachAlarm->setWindowTitle("Watch Detached");
        mDetachAlarm->setText("Watch detached");
        mDetachAlarm->setInformativeText(
            "The watch signal was lost during measurement. "
            "Reattach the watch to the microphone to continue.");
        mDetachAlarm->setStandardButtons(QMessageBox::Ok);
        mDetachAlarm->setModal(false);  // non-blocking: keep measuring underneath
    }
    if (!mDetachAlarm->isVisible()) {
        mDetachAlarm->show();
        mDetachAlarm->raise();
        mDetachAlarm->activateWindow();
    }
}

void MainWindow::DisplayResults(const Measurement &m)
{
    // View: format values for display — no domain logic here
    QString warning  = mWatchDetached ? "⚠ Watch detached   "
                     : m.noSignal      ? "⚠ No signal   " : "";
    QString bphStr   = m.synced ? QString("%1").arg(m.detectedBph, 5, 10, QChar(' ')) : "-----";
    QString rateStr  = m.rateValid ? QString::asprintf("%+6.1f", m.rateErrorSpd) : "------";
    QString beatStr  = m.beatErrorValid ? QString("%1").arg(m.beatErrorMs, 4, 'f', 1) : "----";
    QString ampStr   = m.amplitudeValid ? QString("%1°").arg(qRound64(m.amplitudeDeg), 3, 10, QChar(' ')) : "---";
    QString derived;
    if (m.derivedValid)
        derived = QString::asprintf("   |   DiffTicTac %+.2f ms   DiffPeriod %+.2f ms   AvgPeriod %+.2f ms",
                                    m.diffTicTacMs, m.diffPeriodMs, m.avgPeriodMs);
    ui->Results->setText(warning + "POS " + mActivePosition + "   RATE " + rateStr +
                         " s/d   AMPLITUDE " + ampStr +
                         "   BEAT ERROR " + beatStr + " ms   BEAT " + bphStr + " bph" + derived);
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
// T2: connect DSPWorker engine → all tabs + results label (called after DSPWorker created)
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::wireEngineToTabs()
{
    MeasurementEngine *eng = mDspWorker->engine();
    for (BaseGraphTab *tab : mAllTabs)
        QObject::connect(eng, &MeasurementEngine::measurementReady,
                         tab, &BaseGraphTab::onMeasurement, Qt::QueuedConnection);
    QObject::connect(eng, &MeasurementEngine::measurementReady,
                     this, &MainWindow::onMeasurementReady, Qt::QueuedConnection);
}

// ─────────────────────────────────────────────────────────────────────────────
// T2: receive per-frame log data from DSP thread, write CSV on main thread
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onFrameLogged(Logger::Frame frame)
{
#ifdef ENABLE_LOGGING
    frame.fg_wait_us   = TG_NOW() - frame.dsp_emit_ts;
    frame.replot_count = mLastReplotCount;
    frame.plot_us      = mLastPlotUs;
    if (mLogger && frame.samples > 0)
        mLogger->record(frame);
#else
    (void)frame;
#endif

    if (mBackgroundLastFPS != frame.bg_fps ||
        mBackgroundLastSPS != frame.bg_sps ||
        mDspLastFPS        != frame.fg_fps)
    {
        mBackgroundLastFPS = frame.bg_fps;
        mBackgroundLastSPS = frame.bg_sps;
        mBackgroundLastSPF = frame.bg_spf;
        mDspLastFPS = frame.fg_fps;
        mDspLastSPS = frame.fg_sps;
        mDspLastSPF = frame.fg_spf;
        statusBar()->showMessage(
            QString("BG FPS:%1 SPS:%2 SPF:%3 | DSP FPS:%4 SPS:%5 SPF:%6")
                .arg(mBackgroundLastFPS, 0, 'f', 0).arg(mBackgroundLastSPS, 0, 'f', 0)
                .arg(mBackgroundLastSPF, 0, 'f', 0).arg(mDspLastFPS, 0, 'f', 0)
                .arg(mDspLastSPS, 0, 'f', 0).arg(mDspLastSPF, 0, 'f', 0));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Reset — reinitialises Domain layer + Presentation layer
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::Reset(void)
{
    qInfo() << "RESET";
    // Domain layer (MeasurementEngine) is re-created inside DSPWorker per session.
    // Reset Presentation layer (MVC Views) — all tabs except the position
    // sequence, which must survive Stop/Start while repositioning the watch.
    for (BaseGraphTab *tab : mAllTabs)
        if (tab != mSequenceTab) tab->reset();

    DisplayResults(Measurement{});
    mBackgroundLastFPS = 0.0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Thread lifecycle (Acquisition layer)
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::StartAudioThread(void)
{
    QVariant deviceData = ui->InputDeviceComboBox->currentData();
    QAudioDevice InputDevice = deviceData.value<QAudioDevice>();
    Reset();
#ifdef ENABLE_LOGGING
    delete mLogger;
    {
        QString logDir = QCoreApplication::applicationDirPath() + "/logs/EXP-02";
        QDir().mkpath(logDir);
        QString csvPath = logDir + "/log_" +
                          QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv";
        mLogger = new Logger(csvPath, 100, mCurrentSamplesPerSecond);
    }
#endif
    if (mRawAudio) {
        if (mRawAudio->Samples) { delete[] mRawAudio->Samples; mRawAudio->Samples = nullptr; }
        delete mRawAudio; mRawAudio = nullptr;
    }
    mRawAudio = new TMasterAudioDataRaw;
    mRawAudio->NumberOfAudioSamples = mCurrentSamplesPerSecond * SECONDS_OF_BUFFER;
    mRawAudio->Samples = new float[mRawAudio->NumberOfAudioSamples];

    mAudioWorkerThread = new QThread();
    mAudioWorker = new TAudioWorker(mRawAudio);
    mAudioWorker->moveToThread(mAudioWorkerThread);

    QObject::connect(mAudioWorker, &TAudioWorker::finished,
                     mAudioWorkerThread, &QThread::quit);
    QObject::connect(mAudioWorkerThread, &QThread::finished,
                     mAudioWorker, &QObject::deleteLater);
    QObject::connect(mAudioWorkerThread, &QThread::finished,
                     mAudioWorkerThread, &QObject::deleteLater);
    QObject::connect(this, &MainWindow::LocalStartAudio,
                     mAudioWorker, &TAudioWorker::StartAudioRecording);
    QObject::connect(this, &MainWindow::LocalStopAudio,
                     mAudioWorker, &TAudioWorker::StopAudioRecording);
    QObject::connect(this, &MainWindow::LocalSetAudioInputVolume,
                     mAudioWorker, &TAudioWorker::SetAudioInputVolume);

    // T2: create DSP thread and wire audio worker → DSPWorker
    int bph = ManualAutoBPH[ui->BPHComboBox->currentIndex()];
    mDspWorker = new DSPWorker(mRawAudio, mCurrentSamplesPerSecond, bph, mLiftAngle,
                                mAveragingPeriod, ui->HighLineEdit->text().toDouble(),
                                ui->UseConsetCheckBox->isChecked());
    mDspThread = new QThread();
    mDspWorker->moveToThread(mDspThread);
    QObject::connect(mAudioWorker, &TAudioWorker::AudioDataReady,
                     mDspWorker, &DSPWorker::onDataReady, Qt::QueuedConnection);
    QObject::connect(mDspWorker, &DSPWorker::frameLogged,
                     this, &MainWindow::onFrameLogged, Qt::QueuedConnection);
    QObject::connect(mAudioWorkerThread, &QThread::finished, mDspThread, &QThread::quit);
    QObject::connect(mDspThread, &QThread::finished, mDspWorker, &QObject::deleteLater);
    QObject::connect(mDspThread, &QThread::finished, mDspThread, &QObject::deleteLater);
    wireEngineToTabs();
    mDspThread->start();

    mAudioWorkerThread->start(QThread::TimeCriticalPriority);
    emit LocalStartAudio(InputDevice, mCurrentSamplesPerSecond,
                         ui->MicrophoneHorizontalSlider->sliderPosition() / 1000.0f);
}

void MainWindow::StopAudioThread(void) { emit LocalStopAudio(); }

void MainWindow::StartPlaybackThread(const QString &FileName)
{
    Reset();
#ifdef ENABLE_LOGGING
    delete mLogger;
    {
        QString logDir = QCoreApplication::applicationDirPath() + "/logs/EXP-02";
        QDir().mkpath(logDir);
        QString csvPath = logDir + "/log_" +
                          QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv";
        mLogger = new Logger(csvPath, 100, mCurrentSamplesPerSecond);
    }
#endif
    if (mRawAudio) {
        if (mRawAudio->Samples) { delete[] mRawAudio->Samples; mRawAudio->Samples = nullptr; }
        delete mRawAudio; mRawAudio = nullptr;
    }
    mRawAudio = new TMasterAudioDataRaw;
    mRawAudio->NumberOfAudioSamples = mCurrentSamplesPerSecond * SECONDS_OF_BUFFER;
    mRawAudio->Samples = new float[mRawAudio->NumberOfAudioSamples];

    mPlaybackWorkerThread = new QThread();
    mPlaybackWorker = new TPlaybackWorker(mRawAudio, mCurrentSamplesPerSecond);
    mPlaybackWorker->moveToThread(mPlaybackWorkerThread);

    QObject::connect(mPlaybackWorker, &TPlaybackWorker::finished,
                     mPlaybackWorkerThread, &QThread::quit);
    QObject::connect(mPlaybackWorkerThread, &QThread::finished,
                     mPlaybackWorker, &QObject::deleteLater);
    QObject::connect(mPlaybackWorkerThread, &QThread::finished,
                     mPlaybackWorkerThread, &QObject::deleteLater);
    QObject::connect(this, &MainWindow::LocalStartPlayback,
                     mPlaybackWorker, &TPlaybackWorker::StartPlayback);
    QObject::connect(mPlaybackWorker, &TPlaybackWorker::PlaybackDoneReadingFile,
                     this, &MainWindow::HandlePlaybackDoneReadingFile);

    // T2: create DSP thread and wire playback worker → DSPWorker
    int bph = ManualAutoBPH[ui->BPHComboBox->currentIndex()];
    mDspWorker = new DSPWorker(mRawAudio, mCurrentSamplesPerSecond, bph, mLiftAngle,
                                mAveragingPeriod, ui->HighLineEdit->text().toDouble(),
                                ui->UseConsetCheckBox->isChecked());
    mDspThread = new QThread();
    mDspWorker->moveToThread(mDspThread);
    QObject::connect(mPlaybackWorker, &TPlaybackWorker::PlaybackDataReady,
                     mDspWorker, &DSPWorker::onDataReady, Qt::QueuedConnection);
    QObject::connect(mDspWorker, &DSPWorker::frameLogged,
                     this, &MainWindow::onFrameLogged, Qt::QueuedConnection);
    QObject::connect(mPlaybackWorkerThread, &QThread::finished, mDspThread, &QThread::quit);
    QObject::connect(mDspThread, &QThread::finished, mDspWorker, &QObject::deleteLater);
    QObject::connect(mDspThread, &QThread::finished, mDspThread, &QObject::deleteLater);
    wireEngineToTabs();
    mDspThread->start();

    mPlaybackWorkerThread->start(QThread::TimeCriticalPriority);
    emit LocalStartPlayback(FileName);
}

void MainWindow::StartSimThread(WatchSynthStreamConfig cfg)
{
    Reset();
#ifdef ENABLE_LOGGING
    delete mLogger;
    {
        QString logDir = QCoreApplication::applicationDirPath() + "/logs/EXP-02";
        QDir().mkpath(logDir);
        QString csvPath = logDir + "/log_" +
                          QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv";
        mLogger = new Logger(csvPath, 100, mCurrentSamplesPerSecond);
    }
#endif
    if (mRawAudio) {
        if (mRawAudio->Samples) { delete[] mRawAudio->Samples; mRawAudio->Samples = nullptr; }
        delete mRawAudio; mRawAudio = nullptr;
    }
    mRawAudio = new TMasterAudioDataRaw;
    mRawAudio->NumberOfAudioSamples = mCurrentSamplesPerSecond * SECONDS_OF_BUFFER;
    mRawAudio->Samples = new float[mRawAudio->NumberOfAudioSamples];

    mSimWorkerThread = new QThread();
    mSimWorker = new TSimWorker(mRawAudio, mCurrentSamplesPerSecond);
    mSimWorker->moveToThread(mSimWorkerThread);

    QObject::connect(mSimWorker, &TSimWorker::finished,
                     mSimWorkerThread, &QThread::quit);
    QObject::connect(mSimWorkerThread, &QThread::finished,
                     mSimWorker, &QObject::deleteLater);
    QObject::connect(mSimWorkerThread, &QThread::finished,
                     mSimWorkerThread, &QObject::deleteLater);
    QObject::connect(this, &MainWindow::LocalStartSim,
                     mSimWorker, &TSimWorker::StartSim);
    QObject::connect(mSimWorker, &TSimWorker::SimDone,
                     this, &MainWindow::HandleSimDone);

    // T2: create DSP thread and wire sim worker → DSPWorker
    int bph = ManualAutoBPH[ui->BPHComboBox->currentIndex()];
    mDspWorker = new DSPWorker(mRawAudio, mCurrentSamplesPerSecond, bph, mLiftAngle,
                                mAveragingPeriod, ui->HighLineEdit->text().toDouble(),
                                ui->UseConsetCheckBox->isChecked());
    mDspThread = new QThread();
    mDspWorker->moveToThread(mDspThread);
    QObject::connect(mSimWorker, &TSimWorker::SimDataReady,
                     mDspWorker, &DSPWorker::onDataReady, Qt::QueuedConnection);
    QObject::connect(mDspWorker, &DSPWorker::frameLogged,
                     this, &MainWindow::onFrameLogged, Qt::QueuedConnection);
    QObject::connect(mSimWorkerThread, &QThread::finished, mDspThread, &QThread::quit);
    QObject::connect(mDspThread, &QThread::finished, mDspWorker, &QObject::deleteLater);
    QObject::connect(mDspThread, &QThread::finished, mDspThread, &QObject::deleteLater);
    wireEngineToTabs();
    mDspThread->start();

    mSimWorkerThread->start(QThread::TimeCriticalPriority);
    emit LocalStartSim(cfg);
}

void MainWindow::StopPlaybackThread(void)
{
    if (mPlaybackWorkerThread) mPlaybackWorkerThread->requestInterruption();
}
void MainWindow::StopSimThread(void)
{
    if (mSimWorkerThread) mSimWorkerThread->requestInterruption();
}

// ─────────────────────────────────────────────────────────────────────────────
// Audio thread callbacks
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::HandlePlaybackDoneReadingFile()
{
    SetGuiStopMode();
    if (ui->ModeComboBox->currentIndex() == PLAYBACK) {
        SetAudioDevice(mDeviceNameBeforePlaybackOrSim);
        SetAudioRate(mRateBeforePlaybackOrSim);
    }
    AudioCloseCheck();
    statusBar()->showMessage("Stopped");
}
void MainWindow::HandleSimDone()
{
    SetGuiStopMode();
    if (ui->ModeComboBox->currentIndex() == SIM) {
        SetAudioDevice(mDeviceNameBeforePlaybackOrSim);
        SetAudioRate(mRateBeforePlaybackOrSim);
    }
    AudioCloseCheck();
    statusBar()->showMessage("Stopped");
}

// ─────────────────────────────────────────────────────────────────────────────
// UI helpers
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::ConfigureSoundCard(void)
{
#if defined(Q_OS_LINUX)
    LinuxSetSoundParameters(LINUX_SOUND_CARD_NAME, LINUX_SOUND_MIC_NAME,
                            LINUX_SOUND_AGC_NAME, LINUX_SOUND_MIC_PERCENT_VOLUME);
#elif defined(Q_OS_WIN)
    WindowsSetSoundParameters(WINDOWS_SOUND_ENDPOINT_NAME, WINDOWS_SOUND_MIC_NAME,
                               WINDOWS_SOUND_MIC_PERCENT_VOLUME);
#endif
}

void MainWindow::LoadAudioDevices(void)
{
    const QList<QAudioDevice> inputDevices = QMediaDevices::audioInputs();
    ui->InputDeviceComboBox->clear();
    int renameLen = sizeof RenameAudioDevices / sizeof RenameAudioDevices[0];
    for (const QAudioDevice &device : inputDevices) {
        QString desc = device.description();
        for (int i = 0; i < renameLen; i++) {
            if (desc.contains(RenameAudioDevices[i][0], Qt::CaseSensitive)) {
                desc = RenameAudioDevices[i][1]; break;
            }
        }
        ui->InputDeviceComboBox->addItem(desc, QVariant::fromValue(device));
    }
    ui->InputDeviceComboBox->addItem(PLAYBACK_OR_SIM_PCM);
    int preferredCount = std::size(PreferredAudioDevices);
    for (int i = 0; i < preferredCount; i++) {
        int idx = ui->InputDeviceComboBox->findText(PreferredAudioDevices[i], Qt::MatchContains);
        if (idx != -1) { ui->InputDeviceComboBox->setCurrentIndex(idx); break; }
    }
    LoadMode();
}

void MainWindow::LoadAverageingPeriod(void)
{
    auto length = std::size(AveragingPeriodList);
    for (int i = 0; i < (int)length; i++)
        ui->AveragingPeriodComboBox->addItem(
            QString::asprintf("%ds", AveragingPeriodList[i]), AveragingPeriodList[i]);
    ui->AveragingPeriodComboBox->setCurrentIndex(4);
}

void MainWindow::LoadBPH(void)
{
    auto length = std::size(ManualAutoBPH);
    for (int i = 0; i < (int)length; i++) {
        QString name = (ManualAutoBPH[i] != 0) ? QString::number(ManualAutoBPH[i]) : "Auto BPH";
        ui->BPHComboBox->addItem(name, ManualAutoBPH[i]);
    }
    ui->BPHComboBox->setCurrentIndex(0);
}

void MainWindow::LoadSimBPH(void)
{
    auto length = std::size(SimBPH);
    for (int i = 0; i < (int)length; i++)
        ui->SimBPHComboBox->addItem(QString::number(SimBPH[i]), SimBPH[i]);
    ui->SimBPHComboBox->setCurrentIndex(52);
}

void MainWindow::LoadMode(void)
{
    int start = 0;
    int modeCount = std::size(ModeStrings);
    ui->ModeComboBox->clear();
    if (ui->InputDeviceComboBox->count() == 1) start++;
    for (int i = start; i < modeCount; i++)
        ui->ModeComboBox->addItem(ModeStrings[i], i);
    ui->ModeComboBox->setCurrentIndex(0);
}

bool MainWindow::OpenFile(const QString &FileName)
{
    QFile *file = new QFile(FileName);
    TWaveHeader header;
    if (!file->exists()) {
        statusBar()->showMessage(tr("File %1 could not be opened").arg(FileName));
        delete file; return false;
    }
    QFileInfo fi(*file); mCurrentDir = fi.dir();
    if (!file->open(QIODevice::ReadOnly)) {
        statusBar()->showMessage(tr("File %1 could not be opened").arg(FileName));
        delete file; return false;
    }
    QDataStream in(file);
    in.setByteOrder(QDataStream::LittleEndian);
    file->read(header.riffId, 4); in >> header.fileSize;
    file->read(header.waveId, 4); file->read(header.fmtId, 4);
    in >> header.fmtSize >> header.audioFormat >> header.numChannels
       >> header.sampleRate >> header.byteRate >> header.blockAlign
       >> header.bitsPerSample;
    if (header.fmtSize > 16) file->seek(file->pos() + (header.fmtSize - 16));
    char chunkId[4];
    while (!file->atEnd()) {
        file->read(chunkId, 4);
        uint32_t chunkSize; in >> chunkSize;
        if (qstrncmp(chunkId, "data", 4) == 0) { header.dataSize = chunkSize; break; }
        file->seek(file->pos() + chunkSize);
    }
    GetAudioRate(mRateBeforePlaybackOrSim);
    GetAudioDevice(mDeviceNameBeforePlaybackOrSim);
    if (!SetAudioDevice(PLAYBACK_OR_SIM_PCM)) qInfo() << "SetAudioDevice Failed";
    if (!SetAudioRate(header.sampleRate))     qInfo() << "SetAudioRate Failed";
    if (qstrncmp(header.riffId, "RIFF", 4) != 0 ||
        header.sampleRate != (uint32_t)mCurrentSamplesPerSecond ||
        header.numChannels != 1 || header.bitsPerSample != 32 || header.audioFormat != 3)
    {
        statusBar()->showMessage(tr("File %1 Not a valid PCM Wave file").arg(FileName));
        file->close(); delete file;
        QMessageBox::critical(this, "Error", "Invalid PCM Wave File");
        return false;
    }
    file->close(); delete file; return true;
}

void MainWindow::PopulateSampleRates(QComboBox *comboBox, const QAudioDevice &device)
{
    QList<int> standardRates = {48000, 96000, 192000, 384000};
    comboBox->clear(); mNumberofRates = 0;
    if (device.isNull()) {
        for (int r : standardRates) {
            comboBox->addItem(QString::number(r) + " Hz", r);
            mAvalableRates[mNumberofRates++] = r;
        }
    } else {
        for (int rate : standardRates) {
            QAudioFormat fmt; fmt.setSampleRate(rate);
            fmt.setChannelCount(CHANNELS); fmt.setSampleFormat(SAMPLE_FORMAT);
            if (device.isFormatSupported(fmt)) {
                comboBox->addItem(QString::number(rate) + " Hz", rate);
                mAvalableRates[mNumberofRates++] = rate;
            }
        }
    }
    comboBox->setCurrentIndex(-1);
    comboBox->setCurrentIndex(0);
}

bool MainWindow::SetAudioRate(int Rate)
{
    int idx = ui->SampleRatesComboBox->findData(Rate);
    if (idx != -1) { ui->SampleRatesComboBox->setCurrentIndex(idx); return true; }
    return false;
}
bool MainWindow::SetAudioDevice(QString Name)
{
    int idx = ui->InputDeviceComboBox->findText(Name);
    if (idx != -1) { ui->InputDeviceComboBox->setCurrentIndex(idx); return true; }
    return false;
}
void MainWindow::GetAudioRate(int &Rate)     { Rate = mCurrentSamplesPerSecond; }
void MainWindow::GetAudioDevice(QString &Name) { Name = ui->InputDeviceComboBox->currentText(); }

bool MainWindow::RecordSessionCheck(void)
{
    QMessageBox msgBox;
    msgBox.setText("Record Session");
    msgBox.setInformativeText("Do you want to record this session?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::No);
    int dialogResult = msgBox.exec();
    if (dialogResult == QMessageBox::Yes) {
        QString fileName = QFileDialog::getSaveFileName(
            this, tr("Save Output File"), "../../Output/", tr("Wav Files (*.wav);;All Files (*)"));
        if (fileName.isEmpty()) return false;
        mWavWriter = new WavStreamWriter;
        if (!mWavWriter->open(fileName, mCurrentSamplesPerSecond, 1)) {
            QMessageBox::critical(this, "Error", "Failed to open WAV file");
            delete mWavWriter; mWavWriter = nullptr; return false;
        }
        return true;
    }
    return (dialogResult == QMessageBox::No);
}
void MainWindow::AudioCloseCheck(void)
{
    if (mWavWriter) { mWavWriter->close(); delete mWavWriter; mWavWriter = nullptr; }
}

// ─────────────────────────────────────────────────────────────────────────────
// GUI state management
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setDisplayPaused(bool on)
{
    for (BaseGraphTab *tab : mAllTabs)
        tab->setPaused(on);
    ui->PausePushButton->setText(on ? "Resume" : "Pause");
    statusBar()->showMessage(on ? "Paused — graphs frozen for diagnostic"
                                : "Running");
}

void MainWindow::SetGuiRunMode(void)
{
    ui->InputDeviceComboBox->setEnabled(false);
    ui->SampleRatesComboBox->setEnabled(false);
    ui->BPHComboBox->setEnabled(false);
    ui->ModeComboBox->setEnabled(false);
    ui->StartPushButton->setEnabled(false);
    ui->PausePushButton->setEnabled(true);
    ui->StopPushButton->setEnabled(true);
    ui->RefreshPushButton->setEnabled(false);
    ui->AveragingPeriodComboBox->setEnabled(false);
    ui->LiftAngleSpinBox->setEnabled(false);
    ui->SimAmplitudeSpinBox->setEnabled(false);
    ui->SimBeatErrorSpinBox->setEnabled(false);
    ui->SimBPHComboBox->setEnabled(false);
    ui->SimErrorRateSpinBox->setEnabled(false);
    ui->RealisticCheckBox->setEnabled(false);
    ui->UseConsetCheckBox->setEnabled(false);
    ui->HighLineEdit->setEnabled(false);
}
void MainWindow::SetGuiStopMode(void)
{
    ui->StopPushButton->setEnabled(false);
    ui->PausePushButton->setEnabled(false);
    if (ui->PausePushButton->isChecked()) {
        ui->PausePushButton->blockSignals(true);
        ui->PausePushButton->setChecked(false);
        ui->PausePushButton->blockSignals(false);
    }
    setDisplayPaused(false);
    ui->ModeComboBox->setEnabled(true);
    ui->RefreshPushButton->setEnabled(true);
    ui->StartPushButton->setEnabled(true);
    ui->InputDeviceComboBox->setEnabled(true);
    if (ui->ModeComboBox->currentText() != ModeStrings[PLAYBACK])
        ui->SampleRatesComboBox->setEnabled(true);
    ui->AveragingPeriodComboBox->setEnabled(true);
    ui->LiftAngleSpinBox->setEnabled(true);
    ui->BPHComboBox->setEnabled(true);
    ui->SimAmplitudeSpinBox->setEnabled(true);
    ui->SimBeatErrorSpinBox->setEnabled(true);
    ui->SimBPHComboBox->setEnabled(true);
    ui->SimErrorRateSpinBox->setEnabled(true);
    ui->RealisticCheckBox->setEnabled(true);
    ui->UseConsetCheckBox->setEnabled(true);
    ui->HighLineEdit->setEnabled(true);
}

void MainWindow::LiveStart(void)
{
    if (!RecordSessionCheck()) return;
    StartAudioThread();
    SetGuiRunMode();
    statusBar()->showMessage("Running");
}
void MainWindow::PlaybackStart(void)
{
    bool status = false;
    if (!RecordSessionCheck()) return;
    QFileDialog dlg(this, tr("Open Document"), mCurrentDir.absolutePath(),
                    tr("WAV Files (*.wav)"));
    dlg.setOptions(QFileDialog::DontUseNativeDialog);
    while (dlg.exec() == QDialog::Accepted && !(status = OpenFile(dlg.selectedFiles().constFirst()))) {}
    if (!status) return;
    StartPlaybackThread(dlg.selectedFiles().constFirst());
    SetGuiRunMode();
    statusBar()->showMessage("Running");
}
void MainWindow::SimStart(void)
{
    WatchSynthStreamConfig cfg;
    if (ui->RealisticCheckBox->isChecked())
        watch_synth_stream_realistic_config(&cfg);
    else
        watch_synth_stream_clean_config(&cfg);
    cfg.bph                   = SimBPH[ui->SimBPHComboBox->currentIndex()];
    cfg.sample_rate_hz        = mAvalableRates[ui->SampleRatesComboBox->currentIndex()];
    cfg.beat_error_ms         = -ui->SimBeatErrorSpinBox->value();
    cfg.pcm_peak_amplitude    = 0.40f;
    cfg.watch_amplitude_degrees = ui->SimAmplitudeSpinBox->value();
    cfg.lift_angle_degrees    = ui->LiftAngleSpinBox->value();
    cfg.rate_error_s_per_day  = ui->SimErrorRateSpinBox->value();
    if (!RecordSessionCheck()) return;
    GetAudioRate(mRateBeforePlaybackOrSim);
    GetAudioDevice(mDeviceNameBeforePlaybackOrSim);
    if (!SetAudioDevice(PLAYBACK_OR_SIM_PCM)) qInfo() << "SetAudioDevice Failed";
    if (!SetAudioRate(mRateBeforePlaybackOrSim)) qInfo() << "SetAudioRate Failed";
    StartSimThread(cfg);
    SetGuiRunMode();
    statusBar()->showMessage("Running");
}

// ─────────────────────────────────────────────────────────────────────────────
// UI signal handlers
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::on_RefreshPushButton_clicked()       { LoadAudioDevices(); }
void MainWindow::on_LiftAngleSpinBox_valueChanged(int)
{
    mLiftAngle = ui->LiftAngleSpinBox->value();
    if (mBeatNoiseScopeTab) mBeatNoiseScopeTab->setLiftAngle(mLiftAngle);
    if (mWaveformCompTab)   mWaveformCompTab->setLiftAngle(mLiftAngle);
}
void MainWindow::on_ScopeScaleSpinBox_valueChanged(int value) { mRateScopeTab->setScopeScale(value); }
void MainWindow::on_UseConsetCheckBox_toggled(bool checked)
{
    if (mDspWorker)
        QMetaObject::invokeMethod(mDspWorker, "setUseOnset",
                                  Qt::QueuedConnection, Q_ARG(bool, checked));
}
void MainWindow::on_AveragingPeriodComboBox_currentIndexChanged(int)
{
    mAveragingPeriod = AveragingPeriodList[ui->AveragingPeriodComboBox->currentIndex()];
}
void MainWindow::on_MicrophoneHorizontalSlider_sliderMoved(int)
{
    emit LocalSetAudioInputVolume(ui->MicrophoneHorizontalSlider->sliderPosition() / 1000.0f);
}
void MainWindow::on_StartPushButton_clicked()
{
    if (ui->ModeComboBox->currentText() == ModeStrings[LIVE])       { ConfigureSoundCard(); LiveStart(); }
    else if (ui->ModeComboBox->currentText() == ModeStrings[PLAYBACK]) PlaybackStart();
    else if (ui->ModeComboBox->currentText() == ModeStrings[SIM])      SimStart();
}
void MainWindow::on_PausePushButton_toggled(bool checked)
{
    setDisplayPaused(checked);
}
void MainWindow::on_StopPushButton_clicked()
{
    SetGuiStopMode();
    if (ui->ModeComboBox->currentText() == ModeStrings[LIVE]) {
        StopAudioThread(); AudioCloseCheck();
    } else if (ui->ModeComboBox->currentText() == ModeStrings[PLAYBACK]) {
        StopPlaybackThread();
        AudioCloseCheck();
        SetAudioDevice(mDeviceNameBeforePlaybackOrSim);
        SetAudioRate(mRateBeforePlaybackOrSim);
    } else if (ui->ModeComboBox->currentText() == ModeStrings[SIM]) {
        StopSimThread();
        SetAudioDevice(mDeviceNameBeforePlaybackOrSim);
        SetAudioRate(mRateBeforePlaybackOrSim);
    }
    statusBar()->showMessage("Stopped");
}
void MainWindow::on_InputDeviceComboBox_currentIndexChanged(int)
{
    QAudioDevice dev;
    if (ui->InputDeviceComboBox->currentText() != PLAYBACK_OR_SIM_PCM) {
        dev = ui->InputDeviceComboBox->currentData().value<QAudioDevice>();
        int idx = ui->ModeComboBox->findText(ModeStrings[LIVE]);
        if (idx != -1) ui->ModeComboBox->setCurrentIndex(idx);
    } else {
        if (ui->ModeComboBox->currentText() == ModeStrings[LIVE]) {
            int idx = ui->ModeComboBox->findText(ModeStrings[PLAYBACK]);
            if (idx != -1) ui->ModeComboBox->setCurrentIndex(idx);
        }
    }
    PopulateSampleRates(ui->SampleRatesComboBox, dev);
}
void MainWindow::on_SampleRatesComboBox_currentIndexChanged(int index)
{
    if (index < 0 || index >= mNumberofRates) return;
    mCurrentSamplesPerSecond = mAvalableRates[index];
}
void MainWindow::on_ModeComboBox_currentTextChanged(const QString &arg1)
{
    if (arg1 != ModeStrings[LIVE]) SetAudioDevice(PLAYBACK_OR_SIM_PCM);
    if (arg1 == ModeStrings[PLAYBACK]) ui->SampleRatesComboBox->setEnabled(false);
    else ui->SampleRatesComboBox->setEnabled(true);
    if (arg1 == ModeStrings[LIVE]) {
        bool isSet = false;
        for (int i = 0; i < (int)std::size(PreferredAudioDevices); i++) {
            int idx = ui->InputDeviceComboBox->findText(PreferredAudioDevices[i], Qt::MatchContains);
            if (idx != -1) { ui->InputDeviceComboBox->setCurrentIndex(idx); isSet = true; break; }
        }
        if (!isSet) {
            for (int i = 0; i < ui->InputDeviceComboBox->count(); i++) {
                if (ui->InputDeviceComboBox->itemText(i) != PLAYBACK_OR_SIM_PCM) {
                    ui->InputDeviceComboBox->setCurrentIndex(i); break;
                }
            }
        }
    }
}
