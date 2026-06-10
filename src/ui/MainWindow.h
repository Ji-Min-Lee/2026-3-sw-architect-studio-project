#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QDir>
#include <QMainWindow>
#include <QComboBox>
#include "AudioWorker.h"
#include "PlaybackWorker.h"
#include "SimWorker.h"
#include "WavStreamWriter.h"
#include "WatchSynthStream.h"
#include "MeasurementEngine.h"
#include "SettingsManager.h"
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
    void on_StopPushButton_clicked();
    void on_InputDeviceComboBox_currentIndexChanged(int index);
    void on_SampleRatesComboBox_currentIndexChanged(int index);
    void on_LiftAngleSpinBox_valueChanged(int arg1);
    void on_AveragingPeriodComboBox_currentIndexChanged(int index);
    void on_ModeComboBox_currentTextChanged(const QString &arg1);
    void on_ScopeScaleSpinBox_valueChanged(int value);
    void on_UseConsetCheckBox_toggled(bool checked);

    void onMeasurementReady(const Measurement &m);

public slots:
    void HandleAudioInput();
    void HandlePlaybackInput();
    void HandlePlaybackDoneReadingFile();
    void HandleSimInput();
    void HandleSimDone();

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
    void   LiveStart(void);
    void   PlaybackStart(void);
    void   SimStart(void);

    // Results display
    void   DisplayResults(const Measurement &m);

    // Core processing: reads ring buffer, feeds to engine
    void   HandleInputData(TMasterAudioDataRaw *SharedDataPtr);
    void   Reset(void);

    // Settings persistence
    SettingsManager *mSettings = nullptr;

    // Domain layer (MVC: Model / Observer: Subject)
    MeasurementEngine *mEngine = nullptr;

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

    // Audio threads
    WavStreamWriter       *mWavWriter            = nullptr;
    TMasterAudioDataRaw   *mRawAudio              = nullptr;
    QThread               *mAudioWorkerThread     = nullptr;
    TAudioWorker          *mAudioWorker           = nullptr;
    QThread               *mPlaybackWorkerThread  = nullptr;
    TPlaybackWorker       *mPlaybackWorker        = nullptr;
    QThread               *mSimWorkerThread       = nullptr;
    TSimWorker            *mSimWorker             = nullptr;

    // Persistent input block buffer (owned by MainWindow, fed to MeasurementEngine)
    float *mInputBlock = nullptr;


    int       mAvalableRates[5];
    int       mNumberofRates        = 0;
    double    mLiftAngle            = 52.0;
    int       mAveragingPeriod      = 20;
    unsigned int mLocalWriteIndex   = 0;
    uint64_t  mLocalTotalSamplesWritten = 0;
    QDir      mCurrentDir;
    int       mCurrentSamplesPerSecond  = 48000;
    int       mRateBeforePlaybackOrSim  = 48000;
    QString   mDeviceNameBeforePlaybackOrSim;

    // FPS stats
    double mBackgroundLastFPS = 0.0;
    double mBackgroundLastSPF = 0.0;
    double mBackgroundLastSPS = 0.0;
    double mForegroundFPS     = 0.0;
    double mForegroundSPF     = 0.0;
    double mForegroundSPS     = 0.0;
    double mForegroundLastFPS = 0.0;
    double mForegroundLastSPF = 0.0;
    double mForegroundLastSPS = 0.0;
    bool   mForegroundTimerStarted = false;
    QElapsedTimer mForegroundTimer;
    double mForegroundLastTime    = 0.0;
    uint64_t mForegroundFrameCount = 0;
    uint64_t mForegroundSampleCount = 0;

};
#endif
