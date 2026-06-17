#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QDir>
#include <QMainWindow>
#include <QComboBox>
#include <QElapsedTimer>
#include <QMessageBox>
#include "AudioWorker.h"
#include "PlaybackWorker.h"
#include "SimWorker.h"
#include "DSPWorker.h"
#include "WavStreamWriter.h"
#include "WatchSynthStream.h"
#include "MeasurementEngine.h"
#include "SettingsManager.h"
#include "Logger.h"   // Logger, TG_NOW()
// Tabs (Presentation layer — all 11 graph tabs)
#include "RateScopeTab.h"
#include "TraceTab.h"
#include "SoundPrintTab.h"
#include "BeatErrorTab.h"
#include "VarioTab.h"
#include "SequenceTab.h"
#include "BeatNoiseScopeTab.h"
#include "LongTermTab.h"
#include "EscapementTab.h"
#include "SpectrogramTab.h"
#include "WaveformCompTab.h"
#include "SweepScopeTab.h"
#include "FilterScopeTab.h"
#include "RadarChartTab.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define AUDIO_OUTPUT 0
#define DEBUG_OUTPUT 0

// MVC: Controller.
// Owns threads (Acquisition layer), MeasurementEngine (Domain layer),
// and wires Observer connections from engine to tabs (Presentation layer).
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_RefreshPushButton_clicked();
    void on_MicrophoneHorizontalSlider_sliderMoved(int position);
    void on_StartPushButton_clicked();
    void on_PausePushButton_toggled(bool checked);
    void on_StopPushButton_clicked();
    void on_InputDeviceComboBox_currentIndexChanged(int index);
    void on_SampleRatesComboBox_currentIndexChanged(int index);
    void on_LiftAngleSpinBox_valueChanged(int arg1);
    void on_AveragingPeriodComboBox_currentIndexChanged(int index);
    void on_ModeComboBox_currentTextChanged(const QString &arg1);
    void on_ScopeScaleSpinBox_valueChanged(int value);
    void on_ScopeScaleSlider_valueChanged(int value);
    void on_UseConsetCheckBox_toggled(bool checked);

    void onMeasurementReady(const Measurement &m);

public slots:
    void HandlePlaybackDoneReadingFile();
    void HandleSimDone();
    void onFrameLogged(Logger::Frame frame);

signals:
    void LocalStartAudio(QAudioDevice InputDevice, int SampleRate, float Volume);
    void LocalStopAudio();
    void LocalSetAudioInputVolume(float Volume);
    void LocalStartPlayback(const QString &FileName);
    void LocalStartSim(WatchSynthStreamConfig cfg);

