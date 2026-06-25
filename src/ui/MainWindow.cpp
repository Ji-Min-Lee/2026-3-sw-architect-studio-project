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
#include <QKeySequence>
#include <QShortcut>
#include <QDataStream>
#include <QtEndian>
#include <QDebug>
#include <QTextStream>
#include <QtMath>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QFileInfo>
#include <QTimer>
#include <QPointer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QSplitter>
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
    ui->RunStatusLabel->setTextFormat(Qt::RichText);
    ui->RunStatusLabel->setStyleSheet(
        "QLabel { background-color: #f8f9fa; color: #374151;"
        " padding: 3px 8px; border: 1px solid #e5e7eb; border-radius: 3px; }");
    mRunStatusTimer.setInterval(250);
    connect(&mRunStatusTimer, &QTimer::timeout, this, &MainWindow::updateRunStatusLine);
    ui->LiftAngleSpinBox->setValue(mLiftAngle);
    ui->SoundImage->CreateImage();

    // Keyboard shortcuts
    auto *scSpace = new QShortcut(Qt::Key_Space, this);
    connect(scSpace, &QShortcut::activated, this, [this]() {
        if (ui->StartPushButton->isEnabled())
            ui->StartPushButton->click();
        else if (ui->PausePushButton->isEnabled())
            ui->PausePushButton->click();
    });
    auto *scEsc = new QShortcut(Qt::Key_Escape, this);
    connect(scEsc, &QShortcut::activated, this, [this]() {
        if (ui->StopPushButton->isEnabled())
            ui->StopPushButton->click();
    });


    // ←/→: cycle graph tabs
    auto *scLeft = new QShortcut(Qt::Key_Left, this);
    connect(scLeft, &QShortcut::activated, this, [this]() {
        auto *tw = ui->GraphicsTabWidget;
        int next = (tw->currentIndex() - 1 + tw->count()) % tw->count();
        tw->setCurrentIndex(next);
    });
    auto *scRight = new QShortcut(Qt::Key_Right, this);
    connect(scRight, &QShortcut::activated, this, [this]() {
        auto *tw = ui->GraphicsTabWidget;
        tw->setCurrentIndex((tw->currentIndex() + 1) % tw->count());
    });

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
    mVarioTab          = registerTab(new VarioTab(this),          "Vario");
    mSequenceTab       = registerTab(new SequenceTab(this),       "Sequence");
    mBeatNoiseScopeTab = registerTab(new BeatNoiseScopeTab(this), "Beat Scope");
    mBeatErrorTab      = registerTab(new BeatErrorTab(this),      "Beat Error");
    mLongTermTab       = registerTab(new LongTermTab(this),       "Long Term");
    mEscapementTab     = registerTab(new EscapementTab(this),     "Escapement");
    mSpectrogramTab    = registerTab(new SpectrogramTab(this),    "Spectrogram");
    mWaveformCompTab   = registerTab(new WaveformCompTab(this),   "Waveform");
    mSweepScopeTab     = registerTab(new SweepScopeTab(this),     "Sweep");
    mFilterScopeTab    = registerTab(new FilterScopeTab(this),    "Filters");
    mRadarChartTab     = registerTab(new RadarChartTab(mSequenceTab, this), "Radar");
    connect(mSequenceTab, &SequenceTab::sequenceUpdated, mRadarChartTab, &RadarChartTab::rebuild);

    // Tab tooltips: show full rubric name on hover
    auto *tw = ui->GraphicsTabWidget;
    tw->setTabToolTip(tw->indexOf(ui->RateTab),          "Rate / Scope — baseline rate and oscilloscope view");
    tw->setTabToolTip(tw->indexOf(ui->SoundTab),         "Sound Print — acoustic beat waveform image");
    tw->setTabToolTip(tw->indexOf(mTraceTab),            "Trace Display");
    tw->setTabToolTip(tw->indexOf(mVarioTab),            "Rate and Amplitude Stability Over Time");
    tw->setTabToolTip(tw->indexOf(mSequenceTab),         "Multi-Position Sequence Display");
    tw->setTabToolTip(tw->indexOf(mBeatNoiseScopeTab),   "Beat-Noise Scope Display");
    tw->setTabToolTip(tw->indexOf(mBeatErrorTab),        "Beat Error Display and Diagnostic Trace");
    tw->setTabToolTip(tw->indexOf(mLongTermTab),         "Long-Term Performance Graph");
    tw->setTabToolTip(tw->indexOf(mEscapementTab),       "Escapement Analyzer and Marker-Line Display");
    tw->setTabToolTip(tw->indexOf(mSpectrogramTab),      "Time-Frequency Spectrogram Display");
    tw->setTabToolTip(tw->indexOf(mWaveformCompTab),     "Waveform Comparison Display with Timing Markers");
    tw->setTabToolTip(tw->indexOf(mSweepScopeTab),       "Scope Mode with Synchronized Sweep Display");
    tw->setTabToolTip(tw->indexOf(mFilterScopeTab),      "Scope Function with Multiple Filter Views");
    tw->setTabToolTip(tw->indexOf(mRadarChartTab),       "Radar Chart — multi-position watch health overview (bonus)");

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
    updateRunStatusLine();         // ● Ready · 00:00

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

    // ── Graph tabs: overflow + user guide behind corner "More" drop-down ───
    setupTabOverflow();
    menuBar()->setVisible(false);

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
    const bool liveMode = (ui->ModeComboBox->currentText() == ModeStrings[LIVE]);
    const bool simMode  = (ui->ModeComboBox->currentText() == ModeStrings[SIM]);
    // Gain, InputDevice, Refresh are Live-only — disable (not hide) in other modes
    ui->GainLabel->setEnabled(liveMode);
    ui->MicrophoneHorizontalSlider->setEnabled(liveMode);
    ui->InputDeviceComboBox->setEnabled(liveMode);
    ui->RefreshPushButton->setEnabled(liveMode);
    ui->SimFrame->setVisible(simMode);
    relayoutLeftColumn();
}

