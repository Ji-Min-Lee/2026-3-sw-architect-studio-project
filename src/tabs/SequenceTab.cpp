#include "SequenceTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

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
    auto *lay = new QVBoxLayout(this);

    auto *top = new QHBoxLayout;
    mHeaderLabel = new QLabel(this);
    mCaptureButton = new QPushButton("Capture position", this);
    auto *clearButton = new QPushButton("Clear sequence", this);
    top->addWidget(mHeaderLabel, 1);
    top->addWidget(mCaptureButton);
    top->addWidget(clearButton);
    lay->addLayout(top);

    mTable = new QTableWidget(kPositions.size() + 2, 3, this);
    mTable->setHorizontalHeaderLabels({"Rate (s/d)", "Beat (ms)", "Ampl (°)"});
    QStringList rows = kPositions;
    rows << "X (mean)" << "D (max−min)";
    mTable->setVerticalHeaderLabels(rows);
    mTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (int r = 0; r < mTable->rowCount(); r++)
        for (int c = 0; c < 3; c++) {
            auto *it = new QTableWidgetItem("—");
            it->setTextAlignment(Qt::AlignCenter);
            if (r >= kPositions.size()) {
                QFont f = it->font(); f.setBold(true); it->setFont(f);
                it->setBackground(QColor(235, 235, 235));
            }
            mTable->setItem(r, c, it);
        }
    lay->addWidget(mTable, 1);

    mUnbalanceLabel = new QLabel(this);
    lay->addWidget(mUnbalanceLabel);

    connect(mCaptureButton, &QPushButton::clicked, this, &SequenceTab::captureCurrent);
    connect(clearButton, &QPushButton::clicked, this, [this] { reset(); });

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

    auto set = [&](int col, bool valid, double v, int dec) {
        mTable->item(row, col)->setText(valid ? QString::number(v, 'f', dec) : "—");
    };
    set(kColRate, mLatest.rateValid,      mLatest.rateErrorSpd, 1);
    set(kColBeat, mLatest.beatErrorValid, mLatest.beatErrorMs,  1);
    set(kColAmp,  mLatest.amplitudeValid, mLatest.amplitudeDeg, 0);
    recomputeSummary();
}

void SequenceTab::recomputeSummary()
{
    const int xRow = kPositions.size(), dRow = kPositions.size() + 1;
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

    // Vertical vs horizontal rate comparison (balance-wheel unbalance hint)
    auto meanOf = [&](const QStringList &group, bool &ok) {
        double sum = 0; int n = 0;
        for (const QString &p : group) {
            int r = rowOfPosition(p);
            bool conv = false;
            double v = mTable->item(r, kColRate)->text().toDouble(&conv);
            if (conv) { sum += v; n++; }
        }
        ok = n > 0;
        return ok ? sum / n : 0.0;
    };
    bool okV = false, okH = false;
    double v = meanOf(kVertical, okV), h = meanOf(kHorizontal, okH);
    if (okV && okH)
        mUnbalanceLabel->setText(QString("Vertical X̄ %1 s/d  −  Horizontal X̄ %2 s/d  =  "
                                         "<b>Δ %3 s/d</b>  (large Δ suggests balance unbalance "
                                         "or positional fault)")
                                     .arg(v, 0, 'f', 1).arg(h, 0, 'f', 1)
                                     .arg(v - h, 0, 'f', 1));
    else
        mUnbalanceLabel->setText("Capture at least one horizontal (CH/CB) and one vertical "
                                 "(9H/6H/3H/12H) position for the unbalance comparison.");
}

void SequenceTab::reset()
{
    for (int r = 0; r < mTable->rowCount(); r++)
        for (int c = 0; c < 3; c++)
            mTable->item(r, c)->setText("—");
    mHaveLatest = false;
    recomputeSummary();
}
