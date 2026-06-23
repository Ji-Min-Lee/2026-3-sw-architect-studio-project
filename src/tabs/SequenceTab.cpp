#include "SequenceTab.h"
#include "RadarChartTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSplitter>
#include <cmath>

namespace {
const int kColRate = 0, kColBeat = 1, kColAmp = 2;
// 6 standard positions: 2 horizontal (DU/DD) + 4 vertical (CR/CU/CL/CD)
const QStringList kPositions  = {"DU", "DD", "CR", "CU", "CL", "CD"};
const QStringList kVertical   = {"CR", "CU", "CL", "CD"};
const QStringList kHorizontal = {"DU", "DD"};
// Rotation angles for DVm/Φ sinusoidal fit (Crown Right = 0°)
const QMap<QString, double> kPositionAngleDeg = {
    {"CR",  0.0}, {"CU", 90.0}, {"CL", 180.0}, {"CD", 270.0}
};
} // namespace

QStringList SequenceTab::positions() { return kPositions; }

SequenceTab::SequenceTab(QWidget *parent) : BaseGraphTab(parent)
{
    // Left pane: position controls + the sequence table
    auto *leftPane   = new QWidget(this);
    auto *leftLayout = new QVBoxLayout(leftPane);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    auto *top = new QHBoxLayout;
    mHeaderLabel = new QLabel(leftPane);
    mPositionCombo = new QComboBox(leftPane);
    mPositionCombo->addItems(kPositions);
    mCaptureButton = new QPushButton("Capture position", leftPane);
    mCaptureButton->setEnabled(false);
    auto *clearButton = new QPushButton("Clear sequence", leftPane);
    top->addWidget(mHeaderLabel, 1);
    top->addWidget(new QLabel("Position:", leftPane));
    top->addWidget(mPositionCombo);
    top->addWidget(mCaptureButton);
    top->addWidget(clearButton);
    leftLayout->addLayout(top);

    mTable = new QTableWidget(kPositions.size() + 5, 3, leftPane);
    mTable->setHorizontalHeaderLabels({"Rate (s/d)", "Beat (ms)", "Ampl (°)"});
    QStringList rows = kPositions;
    rows << "X (mean)" << "D (max−min)" << "DVH" << "DVm" << "Φ";
    mTable->setVerticalHeaderLabels(rows);
    // Columns stretch to fill the (width-capped) left pane, so the table
    // spans up to the radar divider with no empty gap on its right.
    mTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (int r = 0; r < mTable->rowCount(); r++)
        for (int c = 0; c < 3; c++) {
            auto *it = new QTableWidgetItem("—");
            it->setTextAlignment(Qt::AlignCenter);
            if (r >= kPositions.size()) {
                QFont summaryFont = it->font(); summaryFont.setBold(true); it->setFont(summaryFont);
                it->setBackground(QColor(235, 235, 235));
                it->setForeground(QColor(30, 30, 30));
            }
            mTable->setItem(r, c, it);
        }
    leftLayout->addWidget(mTable, 1);

    // Cap the left pane width so the table fills it edge-to-edge (no empty gap)
    // while the radar still gets the larger, remaining area.
    leftPane->setMaximumWidth(560);

    // Right pane: radar/polar view of the same multi-position data
    mRadar = new RadarChartTab(this, this);
    mRadar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mRadar->setMinimumWidth(420);

    // Split view — radar fills the remaining (larger) area; user-adjustable.
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(leftPane);
    splitter->addWidget(mRadar);
    splitter->setStretchFactor(0, 0);   // table: keep its capped size
    splitter->setStretchFactor(1, 1);   // radar: absorb extra space
    splitter->setSizes({560, 760});
    splitter->setChildrenCollapsible(false);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(splitter);

    connect(mPositionCombo, &QComboBox::currentTextChanged,
            this, [this](const QString &pos) {
                setActivePosition(pos);
                emit positionChanged(pos);
            });
    connect(mCaptureButton, &QPushButton::clicked, this, &SequenceTab::captureCurrent);
    connect(clearButton, &QPushButton::clicked, this, [this] { reset(); });

    // Keep the embedded radar in sync with captures/clears
    connect(this, &SequenceTab::sequenceUpdated, mRadar, &RadarChartTab::rebuild);

    setActivePosition(mActivePosition);
    recomputeSummary();
}