// -- Graph-tab corner button (feature/ui-improvement) --------------------------
// "More v" button in the tab-bar corner opens:
//   - "Configure Tabs" dialog: checklist of all tabs; checked = visible
//   - "User Guide" (F1)
//   - "AI Diagnosis" (F2)
// Default visibility: Rate/Scope, Sound Print, Trace, Vario (first 4 tabs).
void MainWindow::setupTabOverflow(void)
{
    QTabWidget *tw = ui->GraphicsTabWidget;
    const int n = tw->count();

    // Move Radar to the absolute end (bonus area — shown last in demo)
    tw->tabBar()->moveTab(tw->indexOf(mRadarChartTab), n - 1);

    // Default visibility: Rate/Scope, Sound Print, Trace, Vario
    const QSet<QWidget *> defaultVisible = { ui->RateTab, ui->SoundTab, mTraceTab, mVarioTab };
    for (int i = 0; i < n; ++i)
        tw->setTabVisible(i, defaultVisible.contains(tw->widget(i)));
    tw->setCurrentWidget(ui->RateTab);  // start on Rate/Scope

    // ── More button: tab list + Configure only ──────────────────────────────
    mMoreTabsButton = new QToolButton;
    mMoreTabsButton->setObjectName("MoreTabsButton");
    mMoreTabsButton->setText(tr("More"));
    mMoreTabsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mMoreTabsButton->setArrowType(Qt::DownArrow);
    mMoreTabsButton->setAutoRaise(true);
    mMoreTabsButton->setCursor(Qt::PointingHandCursor);
    mMoreTabsButton->setMinimumWidth(72);
    mMoreTabsButton->setToolTip(tr("Show hidden tabs / manage tabs"));

    mMoreTabsMenu = new QMenu(mMoreTabsButton);

    // Ctrl+\ Split View
    mSplitAct = mMoreTabsMenu->addAction(tr("Split View"));
    mSplitAct->setShortcut(QKeySequence(Qt::Key_Backslash | Qt::CTRL));
    mSplitAct->setShortcutContext(Qt::ApplicationShortcut);
    mSplitAct->setCheckable(true);
    addAction(mSplitAct);
    connect(mSplitAct, &QAction::triggered, this, &MainWindow::toggleSplitView);

    // Ctrl+T Manage Tabs
    QAction *configureTabs = mMoreTabsMenu->addAction(tr("Manage Tabs..."));
    configureTabs->setShortcut(QKeySequence(Qt::Key_T | Qt::CTRL));
    configureTabs->setShortcutContext(Qt::ApplicationShortcut);
    addAction(configureTabs);
    connect(configureTabs, &QAction::triggered, this, &MainWindow::showTabConfigDialog);

    mMoreTabsMenu->addSeparator();

    // F1 User Guide
    QAction *guide = mMoreTabsMenu->addAction(tr("User Guide"));
    guide->setShortcut(QKeySequence::HelpContents);
    guide->setShortcutContext(Qt::ApplicationShortcut);
    addAction(guide);
    connect(guide, &QAction::triggered, this, [this] {
        showUserGuide(UserGuideSection::Overview);
    });

    // Ctrl+D AI Diagnosis
    QAction *diagAct = mMoreTabsMenu->addAction(tr("AI Diagnosis"));
    diagAct->setShortcut(QKeySequence(Qt::Key_D | Qt::CTRL));
    diagAct->setShortcutContext(Qt::ApplicationShortcut);
    addAction(diagAct);
    connect(diagAct, &QAction::triggered, this, &MainWindow::showDiagnosisDialog);

    mMoreTabsMenu->addSeparator();

    // F11 Fullscreen
    QAction *fsAct = mMoreTabsMenu->addAction(tr("Fullscreen"));
    fsAct->setShortcut(QKeySequence(Qt::Key_F11));
    fsAct->setShortcutContext(Qt::ApplicationShortcut);
    fsAct->setCheckable(true);
    addAction(fsAct);
    connect(fsAct, &QAction::triggered, this, [this, fsAct]() {
        if (isFullScreen()) {
            showNormal();
            setContentsMargins(0, 0, 0, 0);
            fsAct->setChecked(false);
        } else {
            showFullScreen();
            setContentsMargins(10, 10, 0, 10);
            fsAct->setChecked(true);
        }
    });

    mMoreTabsMenu->addSeparator();

    // About
    QAction *aboutAct = mMoreTabsMenu->addAction(tr("About TimeGrapher..."));
    connect(aboutAct, &QAction::triggered, this, [this]() {
        QDialog dlg(this);
        dlg.setWindowTitle(tr("About TimeGrapher"));
        dlg.setFixedWidth(480);

        auto *vlay = new QVBoxLayout(&dlg);
        vlay->setSpacing(12);
        vlay->setContentsMargins(20, 20, 20, 20);

        QPixmap photo(QStringLiteral(":/images/team_ghibli.png"));
        if (!photo.isNull()) {
            auto *photoLabel = new QLabel;
            photoLabel->setPixmap(photo.scaledToWidth(440, Qt::SmoothTransformation));
            photoLabel->setAlignment(Qt::AlignCenter);
            vlay->addWidget(photoLabel);
        }

        auto *textLabel = new QLabel(
            tr("<h3>TimeGrapher</h3>"
               "<p>Version 1.0.0</p>"
               "<p>Mechanical Watch Timing Analyzer<br>"
               "LG SW Architect Training Program 2026<br>"
               "Team 3 &middot; Blue Sky</p>"
               "<hr>"
               "<p style='color:gray; font-size:10pt;'>"
               "&copy; 2026 LG Electronics &middot; Internal Use Only</p>"));
        textLabel->setAlignment(Qt::AlignCenter);
        textLabel->setWordWrap(true);
        vlay->addWidget(textLabel);

        auto *btn = new QPushButton(tr("OK"));
        btn->setDefault(true);
        connect(btn, &QPushButton::clicked, &dlg, &QDialog::accept);
        vlay->addWidget(btn, 0, Qt::AlignCenter);

        dlg.exec();
    });

    connect(mMoreTabsButton, &QToolButton::clicked, this, [this] {
        if (!mMoreTabsMenu) return;
        const QPoint pos = mMoreTabsButton->mapToGlobal(
            QPoint(0, mMoreTabsButton->height()));
        mMoreTabsMenu->popup(pos);
    });

    tw->setCornerWidget(mMoreTabsButton, Qt::TopRightCorner);

    auto *hintLabel = new QLabel(
        "  Space: Start/Pause   Esc: Stop   ←/→: Tabs   F11: Fullscreen   |   F1: Guide   Ctrl+T: Tabs   Ctrl+\\: Split   Ctrl+D: AI  ", this);
    hintLabel->setStyleSheet("color: gray; font-size: 11px;");
    statusBar()->addPermanentWidget(hintLabel);
}

