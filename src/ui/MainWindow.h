#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QDir>
#include <QEvent>
#include <QMainWindow>
#include <QComboBox>
#include <QElapsedTimer>
#include <QMessageBox>
#include "SessionController.h"
#include "WavStreamWriter.h"
#include "WatchSynthStream.h"
#include "MeasurementEngine.h"
#include "SettingsManager.h"
#include "Logger.h"   // Logger, TG_NOW()
#include "WatchDiagnostics.h"
#include "WatchExplainer.h"
#include "DiagnosisDialog.h"
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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define AUDIO_OUTPUT 0
#define DEBUG_OUTPUT 0

// MVC: Controller.
// Reads UI state, builds domain VOs, and delegates session lifecycle to
// SessionController (Acquisition layer).  Owns the Presentation layer (tabs)
// and handles all UI feedback (results label, status bar, alarms).
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
    void on_WatchTypeComboBox_currentIndexChanged(int index);
    void on_ModeComboBox_currentTextChanged(const QString &arg1);
    void on_ScopeScaleSpinBox_valueChanged(int value);
    void on_ScopeScaleSlider_valueChanged(int value);
    void on_UseConsetCheckBox_toggled(bool checked);

    void onMeasurementReady(const Measurement &m);

public slots:
    void onSessionStopped();
    void onFrameLogged(Logger::Frame frame);

private:
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
    void   resetTabs(void);

    // Tab registration helpers — call instead of manually editing addTab + mAllTabs.
    // registerTab: adds to GraphicsTabWidget AND mAllTabs; returns tab for assignment.
    // addObserver:  adds to mAllTabs only (tabs embedded in existing .ui widgets).
    template<typename T>
    T*     registerTab(T *tab, const QString &label);
    void   addObserver(BaseGraphTab *tab);

    // Results display
    void   DisplayResults(const Measurement &m);

    // Live-mode watch-detached alarm (QAS-4)
    void   checkWatchDetached(const Measurement &m);
    void   raiseWatchDetachedAlarm(void);

    bool   eventFilter(QObject *obj, QEvent *event) override;

    Ui::MainWindow *ui;

    // Acquisition layer — session lifecycle delegate
    SessionController *mSession = nullptr;

    // Settings persistence
    SettingsManager *mSettings = nullptr;

    // Presentation layer (MVC: View / Observer: Concrete Observers) — 13 tabs
    RateScopeTab      *mRateScopeTab      = nullptr;
    TraceTab          *mTraceTab          = nullptr;
    SoundPrintTab     *mSoundPrintTab     = nullptr;
    BeatErrorTab      *mBeatErrorTab      = nullptr;
    VarioTab          *mVarioTab          = nullptr;
    SequenceTab       *mSequenceTab       = nullptr;
    BeatNoiseScopeTab *mBeatNoiseScopeTab = nullptr;
    LongTermTab       *mLongTermTab       = nullptr;
    EscapementTab     *mEscapementTab     = nullptr;
    SpectrogramTab    *mSpectrogramTab    = nullptr;
    WaveformCompTab   *mWaveformCompTab   = nullptr;
    SweepScopeTab     *mSweepScopeTab     = nullptr;
    FilterScopeTab    *mFilterScopeTab    = nullptr;
    QList<BaseGraphTab *> mAllTabs;

    // Watch-position testing (NIHS 95-10/ISO 3158)
    QString mActivePosition = "CH";

    // Live-mode watch-detached alarm state (edge detection)
    bool         mHadWatchSignal = false;
    bool         mWatchDetached  = false;
    QMessageBox *mDetachAlarm    = nullptr;

    // WAV recording (dialog + writer owned here; session does not touch it)
    WavStreamWriter *mWavWriter = nullptr;

    // UI/session state
    int       mLastReplotCount = 0;
    int64_t   mLastPlotUs     = 0;

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

    // FPS stats (updated from SessionController::frameLogged)
    double mBackgroundLastFPS = 0.0;
    double mBackgroundLastSPF = 0.0;
    double mBackgroundLastSPS = 0.0;
    double mDspLastFPS        = 0.0;
    double mDspLastSPF        = 0.0;
    double mDspLastSPS        = 0.0;

    // AI modules
    WatchDiagnostics mWatchDiagnostics;
    DiagnosisLevel   mLastDiagnosisLevel = DiagnosisLevel::Unknown;
    WatchType        mWatchType          = WatchType::Men;
    WatchExplainer   mWatchExplainer;
    ExplainRequest   mLastExplainRequest;
    DiagnosisDialog *mDiagnosisDialog    = nullptr;

    Logger          *mLogger = nullptr;

};
#endif