void SequenceTab::setActivePosition(const QString &pos)
{
    mActivePosition = pos;
    mHeaderLabel->setText(QString("Active position: <b>%1</b> — "
                                  "let the reading settle, then capture").arg(pos));
    int row = rowOfPosition(pos);
    if (row >= 0) mTable->selectRow(row);
}

int SequenceTab::rowOfPosition(const QString &pos) const
{
    return kPositions.indexOf(pos);
}

void SequenceTab::onMeasurement(const Measurement &m)
{
    mLatest = m;
    mHaveLatest = true;
    mCaptureButton->setEnabled(m.metrics.rate.has_value() || m.metrics.beatError.has_value() || m.metrics.amplitude.has_value());
}

void SequenceTab::captureCurrent()
{
    if (!mHaveLatest) return;
    int row = rowOfPosition(mActivePosition);
    if (row < 0) return;

    mCapturedAt[row] = QDateTime::currentDateTime();
    QString timestamp = mCapturedAt[row].toString("HH:mm:ss");

    auto setCell = [&](int col, bool valid, double cellValue, int dec) {
        auto *it = mTable->item(row, col);
        it->setText(valid ? QString::number(cellValue, 'f', dec) : "—");
        it->setToolTip("Captured at " + timestamp);
    };
    setCell(kColRate, mLatest.metrics.rate.has_value(),       *mLatest.metrics.rate,      1);
    setCell(kColBeat, mLatest.metrics.beatError.has_value(), *mLatest.metrics.beatError, 1);
    setCell(kColAmp,  mLatest.metrics.amplitude.has_value(), *mLatest.metrics.amplitude, 0);
    recomputeSummary();
    emit sequenceUpdated();
}

QVector<SequenceTab::PositionReading> SequenceTab::capturedReadings() const
{
    QVector<PositionReading> out(kPositions.size());
    for (int r = 0; r < kPositions.size(); r++) {
        bool okR = false, okB = false, okA = false;
        double rate = mTable->item(r, kColRate)->text().toDouble(&okR);
        double beat = mTable->item(r, kColBeat)->text().toDouble(&okB);
        double amp  = mTable->item(r, kColAmp)->text().toDouble(&okA);
        // A position counts as captured if any of its three cells holds a value
        out[r].valid = okR || okB || okA;
        out[r].rate  = okR ? rate : 0.0;
        out[r].beat  = okB ? beat : 0.0;
        out[r].amp   = okA ? amp  : 0.0;
    }
    return out;
}