// Singleton user guide dialog: raise existing window instead of opening a new one.
void MainWindow::showUserGuide(UserGuideSection section)
{
    if (mUserGuideDialog) {
        mUserGuideDialog->raise();
        mUserGuideDialog->activateWindow();
        return;
    }
    mUserGuideDialog = new UserGuideDialog(section, this);
    mUserGuideDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(mUserGuideDialog, &QObject::destroyed,
            this, [this]() { mUserGuideDialog = nullptr; });
    mUserGuideDialog->show();
    mUserGuideDialog->raise();
    mUserGuideDialog->activateWindow();
}

// Singleton AI diagnosis dialog (same singleton pattern as UserGuideDialog).
void MainWindow::showDiagnosisDialog(void)
{
    if (mDiagnosisDialog) {
        mDiagnosisDialog->raise();
        mDiagnosisDialog->activateWindow();
        return;
    }
    mDiagnosisDialog = new DiagnosisDialog(mLastExplainRequest, &mWatchExplainer, this);
    mDiagnosisDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(mDiagnosisDialog, &QObject::destroyed,
            this, [this]() { mDiagnosisDialog = nullptr; });
    mDiagnosisDialog->show();
}

// Tab configuration dialog: checklist of all tabs; checked = visible.
void MainWindow::showTabConfigDialog(void)
{
    // In split view one tab is detached into the right pane, so it would be
    // invisible to this dialog. Collapse the split first so the dialog can see
    // and manage the full tab set consistently.
    if (mSplitMode) exitSplitView();

    QTabWidget *tw = ui->GraphicsTabWidget;
    const int n = tw->count();

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Configure Tabs"));
    dlg.setMinimumWidth(320);

    auto *layout = new QVBoxLayout(&dlg);
    layout->addWidget(new QLabel(tr("Select tabs to display:"), &dlg));

    QVector<QCheckBox *> boxes;
    for (int i = 0; i < n; ++i) {
        auto *cb = new QCheckBox(tw->tabText(i), &dlg);
        cb->setChecked(tw->isTabVisible(i));
        const QString tip = tw->tabToolTip(i);
        if (!tip.isEmpty()) cb->setToolTip(tip);
        layout->addWidget(cb);
        boxes.append(cb);
    }

    // Select All / Deselect All
    auto *selRow = new QHBoxLayout();
    auto *selAll   = new QPushButton(tr("Select All"),   &dlg);
    auto *deselAll = new QPushButton(tr("Deselect All"), &dlg);
    selRow->addWidget(selAll);
    selRow->addWidget(deselAll);
    selRow->addStretch();
    layout->addLayout(selRow);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);

    // OK enabled only when at least one tab is checked
    auto *okBtn = buttons->button(QDialogButtonBox::Ok);
    auto updateOk = [&boxes, okBtn] {
        bool any = std::any_of(boxes.begin(), boxes.end(),
                               [](QCheckBox *cb){ return cb->isChecked(); });
        okBtn->setEnabled(any);
    };
    for (auto *cb : boxes)
        // QCheckBox::checkStateChanged was added in Qt 6.7; fall back to
        // stateChanged on older Qt (updateOk takes no args, so both connect).
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        connect(cb, &QCheckBox::checkStateChanged, &dlg, updateOk);
#else
        connect(cb, &QCheckBox::stateChanged, &dlg, updateOk);
