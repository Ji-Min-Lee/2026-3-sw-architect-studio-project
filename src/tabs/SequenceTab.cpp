#include "SequenceTab.h"
#include "RadarChartTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSplitter>

namespace {
const int kColRate = 0, kColBeat = 1, kColAmp = 2;
const QStringList kPositions = {"CH", "CB", "9H", "6H", "3H", "12H",
                                "CU(R)", "CU(L)", "CD(R)", "CD(L)"};
const QStringList kVertical   = {"9H", "6H", "3H", "12H"};
const QStringList kHorizontal = {"CH", "CB"};
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

    mTable = new QTableWidget(kPositions.size() + 3, 3, leftPane);
    mTable->setHorizontalHeaderLabels({"Rate (s/d)", "Beat (ms)", "Ampl (°)"});
    QStringList rows = kPositions;
    rows << "X (mean)" << "D (max−min)" << "DVH";
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
    mCaptureButton->setEnabled(m.rateValid || m.beatErrorValid || m.amplitudeValid);
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

    for (int c = 0; c < 3; c++) {
        QVector<double> vals;
        for (int r = 0; r < kPositions.size(); r++) {
            bool parseOk = false;
            double cellValue = mTable->item(r, c)->text().toDouble(&parseOk);
            if (parseOk) vals.append(cellValue);
        }
        if (vals.isEmpty()) {
            mTable->item(xRow, c)->setText("—");
            mTable->item(dRow, c)->setText("—");
            continue;
        }
        double sum = 0, minVal = vals[0], maxVal = vals[0];
        for (double val : vals) { sum += val; minVal = qMin(minVal, val); maxVal = qMax(maxVal, val); }
        int dec = (c == kColAmp) ? 0 : 1;
        mTable->item(xRow, c)->setText(QString::number(sum / vals.size(), 'f', dec));
        mTable->item(dRow, c)->setText(QString::number(maxVal - minVal, 'f', dec));
    }

    // DVH: Vertical mean − Horizontal mean (Rate + Ampl only; Beat left blank)
    auto groupMean = [&](const QStringList &group, int col, bool &ok) {
        double sum = 0; int count = 0;
        for (const QString &p : group) {
            int r = rowOfPosition(p);
            bool conv = false;
            double cellValue = mTable->item(r, col)->text().toDouble(&conv);
            if (conv) { sum += cellValue; count++; }
        }
        ok = count > 0;
        return ok ? sum / count : 0.0;
    };
    for (int c = 0; c < 3; c++) {
        if (c == kColBeat) {
            mTable->item(dvhRow, c)->setText("—");
            continue;
        }
        bool okV = false, okH = false;
        double vMean = groupMean(kVertical,   c, okV);
        double hMean = groupMean(kHorizontal, c, okH);
        int dec = (c == kColAmp) ? 0 : 1;
        mTable->item(dvhRow, c)->setText(
            (okV && okH) ? QString::number(vMean - hMean, 'f', dec) : "—");
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
