#include "DiagnosisDialog.h"
#include <QHBoxLayout>
#include <QFont>

DiagnosisDialog::DiagnosisDialog(const ExplainRequest &req,
                                 WatchExplainer       *explainer,
                                 QWidget              *parent)
    : QDialog(parent)
    , m_explainer(explainer)
{
    setWindowTitle(tr("AI Diagnosis Explanation"));
    setMinimumWidth(480);
    setMinimumHeight(280);

    setupUi(req.result);

    // Forward signals from shared explainer to this dialog's slots.
    // Use direct connection so the dialog cleans up properly on close.
    connect(m_explainer, &WatchExplainer::explanationReady,
            this,        &DiagnosisDialog::onExplanationReady,
            Qt::UniqueConnection);
    connect(m_explainer, &WatchExplainer::errorOccurred,
            this,        &DiagnosisDialog::onErrorOccurred,
            Qt::UniqueConnection);

    setLoading(true);
    m_explainer->explain(req);
}

// ── Private slots ─────────────────────────────────────────────────────────────

void DiagnosisDialog::onExplanationReady(const QString &text)
{
    setLoading(false);
    m_explanationEdit->setPlainText(text);
}

void DiagnosisDialog::onErrorOccurred(const QString &error)
{
    setLoading(false);
    m_explanationEdit->setPlainText(
        tr("Could not get AI explanation.\n\n%1\n\n"
           "Make sure Ollama is running:\n"
           "  ollama serve\n"
           "  ollama pull phi3:mini").arg(error));
}

// ── Private helpers ───────────────────────────────────────────────────────────

void DiagnosisDialog::setupUi(const DiagnosisResult &result)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(16, 16, 16, 12);

    // Coloured diagnosis banner
    m_titleLabel = new QLabel(result.label, this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    QColor bg = DiagnosisColor(result.level);
    m_titleLabel->setStyleSheet(
        QString("color: white; background: %1; padding: 6px 10px; border-radius: 4px;")
            .arg(bg.name()));
    layout->addWidget(m_titleLabel);

    // Status / loading label
    m_statusLabel = new QLabel(tr("Asking AI watchmaker…"), this);
    m_statusLabel->setStyleSheet("color: gray; font-style: italic;");
    layout->addWidget(m_statusLabel);

    // Progress bar (indeterminate while waiting)
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);   // indeterminate
    m_progressBar->setFixedHeight(4);
    m_progressBar->setTextVisible(false);
    layout->addWidget(m_progressBar);

    // Explanation text
    m_explanationEdit = new QTextEdit(this);
    m_explanationEdit->setReadOnly(true);
    m_explanationEdit->setFrameStyle(QFrame::NoFrame);
    m_explanationEdit->setStyleSheet("background: transparent;");
    layout->addWidget(m_explanationEdit, 1);

    // Close button
    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    m_closeButton = new QPushButton(tr("Close"), this);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(m_closeButton);
    layout->addLayout(btnRow);
}

void DiagnosisDialog::setLoading(bool loading)
{
    m_progressBar->setVisible(loading);
    m_statusLabel->setVisible(loading);
}