private:
    Ui::MainWindow *ui;

    // Thread management (Acquisition layer)
    void   StartAudioThread(void);
    void   StartPlaybackThread(const QString &FileName);
    void   StartSimThread(WatchSynthStreamConfig cfg);
    void   StopAudioThread(void);
    void   StopPlaybackThread(void);
    void   StopSimThread(void);

    // UI helpers
    void   LoadAudioDevices(void);
    void   LoadBPH(void);
    void   LoadSimBPH(void);
    void   LoadMode(void);
    void   LoadAverageingPeriod(void);
    void   ConfigureSoundCard(void);
    bool   RecordSessionCheck(void);
    void   AudioCloseCheck(void);
    bool   OpenFile(const QString &FileName);
    bool   SetAudioRate(int Rate);
    bool   SetAudioDevice(QString Name);
    void   GetAudioRate(int &Rate);
    void   GetAudioDevice(QString &Name);
    void   PopulateSampleRates(QComboBox *comboBox, const QAudioDevice &device);
    void   SetGuiRunMode(void);
    void   SetGuiStopMode(void);
    void   setDisplayPaused(bool on);
    void   LiveStart(void);
    void   PlaybackStart(void);
    void   SimStart(void);

    // Results display
    void   DisplayResults(const Measurement &m);

    // Live-mode watch-detached alarm (QAS-4): edge-detect signal loss while
    // measuring and raise a one-shot alarm + "Watch detached" message.
    void   checkWatchDetached(const Measurement &m);
    void   raiseWatchDetachedAlarm(void);

    void   Reset(void);
    void   wireEngineToTabs();

    // Settings persistence
    SettingsManager *mSettings = nullptr;

    // Domain layer (T2: owned by DSPWorker, accessed via mDspWorker->engine())
    DSPWorker *mDspWorker = nullptr;
    QThread   *mDspThread = nullptr;

    // Presentation layer (MVC: View / Observer: Concrete Observers) — 11 tabs
    RateScopeTab     *mRateScopeTab     = nullptr;
    TraceTab         *mTraceTab         = nullptr;
    SoundPrintTab    *mSoundPrintTab    = nullptr;
    BeatErrorTab     *mBeatErrorTab     = nullptr;
    VarioTab         *mVarioTab         = nullptr;
    SequenceTab      *mSequenceTab      = nullptr;
    BeatNoiseScopeTab *mBeatNoiseScopeTab = nullptr;
    LongTermTab      *mLongTermTab      = nullptr;
    EscapementTab    *mEscapementTab    = nullptr;
    SpectrogramTab   *mSpectrogramTab   = nullptr;
    WaveformCompTab  *mWaveformCompTab  = nullptr;
    SweepScopeTab    *mSweepScopeTab    = nullptr;
    FilterScopeTab   *mFilterScopeTab   = nullptr;
    RadarChartTab    *mRadarChartTab    = nullptr;  // bonus: multi-position radar
    QList<BaseGraphTab *> mAllTabs;     // for global pause + reset

    // Watch-position testing (NIHS 95-10/ISO 3158)
    QString mActivePosition = "CH";

    // Live-mode watch-detached alarm state (edge detection)
    bool         mHadWatchSignal = false;  // a watch was being measured this run
    bool         mWatchDetached  = false;  // currently detached (alarm latched)
    QMessageBox *mDetachAlarm    = nullptr;

    // Audio threads
    WavStreamWriter       *mWavWriter            = nullptr;
    TMasterAudioDataRaw   *mRawAudio              = nullptr;
    QThread               *mAudioWorkerThread     = nullptr;
    TAudioWorker          *mAudioWorker           = nullptr;
    QThread               *mPlaybackWorkerThread  = nullptr;
    TPlaybackWorker       *mPlaybackWorker        = nullptr;
    QThread               *mSimWorkerThread       = nullptr;
    TSimWorker            *mSimWorker             = nullptr;

    // Per-frame performance logger (active only when ENABLE_LOGGING is defined)
    Logger   *mLogger = nullptr;
    int       mLastReplotCount = 0;
    int64_t   mLastPlotUs     = 0;   // g_plotUs snapshot from last onMeasurementReady

    int       mAvalableRates[5];
    int       mNumberofRates        = 0;
    double    mLiftAngle            = 52.0;
    int       mAveragingPeriod      = 20;
    QDir      mCurrentDir;
    int       mCurrentSamplesPerSecond  = 48000;
    int       mRateBeforePlaybackOrSim  = 48000;
    QString   mDeviceNameBeforePlaybackOrSim;

    // CLI automation flags (--autostart, --no-record, --rate N, --duration N)
    bool          mNoRecord       = false;
    bool          mCmdAutoStart   = false;
    int           mCmdRate        = 0;
    int           mCmdDurationSec = 0;
    QElapsedTimer mCmdDurationTimer;

    // FPS stats (updated from DSPWorker::frameLogged)
    double mBackgroundLastFPS = 0.0;
    double mBackgroundLastSPF = 0.0;
    double mBackgroundLastSPS = 0.0;
    double mDspLastFPS        = 0.0;
    double mDspLastSPF        = 0.0;
    double mDspLastSPS        = 0.0;

};
#endif