void SequenceTab::recomputeSummary()
{
    const int xRow   = kPositions.size();
    const int dRow   = kPositions.size() + 1;
    const int dvhRow = kPositions.size() + 2;
    const int dvmRow = kPositions.size() + 3;
    const int phiRow = kPositions.size() + 4;

    // ── X (mean) and D (max-min) across all captured positions ──────────────
    for (int c = 0; c < 3; c++) {
        QVector<double> vals;
        for (int r = 0; r < kPositions.size(); r++) {
            bool ok = false;
            double v = mTable->item(r, c)->text().toDouble(&ok);
            if (ok) vals.append(v);
        }
        if (vals.isEmpty()) {
            mTable->item(xRow, c)->setText("—");
            mTable->item(dRow, c)->setText("—");
            continue;
        }
        double sum = 0, lo = vals[0], hi = vals[0];
        for (double v : vals) { sum += v; lo = qMin(lo, v); hi = qMax(hi, v); }
        int dec = (c == kColAmp) ? 0 : 1;
        mTable->item(xRow, c)->setText(QString::number(sum / vals.size(), 'f', dec));
        mTable->item(dRow, c)->setText(QString::number(hi - lo, 'f', dec));
    }

    // ── DVH: vertical mean − horizontal mean (Rate + Ampl; Beat blank) ──────
    auto groupMean = [&](const QStringList &group, int col, bool &ok) {
        double sum = 0; int count = 0;
        for (const QString &p : group) {
            int r = rowOfPosition(p);
            bool conv = false;
            double v = mTable->item(r, col)->text().toDouble(&conv);
            if (conv) { sum += v; count++; }
        }
        ok = (count > 0);
        return ok ? sum / count : 0.0;
    };
    for (int c = 0; c < 3; c++) {
        if (c == kColBeat) { mTable->item(dvhRow, c)->setText("—"); continue; }
        bool okV = false, okH = false;
        double vMean = groupMean(kVertical,   c, okV);
        double hMean = groupMean(kHorizontal, c, okH);
        int dec = (c == kColAmp) ? 0 : 1;
        mTable->item(dvhRow, c)->setText(
            (okV && okH) ? QString::number(vMean - hMean, 'f', dec) : "—");
    }

    // ── DVm and Φ: sinusoidal fit to 4 vertical positions ───────────────────
    // Model: rate(θ) = R₀ + DVm × cos(θ − Φ)
    //   CR = θ  0°,  CU = θ  90°,  CL = θ 180°,  CD = θ 270°
    // Proof:  rate_CR − rate_CL = 2 DVm cos(Φ)
    //         rate_CU − rate_CD = 2 DVm sin(Φ)
    // DVm normalised to 270° reference amplitude (Witschi convention).
    auto cellVal = [&](const QString &pos, int col, bool &ok) -> double {
        int r = rowOfPosition(pos);
        if (r < 0) { ok = false; return 0.0; }
        return mTable->item(r, col)->text().toDouble(&ok);
    };

    bool okRcr, okRcu, okRcl, okRcd;
    double rCR = cellVal("CR", kColRate, okRcr);
    double rCU = cellVal("CU", kColRate, okRcu);
    double rCL = cellVal("CL", kColRate, okRcl);
    double rCD = cellVal("CD", kColRate, okRcd);

    if (okRcr && okRcu && okRcl && okRcd) {
        double xComp = (rCR - rCL) / 2.0;   // DVm × cos(Φ)
        double yComp = (rCU - rCD) / 2.0;   // DVm × sin(Φ)
        double dvm   = std::sqrt(xComp * xComp + yComp * yComp);

        // Normalise to 270° amplitude when all 4 amplitudes are available
        bool okACR, okACU, okACL, okACD;
        double aCR = cellVal("CR", kColAmp, okACR);
        double aCU = cellVal("CU", kColAmp, okACU);
        double aCL = cellVal("CL", kColAmp, okACL);
        double aCD = cellVal("CD", kColAmp, okACD);
        if (okACR && okACU && okACL && okACD) {
            double avgAmp = (aCR + aCU + aCL + aCD) / 4.0;
            if (avgAmp > 1.0) dvm *= (270.0 / avgAmp);
        }

        double phi = std::atan2(yComp, xComp) * 180.0 / M_PI;
        if (phi < 0.0) phi += 360.0;

        // DVm shown in Rate column (s/d); Φ shown in Ampl column (°)
        mTable->item(dvmRow, kColRate)->setText(QString::number(dvm, 'f', 1));
        mTable->item(dvmRow, kColBeat)->setText("—");
        mTable->item(dvmRow, kColAmp)->setText("—");
        mTable->item(phiRow, kColRate)->setText("—");
        mTable->item(phiRow, kColBeat)->setText("—");
        mTable->item(phiRow, kColAmp)->setText(QString::number(phi, 'f', 0) + "°");
    } else {
        for (int c = 0; c < 3; c++) {
            mTable->item(dvmRow, c)->setText("—");
            mTable->item(phiRow, c)->setText("—");
        }
    }
}

void SequenceTab::reset()
{
    for (int r = 0; r < mTable->rowCount(); r++)
        for (int c = 0; c < 3; c++) {
            mTable->item(r, c)->setText("—");
            mTable->item(r, c)->setToolTip({});
        }
    for (int r = 0; r < kPositions.size(); r++)
        mCapturedAt[r] = QDateTime{};
    mHaveLatest = false;
    recomputeSummary();
    emit sequenceUpdated();
}