#endif
    connect(selAll,   &QPushButton::clicked, &dlg, [&boxes, updateOk]{ for (auto *cb : boxes) cb->setChecked(true);  updateOk(); });
    connect(deselAll, &QPushButton::clicked, &dlg, [&boxes, updateOk]{ for (auto *cb : boxes) cb->setChecked(false); updateOk(); });
    updateOk();  // set initial state

    if (dlg.exec() != QDialog::Accepted) return;

    int currentIdx = tw->currentIndex();
    bool currentWillHide = !boxes[currentIdx]->isChecked();

    for (int i = 0; i < n; ++i)
        tw->setTabVisible(i, boxes[i]->isChecked());

    // If the active tab was unchecked, switch to the first visible tab
    if (currentWillHide) {
        for (int i = 0; i < n; ++i) {
            if (tw->isTabVisible(i)) { tw->setCurrentIndex(i); break; }
        }
    }

    refreshSplitCombo();
}

void MainWindow::refreshSplitCombo()
{
    if (!mSplitMode || !mSplitCombo) return;
    auto *tw = ui->GraphicsTabWidget;

    mSplitCombo->blockSignals(true);
    mSplitCombo->clear();

    // Add all currently visible tabs in GraphicsTabWidget
    for (int i = 0; i < tw->count(); ++i)
        if (tw->isTabVisible(i))
            mSplitCombo->addItem(tw->tabText(i), QVariant::fromValue(tw->widget(i)));

    // The detached widget is not in tw — add it too so user can still select it
    if (mSplitCurrentWidget)
        mSplitCombo->addItem(mSplitOriginalName, QVariant::fromValue(mSplitCurrentWidget));

    // Restore selection to the currently shown widget
    int restoreIdx = mSplitCombo->count() - 1;  // defaults to last (detached)
    if (mSplitCurrentWidget) {
        for (int i = 0; i < mSplitCombo->count(); ++i) {
            if (mSplitCombo->itemData(i).value<QWidget *>() == mSplitCurrentWidget) {
                restoreIdx = i;
                break;
            }
        }
    }
    mSplitCombo->setCurrentIndex(restoreIdx);
    mSplitCombo->blockSignals(false);
}

