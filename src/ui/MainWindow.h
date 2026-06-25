#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QDir>
#include <QEvent>
#include <QMainWindow>
#include <QComboBox>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QTimer>
#include <QToolButton>
#include <QCheckBox>
#include "SessionController.h"
#include "WavStreamWriter.h"
#include "WatchSynthStream.h"
#include "MeasurementEngine.h"
#include "SettingsManager.h"
#include "Logger.h"   // Logger, TG_NOW()
#include "WatchDiagnostics.h"
#include "WatchExplainer.h"
#include "DiagnosisDialog.h"
#include "UserGuideDialog.h"
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

enum class SessionState { Idle, Warming, Running, Paused };

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

    // Left control-panel space efficiency (feature/ui-improvement):
    // mode-driven visibility (SimFrame shows only in Sim mode) + collapsible
    // "Advanced" group (MiscFrame). Frames keep their .ui sizes; these helpers
    // only restack the left column vertically as visibility changes.
    void   relayoutLeftColumn(void);
    void   setAdvancedExpanded(bool expanded);
    void   updateLeftPanelForMode(void);

    // Graph-tab overflow: show the first kDefaultVisibleTabs as tabs, move the
    // rest behind a "More" drop-down. onGraphTabChanged() collapses a revealed
    // overflow tab back into the menu once the user returns to a default tab.
    // kDefaultVisibleTabs unused for initial visibility (set explicitly by widget ptr in setupTabOverflow)
    void   setupTabOverflow(void);
    void   showUserGuide(UserGuideSection section);
    void   showDiagnosisDialog(void);
    void   showTabConfigDialog(void);
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

    // Run-status line below Start/Pause/Stop (independent of DiagnosisLabel)
    void   updateRunStatusLine();
    void   startSessionClock();
    void   stopSessionClock();
    qint64 sessionElapsedMs() const;

    // Live-mode watch-detached alarm (QAS-4)
    void   checkWatchDetached(const Measurement &m);
    void   raiseWatchDetachedAlarm(void);

    // All-mode ambient-noise popup: warn when the environment is too noisy to
    // measure (>= 51 dB sustained, EXP-04 calibrated); dismiss when quiet again.
    void   checkNoise(const Measurement &m);
    void   raiseNoiseAlarm(void);

    // Demo: auto-detect horizontal <-> vertical from the amplitude drop and set
    // POS accordingly (enabled by the Sequence tab "Auto H↔V" checkbox). Starts
    // from horizontal; learns the horizontal amplitude baseline automatically.
    void   checkPosition(const Measurement &m);

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
    RadarChartTab     *mRadarChartTab     = nullptr;
    QList<BaseGraphTab *> mAllTabs;

    // Watch-position testing (NIHS 95-10/ISO 3158)
    QString mActivePosition = "CH";

    // Live-mode watch-detached alarm state (edge detection)
    bool         mHadWatchSignal = false;
    bool         mWatchDetached  = false;
    QMessageBox *mDetachAlarm    = nullptr;

    // All-mode ambient-noise popup state (debounced on both edges)
    bool          mNoiseShown = false;
    QElapsedTimer mNoiseAboveSince;        // continuous time at/above threshold
    QElapsedTimer mNoiseBelowSince;        // continuous time below threshold
    QMessageBox  *mNoiseAlarm = nullptr;
    static constexpr double  kNoiseThresholdDb = 51.0;   // EXP-04 calibrated (was 55; failure onset ~54)
    static constexpr qint64  kNoiseOnMs        = 2000;   // sustained → show
    static constexpr qint64  kNoiseOffMs       = 2000;   // sustained → hide

    // Demo: auto horizontal<->vertical position detection (amplitude step).
    QCheckBox    *mAutoPosCheck  = nullptr; // "Auto H↔V" toggle in the Advanced group
    bool          mPosRunActive  = false;  // a Live run is in progress (re-inits each run)
    bool          mPosVertical   = false;  // current detected state (false = horizontal)
    bool          mPosBaselineSet = false; // horizontal amplitude baseline learned?
    double        mPosBaselineAmp = 0.0;   // EMA of amplitude while horizontal
    QElapsedTimer mPosBelowSince;          // sustained time below the drop threshold
    QElapsedTimer mPosAboveSince;          // sustained time back near the baseline
    // On the demo rig lying flat (horizontal) reads HIGHER amplitude than
    // standing (vertical) — horizontal ~>=270, vertical ~<=260s — so vertical is
    // detected by an amplitude DROP below the learned horizontal baseline.
    static constexpr double kPosDropDeg    = 7.0;   // amp < baseline-this → vertical
    static constexpr double kPosReturnDeg  = 4.0;   // amp > baseline-this → horizontal (hysteresis)
    static constexpr qint64 kPosDebounceMs = 1200;  // sustained for this long before switching
    // Position labels shown for each class (vertical is acoustically ambiguous —
    // pick the one the demo physically uses; change here if needed).
    inline static const QString kPosHorizLabel = "CH";   // dial up (flat)
    inline static const QString kPosVertLabel  = "6H";   // standing (vertical)

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

    SessionState  mSessionState   = SessionState::Idle;
    int           mSyncedCount    = 0;
    QElapsedTimer mSessionTimer;
    qint64        mSessionActiveMs = 0;
    QTimer        mRunStatusTimer;

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
    DiagnosisDialog  *mDiagnosisDialog    = nullptr;
    UserGuideDialog  *mUserGuideDialog    = nullptr;

    // Left control-panel reflow state (feature/ui-improvement)
    QToolButton *mAdvancedToggle   = nullptr;  // collapses the MiscFrame "Advanced" group
    bool         mAdvancedExpanded = false;    // MiscFrame collapsed by default

    // Graph-tab overflow "More" drop-down (feature/ui-improvement)
    QToolButton *mMoreTabsButton   = nullptr;
    QMenu       *mMoreTabsMenu     = nullptr;

    // Split view (F4): moves one tab widget to a right panel; same instance = live data
    void   toggleSplitView();
    void   enterSplitView();
    void   exitSplitView();
    void   setSplitViewTab(int comboIdx);
    void   refreshSplitCombo();

    QSplitter   *mSplitter           = nullptr;
    QComboBox   *mSplitCombo         = nullptr;
    QWidget     *mSplitContentArea   = nullptr;
    QVBoxLayout *mSplitContentLayout = nullptr;
    QWidget     *mSplitCurrentWidget = nullptr;
    int          mSplitOriginalIdx   = -1;
    QString      mSplitOriginalName;
    bool         mSplitOriginalVisible = true;
    bool         mSplitMode          = false;

    Logger          *mLogger = nullptr;

};
#endif
