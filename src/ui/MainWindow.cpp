#include <QtGlobal>
#include "MainWindow.h"
#include "MovementSpec.h"
#include "AcquisitionConfig.h"
#include "SessionController.h"
#include "ReplotCounter.h"
#include "ui_MainWindow.h"
#include "WaveHeader.h"
#include "Timegrapher.h"
#include "Bph.h"
#include <QDateTime>
#include <QCoreApplication>
#include <QMediaDevices>
#include <QAudioDevice>

#if defined(Q_OS_LINUX)
#include "LinuxAudio.h"
#elif defined(Q_OS_WIN)
#include "WindowsAudio.h"
#endif

namespace {

QString resolveRagDatabasePath()
{
    const QStringList candidates = {
        QCoreApplication::applicationDirPath() + QStringLiteral("/rag/vector.db"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../rag/vector.db"),
    };
    for (const QString &candidate : candidates) {
        const QString resolved = QFileInfo(candidate).canonicalFilePath();
        if (!resolved.isEmpty() && QFile::exists(resolved))
            return resolved;
    }
    return candidates.first();
}

} // namespace

#include <QFileDialog>
#include <QFile>
#include <QMenu>
#include <QAction>
#include <QDataStream>
#include <QtEndian>
#include <QDebug>
#include <QTextStream>
#include <QtMath>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QFileInfo>
#include <QTimer>
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

// Canonical BPH value list — shared by the Live/Playback combo (with leading 0
// for Auto) and the Sim combo (no Auto entry).
static constexpr int kBphValues[]={
                    3600,  6000,  7200,  7380,  7440,  7800,  9000,  9100, 10800, 11880,
                    12000, 12342, 12480, 12600, 13320, 13440, 13500, 14000, 14040, 14160,
                    14200, 14280, 14400, 14520, 14580, 14760, 14850, 15000, 15360, 15600,
                    16200, 16320, 16800, 17196, 11258, 17280, 17186, 17897, 18000, 18049,
                    18514, 19332, 19440, 19800, 20160, 20222, 20944, 21000, 21031, 21306,
                    21600, 25200, 28800, 32400, 36000, 43200};
// Live/Playback BPH combo: prepends 0 (Auto-detect) to kBphValues.
static constexpr int kManualAutoBPH[] = {0,
                    3600,  6000,  7200,  7380,  7440,  7800,  9000,  9100, 10800, 11880,
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
    ui->Results->setFont(QFont("Consolas", 9));

    // AI step 2: fetch available Ollama models → populate combobox
    connect(&mWatchExplainer, &WatchExplainer::modelsAvailable,
            this, [this](const QStringList &models) {
                ui->AiModelComboBox->blockSignals(true);
                ui->AiModelComboBox->clear();
                ui->AiModelComboBox->addItems(models);
                // index 0 = smallest model (sorted by size in WatchExplainer)
                ui->AiModelComboBox->setCurrentIndex(0);
                ui->AiModelComboBox->blockSignals(false);
                // warmup after we know which model to load
                const QString selected = ui->AiModelComboBox->currentText();
                mLastExplainRequest.modelName = selected;
                mWatchExplainer.warmup(selected);
            });
    connect(ui->AiModelComboBox, &QComboBox::currentTextChanged,
            this, [this](const QString &model) {
                mLastExplainRequest.modelName = model;
            });
    mWatchExplainer.checkAvailability();   // async: populates combobox, then warms up

    // RAG: load pre-computed embeddings from vector.db if present
    const QString ragDb = resolveRagDatabasePath();
    qInfo() << "[MainWindow] RAG database path:" << ragDb;
    mWatchExplainer.loadRag(ragDb);

    // clicking the diagnosis label opens the LLM explanation dialog
    ui->DiagnosisLabel->setCursor(Qt::PointingHandCursor);
    ui->DiagnosisLabel->setToolTip(tr("Hover for per-axis breakdown. Click for AI explanation."));
    ui->DiagnosisLabel->installEventFilter(this);
    ui->LiftAngleSpinBox->setValue(mLiftAngle);
    ui->SoundImage->CreateImage();

    // Acquisition layer — session lifecycle delegate
    mSession = new SessionController(this);
    connect(mSession, &SessionController::sessionStopped,
            this,     &MainWindow::onSessionStopped);
    connect(mSession, &SessionController::frameLogged,
            this,     &MainWindow::onFrameLogged, Qt::QueuedConnection);

    // ── Presentation layer: tabs (MVC View / Observer ConcreteObserver) ───────
    // Tabs embedded in existing .ui widgets — addObserver() registers them as
    // measurement observers without adding a new tab page.
    mRateScopeTab  = new RateScopeTab(ui->RatePlot, ui->ScopePlot, this);
    addObserver(mRateScopeTab);

    mSoundPrintTab = new SoundPrintTab(ui->SoundImage, mCurrentSamplesPerSecond, this);
    addObserver(mSoundPrintTab);

    // Standard tabs — registerTab() adds a new tab page AND registers as observer.
    // Adding a new tab: construct it here, call registerTab(), done — no other edits.
    // Bonus radar lives inside SequenceTab (table + radar side-by-side);
    // SequenceTab owns and wires it, so it is not registered separately.
    mTraceTab          = registerTab(new TraceTab(this),          "Trace");
    mBeatErrorTab      = registerTab(new BeatErrorTab(this),      "Beat Error");
    mVarioTab          = registerTab(new VarioTab(this),          "Vario");
    mSequenceTab       = registerTab(new SequenceTab(this),       "Sequence");
    mBeatNoiseScopeTab = registerTab(new BeatNoiseScopeTab(this), "Beat Scope");
    mLongTermTab       = registerTab(new LongTermTab(this),       "Long Term");
    mEscapementTab     = registerTab(new EscapementTab(this),     "Escapement");
    mSpectrogramTab    = registerTab(new SpectrogramTab(this),    "Spectrogram");
    mWaveformCompTab   = registerTab(new WaveformCompTab(this),   "Waveform");
    mSweepScopeTab     = registerTab(new SweepScopeTab(this),     "Sweep");
    mFilterScopeTab    = registerTab(new FilterScopeTab(this),    "Filters");

    mBeatNoiseScopeTab->setLiftAngle(mLiftAngle);
    mWaveformCompTab->setLiftAngle(mLiftAngle);

    QObject::connect(mSequenceTab, &SequenceTab::positionChanged,
                     this, [this](const QString &pos) { mActivePosition = pos; });

    // Register tabs with SessionController — engine→observer connections are
    // applied per-session in SessionController::startSourceThread().
    mSession->connectObservers(mAllTabs, this,
                               SLOT(onMeasurementReady(Measurement)));

    LoadBPH();
    LoadSimBPH();
    LoadAudioDevices();
    LoadAverageingPeriod();
    DisplayResults(Measurement{}); // show blank results

    // Parse CLI args for automated experiment runs:
    //   --rate N       : set sample rate (48000 / 96000 / 192000)
    //   --autostart    : click Start automatically after window opens
    //   --no-record    : answer NO to the "Record session?" dialog
    //   --duration N   : stop and exit after N seconds
    {
        QStringList cliArgs = QCoreApplication::arguments();
        for (int i = 1; i < cliArgs.size(); ++i) {
            if      (cliArgs[i] == "--rate"     && i + 1 < cliArgs.size()) mCmdRate        = cliArgs[++i].toInt();
            else if (cliArgs[i] == "--autostart")                           mCmdAutoStart   = true;
            else if (cliArgs[i] == "--no-record")                           mNoRecord       = true;
            else if (cliArgs[i] == "--duration" && i + 1 < cliArgs.size()) mCmdDurationSec = cliArgs[++i].toInt();
        }
        if (mCmdAutoStart) {
            QTimer::singleShot(500, this, [this]() {
                if (mCmdRate > 0) SetAudioRate(mCmdRate);
                int liveIdx = ui->ModeComboBox->findText(ModeStrings[LIVE]);
                if (liveIdx >= 0) ui->ModeComboBox->setCurrentIndex(liveIdx);
                on_StartPushButton_clicked();
                if (mCmdDurationSec > 0)
                    mCmdDurationTimer.start();
            });
        }
    }

    // ── Graph tabs: show the first 9, rest behind a "More" drop-down ──────
    setupTabOverflow();

    // ── Left control panel: reclaim screen space (feature/ui-improvement) ──
    // SimFrame is shown only in Sim mode, and the set-once MiscFrame collapses
    // behind an "Advanced" toggle (collapsed by default). Frames keep their .ui
    // sizes; relayoutLeftColumn() only restacks them as visibility changes.
    mAdvancedToggle = new QToolButton(ui->RunFrame->parentWidget());
    mAdvancedToggle->setObjectName("AdvancedToggle");
    mAdvancedToggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mAdvancedToggle->setArrowType(Qt::RightArrow);
    mAdvancedToggle->setText(tr("Advanced"));
    mAdvancedToggle->setAutoRaise(true);
    mAdvancedToggle->setCursor(Qt::PointingHandCursor);
    mAdvancedToggle->setFixedSize(242, 24);
    mAdvancedToggle->show();
    connect(mAdvancedToggle, &QToolButton::clicked, this,
            [this] { setAdvancedExpanded(!mAdvancedExpanded); });
    ui->MiscFrame->setVisible(false);   // "Advanced" collapsed by default
    mAdvancedExpanded = false;
    updateLeftPanelForMode();           // set Sim visibility + initial reflow
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ── Left control-panel space efficiency (feature/ui-improvement) ──────────────
// The .ui places the left frames with absolute geometry, so we restack them in
// code as their visibility changes (hiding a frame must pull the rest upward).

// Vertically stack the currently-visible left frames from the top, in priority
// order: core (Run, Watch) → mode-specific (Sim, only in Sim mode) → the
// "Advanced" toggle (always) → MiscFrame (only when expanded). Each frame keeps
// its .ui height; we only move it.
void MainWindow::relayoutLeftColumn(void)
{
    if (!mAdvancedToggle) return;          // not constructed yet (early signal)
    const int x = 0, gap = 4;
    int y = 0;
    auto place = [&](QWidget *w) {
        w->setGeometry(x, y, 242, w->height());
        y += w->height() + gap;
    };
    place(ui->RunFrame);
    place(ui->WatchFrame);
    if (!ui->SimFrame->isHidden()) place(ui->SimFrame);   // Sim mode only
    place(mAdvancedToggle);
    if (mAdvancedExpanded)         place(ui->MiscFrame);   // expanded only
}

// Collapse/expand the "Advanced" (MiscFrame) group of set-once parameters.
void MainWindow::setAdvancedExpanded(bool expanded)
{
    mAdvancedExpanded = expanded;
    if (mAdvancedToggle)
        mAdvancedToggle->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
    ui->MiscFrame->setVisible(expanded);
    relayoutLeftColumn();
}

// Show the simulation parameters only in Sim mode (less-used elsewhere), then
// reflow the column to reclaim the freed space.
void MainWindow::updateLeftPanelForMode(void)
{
    if (!mAdvancedToggle) return;          // not constructed yet (early signal)
    const bool simMode = (ui->ModeComboBox->currentText() == ModeStrings[SIM]);
    ui->SimFrame->setVisible(simMode);
    relayoutLeftColumn();
}

// ── Graph-tab overflow (feature/ui-improvement) ───────────────────────────────
// Keep the first kDefaultVisibleTabs graphs as always-visible tabs; collapse the
// remaining specialized scopes into a "More ▾" drop-down in the tab-bar corner.
void MainWindow::setupTabOverflow(void)
{
    QTabWidget *tw = ui->GraphicsTabWidget;
    const int n = tw->count();
    if (n <= kDefaultVisibleTabs) return;          // nothing to overflow

    mMoreTabsButton = new QToolButton(tw);
    mMoreTabsButton->setObjectName("MoreTabsButton");
    mMoreTabsButton->setText(tr("More"));
    mMoreTabsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mMoreTabsButton->setArrowType(Qt::DownArrow);
    mMoreTabsButton->setPopupMode(QToolButton::InstantPopup);
    mMoreTabsButton->setAutoRaise(true);
    mMoreTabsButton->setCursor(Qt::PointingHandCursor);
    mMoreTabsButton->setToolTip(tr("More graphs"));

    QMenu *menu = new QMenu(mMoreTabsButton);
    for (int i = kDefaultVisibleTabs; i < n; ++i) {
        QAction *act = menu->addAction(tw->tabText(i));
        connect(act, &QAction::triggered, this, [this, i] {
            ui->GraphicsTabWidget->setTabVisible(i, true);   // reveal the chosen graph
            ui->GraphicsTabWidget->setCurrentIndex(i);       // and switch to it
        });
    }
    mMoreTabsButton->setMenu(menu);
    tw->setCornerWidget(mMoreTabsButton, Qt::TopRightCorner);

    for (int i = kDefaultVisibleTabs; i < n; ++i)  // collapse overflow into the menu
        tw->setTabVisible(i, false);

    connect(tw, &QTabWidget::currentChanged, this, &MainWindow::onGraphTabChanged);
}

// Keep at most one overflow tab revealed at a time: when the user returns to a
// default tab, collapse any revealed overflow tab back into the "More" menu.
void MainWindow::onGraphTabChanged(int index)
{
    QTabWidget *tw = ui->GraphicsTabWidget;
    const int n = tw->count();
    for (int i = kDefaultVisibleTabs; i < n; ++i)
        if (i != index) tw->setTabVisible(i, false);
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
    QString rateStr  = m.metrics.rate      ? QString::asprintf("%+6.1f", *m.metrics.rate)                          : "------";
    QString beatStr  = m.metrics.beatError ? QString("%1").arg(*m.metrics.beatError, 4, 'f', 1)                    : "----";
    QString ampStr   = m.metrics.amplitude ? QString("%1°").arg(qRound64(*m.metrics.amplitude), 3, 10, QChar(' ')) : "---";
    ui->Results->setText(warning +
                         "POS " + mActivePosition +
                         "  ▏  RATE " + rateStr + " s/d" +
                         "  ▏  AMP " + ampStr +
                         "  ▏  ERR " + beatStr + " ms" +
                         "  ▏  BPH " + bphStr);

    DiagnosisInput diagInput { m.metrics, mWatchType, m.noSignal };
    DiagnosisResult diagResult = mWatchDiagnostics.Evaluate(diagInput);

    // Keep latest input/result for the LLM dialog (AI step 2)
    mLastExplainRequest.input  = diagInput;
    mLastExplainRequest.result = diagResult;


    ui->DiagnosisLabel->setText(diagResult.label);
    ui->DiagnosisLabel->setToolTip(formatDiagnosisTooltip(diagResult, diagInput));
    QColor diagColor = DiagnosisColor(diagResult.level);
    ui->DiagnosisLabel->setStyleSheet(
        QString("background-color: %1; color: white; border-radius: 4px;")
            .arg(diagColor.name()));
    if (diagResult.level != mLastDiagnosisLevel)
    {
        qInfo() << "[WatchDiagnostics]" << diagResult.label
                 << "rate="      << (m.metrics.rate      ? QString::number(*m.metrics.rate,      'f', 1) : "NULL")
                 << "amplitude=" << (m.metrics.amplitude ? QString::number(*m.metrics.amplitude, 'f', 0) : "NULL")
                 << "beatError=" << (m.metrics.beatError ? QString::number(*m.metrics.beatError, 'f', 2) : "NULL");
        mLastDiagnosisLevel = diagResult.level;
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// T2: receive per-frame log data from DSP thread, write CSV on main thread
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onFrameLogged(Logger::Frame frame)
{
    // Wall-clock duration check: exit once --duration seconds have elapsed.
    if (mCmdDurationSec > 0 && mCmdDurationTimer.isValid() &&
        mCmdDurationTimer.elapsed() >= (qint64)mCmdDurationSec * 1000)
    {
        close();
        return;
    }

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
// Reset — reinitialise Presentation layer before a new session starts.
// MeasurementEngine is re-created per-session inside SessionController.
// ─────────────────────────────────────────────────────────────────────────────
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->DiagnosisLabel && event->type() == QEvent::MouseButtonPress) {
        if (mDiagnosisDialog) {
            mDiagnosisDialog->raise();
            mDiagnosisDialog->activateWindow();
            return true;
        }
        mDiagnosisDialog = new DiagnosisDialog(mLastExplainRequest, &mWatchExplainer, this);
        mDiagnosisDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(mDiagnosisDialog, &QObject::destroyed,
                this, [this]() { mDiagnosisDialog = nullptr; });
        mDiagnosisDialog->show();
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

template<typename T>
T* MainWindow::registerTab(T *tab, const QString &label)
{
    ui->GraphicsTabWidget->addTab(tab, label);
    mAllTabs.append(tab);
    return tab;
}

void MainWindow::addObserver(BaseGraphTab *tab)
{
    mAllTabs.append(tab);
}

void MainWindow::resetTabs(void)
{
    qInfo() << "RESET";
    for (BaseGraphTab *tab : mAllTabs)
        if (tab != mSequenceTab) tab->reset();
    DisplayResults(Measurement{});
    mBackgroundLastFPS = 0.0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Session stopped — Playback/Sim EOF or explicit Stop.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onSessionStopped()
{
    SetGuiStopMode();
    int mode = ui->ModeComboBox->currentIndex();
    if (mode == PLAYBACK || mode == SIM) {
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
    auto length = std::size(kManualAutoBPH);
    for (int i = 0; i < (int)length; i++) {
        QString name = (kManualAutoBPH[i] != 0) ? QString::number(kManualAutoBPH[i]) : "Auto BPH";
        ui->BPHComboBox->addItem(name, kManualAutoBPH[i]);
    }
    ui->BPHComboBox->setCurrentIndex(0);
}

void MainWindow::LoadSimBPH(void)
{
    auto length = std::size(kBphValues);
    for (int i = 0; i < (int)length; i++)
        ui->SimBPHComboBox->addItem(QString::number(kBphValues[i]), kBphValues[i]);
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
    if (mNoRecord) return true;   // --no-record CLI flag: skip dialog, answer NO

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
    resetTabs();
    MovementSpec      movement{ kManualAutoBPH[ui->BPHComboBox->currentIndex()], mLiftAngle };
    AcquisitionConfig config  { mCurrentSamplesPerSecond,
                                ui->HighLineEdit->text().toDouble(),
                                mAveragingPeriod };
    QAudioDevice device = ui->InputDeviceComboBox->currentData().value<QAudioDevice>();
    float volume = ui->MicrophoneHorizontalSlider->sliderPosition() / 1000.0f;
    mSession->startLive(movement, config, ui->UseConsetCheckBox->isChecked(), device, volume);
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
    resetTabs();
    MovementSpec      movement{ kManualAutoBPH[ui->BPHComboBox->currentIndex()], mLiftAngle };
    AcquisitionConfig config  { mCurrentSamplesPerSecond,
                                ui->HighLineEdit->text().toDouble(),
                                mAveragingPeriod };
    mSession->startPlayback(movement, config, ui->UseConsetCheckBox->isChecked(),
                            dlg.selectedFiles().constFirst());
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
    cfg.bph                     = kBphValues[ui->SimBPHComboBox->currentIndex()];
    cfg.sample_rate_hz          = mAvalableRates[ui->SampleRatesComboBox->currentIndex()];
    cfg.beat_error_ms           = -ui->SimBeatErrorSpinBox->value();
    cfg.pcm_peak_amplitude      = 0.40f;
    cfg.watch_amplitude_degrees = ui->SimAmplitudeSpinBox->value();
    cfg.lift_angle_degrees      = ui->LiftAngleSpinBox->value();
    cfg.rate_error_s_per_day    = ui->SimErrorRateSpinBox->value();
    if (!RecordSessionCheck()) return;
    GetAudioRate(mRateBeforePlaybackOrSim);
    GetAudioDevice(mDeviceNameBeforePlaybackOrSim);
    if (!SetAudioDevice(PLAYBACK_OR_SIM_PCM)) qInfo() << "SetAudioDevice Failed";
    if (!SetAudioRate(mRateBeforePlaybackOrSim)) qInfo() << "SetAudioRate Failed";
    resetTabs();
    MovementSpec      movement{ kManualAutoBPH[ui->BPHComboBox->currentIndex()], mLiftAngle };
    AcquisitionConfig config  { mCurrentSamplesPerSecond,
                                ui->HighLineEdit->text().toDouble(),
                                mAveragingPeriod };
    mSession->startSim(movement, config, ui->UseConsetCheckBox->isChecked(), cfg);
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
void MainWindow::on_ScopeScaleSpinBox_valueChanged(int value)
{
    mRateScopeTab->setScopeScale(value);
    if (ui->ScopeScaleSlider->value() != value)
        ui->ScopeScaleSlider->setValue(value);
}
void MainWindow::on_ScopeScaleSlider_valueChanged(int value)
{
    if (ui->ScopeScaleSpinBox->value() != value)
        ui->ScopeScaleSpinBox->setValue(value);
}
void MainWindow::on_UseConsetCheckBox_toggled(bool checked)
{
    if (mSession->dspWorker())
        QMetaObject::invokeMethod(mSession->dspWorker(), "setUseOnset",
                                  Qt::QueuedConnection, Q_ARG(bool, checked));
}
void MainWindow::on_AveragingPeriodComboBox_currentIndexChanged(int)
{
    mAveragingPeriod = AveragingPeriodList[ui->AveragingPeriodComboBox->currentIndex()];
}
void MainWindow::on_WatchTypeComboBox_currentIndexChanged(int index)
{
    mWatchType = (index == 1) ? WatchType::Women : WatchType::Men;
    qInfo() << "Watch Type=" << ui->WatchTypeComboBox->currentText();
}
void MainWindow::on_MicrophoneHorizontalSlider_sliderMoved(int)
{
    mSession->setMicVolume(ui->MicrophoneHorizontalSlider->sliderPosition() / 1000.0f);
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
    int mode = ui->ModeComboBox->currentIndex();
    if (mode == LIVE) {
        mSession->stopLiveAudio();
        AudioCloseCheck();
    } else {
        mSession->stop();
        AudioCloseCheck();
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
    updateLeftPanelForMode();   // show/hide SimFrame for this mode + reflow column
}