// ─────────────────────────────────────────────────────────────────────────────
// Controller slot: update Results label from Measurement
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onMeasurementReady(const Measurement &m)
{
    mLastReplotCount = g_replotCount.exchange(0);
    mLastPlotUs      = g_plotUs.exchange(0);
    if (m.synced) ++mSyncedCount;
    checkWatchDetached(m);  // update detached state before formatting the label
    checkNoise(m);          // all modes: ambient-noise popup
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

// All modes: when the ambient noise level stays at/above the threshold the
// watch cannot be measured reliably → raise a popup. Dismiss once the noise
// has subsided AND normal measurement has resumed. Both edges are debounced
// (sustained for kNoiseOn/OffMs) so transient bumps don't flicker the popup.
void MainWindow::checkNoise(const Measurement &m)
{
    // Simulation has no real ambient noise — skip so the alarm never fires in sim.
    if (ui->ModeComboBox->currentText() == ModeStrings[SIM]) return;

    if (!ui->StopPushButton->isEnabled()) {   // not running → clear everything
        if (mNoiseAlarm) mNoiseAlarm->hide();
        mNoiseShown = false;
        mNoiseAboveSince.invalidate();
        mNoiseBelowSince.invalidate();
        return;
    }

    if (m.noiseDb >= kNoiseThresholdDb) {                 // noisy
        mNoiseBelowSince.invalidate();
        if (!mNoiseAboveSince.isValid()) mNoiseAboveSince.restart();
        if (!mNoiseShown && mNoiseAboveSince.elapsed() >= kNoiseOnMs) {
            mNoiseShown = true;
            raiseNoiseAlarm();
        }
    } else {                                              // quiet enough
        mNoiseAboveSince.invalidate();
        const bool measuring = m.synced || m.metrics.rate.has_value()
                                        || m.metrics.amplitude.has_value();
        if (!mNoiseBelowSince.isValid()) mNoiseBelowSince.restart();
        if (mNoiseShown && measuring && mNoiseBelowSince.elapsed() >= kNoiseOffMs) {
            mNoiseShown = false;
            if (mNoiseAlarm) mNoiseAlarm->hide();
        }
    }
}

void MainWindow::raiseNoiseAlarm(void)
{
    statusBar()->showMessage("High ambient noise", 5000);
    if (!mNoiseAlarm) {
        mNoiseAlarm = new QMessageBox(this);
        mNoiseAlarm->setIcon(QMessageBox::Warning);
        mNoiseAlarm->setWindowTitle("Noisy Environment");
        mNoiseAlarm->setText("High ambient noise detected");
        mNoiseAlarm->setInformativeText(
            "The surrounding noise is too high to measure the watch reliably. "
            "Please move to a quieter location.");
        mNoiseAlarm->setStandardButtons(QMessageBox::Ok);
        mNoiseAlarm->setModal(false);  // non-blocking: keep measuring underneath
    }
    if (!mNoiseAlarm->isVisible()) {
        mNoiseAlarm->show();
        mNoiseAlarm->raise();
        mNoiseAlarm->activateWindow();
    }
}

void MainWindow::DisplayResults(const Measurement &m)
{
    // View: format values for display — no domain logic here
    // Evaluate diagnosis first so per-axis breakdown colors are available for the header.
    DiagnosisInput diagInput { m.metrics, mWatchType, m.noSignal };
    DiagnosisResult diagResult = mWatchDiagnostics.Evaluate(diagInput);

    QString warning  = mWatchDetached ? "⚠ Watch detached   "
                     : m.noSignal      ? "⚠ No signal   " : "";
    QString bphStr   = m.synced ? QString("%1").arg(m.detectedBph, 5, 10, QChar(' ')) : "-----";
    QString rateStr  = m.metrics.rate      ? QString::asprintf("%+6.1f", *m.metrics.rate)                          : "------";
    QString beatStr  = m.metrics.beatError ? QString("%1").arg(*m.metrics.beatError, 4, 'f', 1)                    : "----";
    QString ampStr   = m.metrics.amplitude ? QString("%1°").arg(qRound64(*m.metrics.amplitude), 3, 10, QChar(' ')) : "---";
    // Highlight values: normal color per metric; alert red if out of range
    auto colored = [](const QString &val, const QString &hex) {
        return QString("<span style='color:%1'>%2</span>").arg(hex, val);
    };
    // Use the same 3-level colors as the diagnosis badge: green/amber/red per axis.
    // AxisStatus already encodes the thresholds — reuse it directly.
    auto axisColor = [](AxisStatus s) -> QString {
        switch (s) {
            case AxisStatus::PassExcellent: return QStringLiteral("#2ea04d");
            case AxisStatus::PassGood:      return QStringLiteral("#e0a000");
            case AxisStatus::Fail:          return QStringLiteral("#c03030");
            default:                        return QStringLiteral("#888888"); // Unknown
        }
    };

    const QString sep   = "<span style='color:#888'>  |  </span>";
    const QString cLbl  = "#444444", cBph = "#5B2D90", cAlert = "#C50F1F";

    QString html = "<span style='font-size:12pt; font-weight:bold;'>";
    html += warning.isEmpty() ? "" : colored(warning, cAlert);
    html += colored("POS ", cLbl) + colored(mActivePosition, cBph);
    html += sep + colored("RATE ", cLbl) + colored(rateStr + " s/d", axisColor(diagResult.breakdown.rate));
    html += sep + colored("AMP ",  cLbl) + colored(ampStr,           axisColor(diagResult.breakdown.amplitude));
    html += sep + colored("ERR ",  cLbl) + colored(beatStr + " ms",  axisColor(diagResult.breakdown.beatError));
    html += sep + colored("BPH ",  cLbl) + colored(bphStr,           cBph);
    html += "</span>";
    ui->Results->setText(html);

    // Keep latest input/result for the LLM dialog (AI step 2)
    mLastExplainRequest.input  = diagInput;
    mLastExplainRequest.result = diagResult;


    ui->DiagnosisLabel->setText(diagResult.label);
    ui->DiagnosisLabel->setToolTip(formatDiagnosisTooltip(diagResult, diagInput));
    QColor diagColor = DiagnosisColor(diagResult.level);
    ui->DiagnosisLabel->setStyleSheet(
        QString("background-color: %1; color: white; border-radius: 4px;")
            .arg(diagColor.name()));

    if (mSessionState == SessionState::Warming &&
        diagResult.level != DiagnosisLevel::Unknown)
        mSessionState = SessionState::Running;
    updateRunStatusLine();

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

    // fg_fps is 0 on most frames (only updated every 2 s in DSPWorker);
    // ignore zero updates so the last real value stays on screen.
    const bool dspUpdated = frame.fg_fps > 0 && mDspLastFPS != frame.fg_fps;
    if (mBackgroundLastFPS != frame.bg_fps ||
        mBackgroundLastSPS != frame.bg_sps ||
        dspUpdated)
    {
        mBackgroundLastFPS = frame.bg_fps;
        mBackgroundLastSPS = frame.bg_sps;
        mBackgroundLastSPF = frame.bg_spf;
        if (dspUpdated) {
            mDspLastFPS = frame.fg_fps;
            mDspLastSPS = frame.fg_sps;
            mDspLastSPF = frame.fg_spf;
        }
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
        showDiagnosisDialog();
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
    mSyncedCount = 0;
    DisplayResults(Measurement{});
    mBackgroundLastFPS = 0.0;
}

namespace {

struct RunStatusPresentation {
    QString stateText;
    QColor  accent;
    QString tooltip;
};

RunStatusPresentation computeRunStatusPresentation(SessionState state,
                                                   int syncedCount,
                                                   int averagingPeriod,
                                                   const DiagnosisResult &diag)
{
    RunStatusPresentation p;
    const int prog = qMin(syncedCount, averagingPeriod);
    const QString acquiring = QObject::tr("Acquiring %1/%2").arg(prog).arg(averagingPeriod);

    switch (state) {
    case SessionState::Idle:
        p.stateText = QObject::tr("Ready");
        p.accent    = QColor("#6b7280");
        p.tooltip   = QObject::tr("Press Start to begin measuring.");
        break;

    case SessionState::Warming:
        p.stateText = acquiring;
        p.accent    = QColor("#1a6bbf");
        p.tooltip   = QObject::tr(
            "Collecting synced beats — watch diagnosis appears after %1 beats.")
                          .arg(averagingPeriod);
        break;

    case SessionState::Running:
        if (diag.level != DiagnosisLevel::Unknown) {
            p.stateText = QObject::tr("Running");
            p.accent    = QColor("#15803d");
            p.tooltip   = QString();
        } else {
            p.stateText = acquiring;
            p.accent    = QColor("#1a6bbf");
            p.tooltip   = QObject::tr("Still collecting data for a diagnosis.");
        }
        break;

    case SessionState::Paused:
        p.stateText = QObject::tr("Paused");
        p.accent    = QColor("#b45309");
        p.tooltip   = QObject::tr("Graphs frozen for diagnostic review — click Resume to continue.");
        break;
    }
    return p;
}

} // namespace

static QString formatSessionClock(qint64 ms)
{
    const int totalSec = static_cast<int>(ms / 1000);
    const int h = totalSec / 3600;
    const int m = (totalSec % 3600) / 60;
    const int s = totalSec % 60;
    if (h > 0)
        return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

qint64 MainWindow::sessionElapsedMs() const
{
    qint64 ms = mSessionActiveMs;
    if (mSessionTimer.isValid())
        ms += mSessionTimer.elapsed();
    return ms;
}

void MainWindow::updateRunStatusLine()
{
    const RunStatusPresentation p = computeRunStatusPresentation(
        mSessionState, mSyncedCount, mAveragingPeriod, mLastExplainRequest.result);

    const QString clock = formatSessionClock(sessionElapsedMs());
    const QString hex   = p.accent.name();
    ui->RunStatusLabel->setText(
        QString("<span style='color:%1; font-weight:600'>●</span>"
                "&nbsp;<span style='color:%1'>%2</span>"
                "<span style='color:#d1d5db'>&nbsp;·&nbsp;</span>"
                "<span style='font-family:Consolas,monospace; color:#4b5563'>%3</span>")
            .arg(hex, p.stateText.toHtmlEscaped(), clock.toHtmlEscaped()));

    if (p.tooltip.isEmpty())
        ui->RunStatusLabel->setToolTip(ui->DiagnosisLabel->toolTip());
    else
        ui->RunStatusLabel->setToolTip(p.tooltip);
}

void MainWindow::startSessionClock()
{
    mSessionActiveMs = 0;
    mSessionTimer.start();
    mRunStatusTimer.start();
    updateRunStatusLine();
}

void MainWindow::stopSessionClock()
{
    mSessionActiveMs = 0;
    mSessionTimer.invalidate();
    mRunStatusTimer.stop();
    updateRunStatusLine();
}

// ─────────────────────────────────────────────────────────────────────────────
// Session stopped — Playback/Sim EOF or explicit Stop.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onSessionStopped()
{
    mSessionState = SessionState::Idle;
    stopSessionClock();
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
    if (mNoRecord || !ui->RecordCheckBox->isChecked()) return true;

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
    if (on) {
        if (mSessionTimer.isValid()) {
            mSessionActiveMs += mSessionTimer.elapsed();
            mSessionTimer.invalidate();
        }
        mSessionState = SessionState::Paused;
    } else if (mSessionState == SessionState::Paused) {
        mSessionState = (mLastExplainRequest.result.level != DiagnosisLevel::Unknown)
                            ? SessionState::Running : SessionState::Warming;
        mSessionTimer.start();
    }
    updateRunStatusLine();
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
    ui->RecordCheckBox->setEnabled(false);
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
    ui->RecordCheckBox->setEnabled(true);
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
    mSessionState = SessionState::Warming;
    startSessionClock();
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
    mSessionState = SessionState::Warming;
    startSessionClock();
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
    mSessionState = SessionState::Warming;
    startSessionClock();
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
    mSessionState = SessionState::Idle;
    stopSessionClock();
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

// ─────────────────────────────────────────────────────────────────────────────
// Split view (F3) — moves one tab widget to a right panel; same instance = live
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::toggleSplitView()
{
    mSplitMode ? exitSplitView() : enterSplitView();
}

void MainWindow::enterSplitView()
{
    auto *tw = ui->GraphicsTabWidget;

    // Count visible tabs; need at least 2 to make split meaningful
    int visibleCount = 0;
    for (int i = 0; i < tw->count(); ++i)
        if (tw->isTabVisible(i)) ++visibleCount;
    if (visibleCount < 2) {
        statusBar()->showMessage(tr("Split view needs at least 2 visible tabs (F2 to add tabs)"), 3000);
        return;
    }

    mSplitMode = true;
    if (mSplitAct) mSplitAct->setChecked(true);

    // Create splitter in the graph area
    const int totalW = tw->width();   // capture before reparent invalidates it
    mSplitter = new QSplitter(Qt::Horizontal, ui->CentralWidget);
    mSplitter->setGeometry(tw->geometry());
    // Move GraphicsTabWidget into left side of splitter
    tw->setParent(mSplitter);
    mSplitter->addWidget(tw);

    // Right panel: dropdown + content area
    auto *rightPanel = new QWidget(mSplitter);
    auto *vlay = new QVBoxLayout(rightPanel);
    vlay->setContentsMargins(2, 2, 2, 2);
    vlay->setSpacing(4);

    mSplitCombo = new QComboBox(rightPanel);
    mSplitCombo->setFont(QFont(font().family(), 10, QFont::Bold));
    for (int i = 0; i < tw->count(); ++i)
        if (tw->isTabVisible(i))
            mSplitCombo->addItem(tw->tabText(i), QVariant::fromValue(tw->widget(i)));
    vlay->addWidget(mSplitCombo, 0);

    mSplitContentArea = new QWidget(rightPanel);
    mSplitContentLayout = new QVBoxLayout(mSplitContentArea);
    mSplitContentLayout->setContentsMargins(0, 0, 0, 0);
    vlay->addWidget(mSplitContentArea, 1);

    mSplitter->addWidget(rightPanel);

    // Force both panes to allow shrinking below their content's minimumSizeHint.
    // NOTE: setMinimumWidth(0) does NOT do this — Qt treats 0 as "defer to
    // minimumSizeHint", so the tab widget's large hint would keep skewing the
    // split. A small *non-zero* explicit minimum actually overrides the hint.
    tw->setMinimumWidth(50);
    rightPanel->setMinimumWidth(50);
    mSplitter->setChildrenCollapsible(false);
    mSplitter->setStretchFactor(0, 1);
    mSplitter->setStretchFactor(1, 1);

    mSplitter->show();

    connect(mSplitCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::setSplitViewTab);

    // Default right panel: the tab AFTER the currently active one
    int curTabIdx = tw->currentIndex();
    int defaultComboIdx = 0;
    for (int i = 0; i < mSplitCombo->count(); ++i) {
        QWidget *w = mSplitCombo->itemData(i).value<QWidget *>();
        int idx = tw->indexOf(w);
        if (idx > curTabIdx) { defaultComboIdx = i; break; }
        if (i == mSplitCombo->count() - 1) defaultComboIdx = 0; // wrap: pick first if current is last
    }
    mSplitCombo->setCurrentIndex(defaultComboIdx);
    setSplitViewTab(defaultComboIdx);

    // Apply the 50/50 ratio LAST — after the heavy tab is moved into the right
    // pane, so its large minimumSizeHint can't re-skew the split afterwards.
    // The ratio (not absolute px) is what matters; equal values => 50/50.
    mSplitter->setSizes({totalW / 2, totalW / 2});

    // Re-apply once more after the pending layout pass settles, otherwise a
    // queued relayout can override the ratio right after this function returns.
    QPointer<QSplitter> sp = mSplitter;
    QTimer::singleShot(0, this, [sp] {
        if (sp) { const int h = sp->width() / 2; sp->setSizes({h, h}); }
    });
}

void MainWindow::setSplitViewTab(int comboIdx)
{
    auto *tw = ui->GraphicsTabWidget;

    // Restore current right-panel widget back to GraphicsTabWidget
    if (mSplitCurrentWidget) {
        mSplitContentLayout->removeWidget(mSplitCurrentWidget);
        mSplitCurrentWidget->setParent(nullptr);
        tw->insertTab(mSplitOriginalIdx, mSplitCurrentWidget, mSplitOriginalName);
        tw->setTabVisible(mSplitOriginalIdx, mSplitOriginalVisible);
        mSplitCurrentWidget = nullptr;
    }

    if (comboIdx < 0 || comboIdx >= mSplitCombo->count()) return;

    QWidget *w = mSplitCombo->itemData(comboIdx).value<QWidget *>();
    if (!w) return;

    int idx = tw->indexOf(w);
    if (idx < 0) return;

    mSplitOriginalIdx     = idx;
    mSplitOriginalName    = tw->tabText(idx);
    mSplitOriginalVisible = tw->isTabVisible(idx);
    mSplitCurrentWidget   = w;

    tw->removeTab(idx);
    w->setParent(mSplitContentArea);
    mSplitContentLayout->addWidget(w);
    w->show();
}

void MainWindow::exitSplitView()
{
    // Restore right-panel widget
    if (mSplitCurrentWidget) {
        mSplitContentLayout->removeWidget(mSplitCurrentWidget);
        mSplitCurrentWidget->setParent(nullptr);
        ui->GraphicsTabWidget->insertTab(mSplitOriginalIdx,
            mSplitCurrentWidget, mSplitOriginalName);
        ui->GraphicsTabWidget->setTabVisible(mSplitOriginalIdx, mSplitOriginalVisible);
        mSplitCurrentWidget = nullptr;
    }

    // Restore GraphicsTabWidget to CentralWidget
    QRect geo = mSplitter->geometry();
    ui->GraphicsTabWidget->setParent(ui->CentralWidget);
    ui->GraphicsTabWidget->setGeometry(geo);
    ui->GraphicsTabWidget->show();

    delete mSplitter;       // also deletes rightPanel and its children
    mSplitter           = nullptr;
    mSplitCombo         = nullptr;
    mSplitContentArea   = nullptr;
    mSplitContentLayout = nullptr;
    mSplitMode          = false;
    if (mSplitAct) mSplitAct->setChecked(false);
}
