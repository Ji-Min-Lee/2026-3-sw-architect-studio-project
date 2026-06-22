#include "DiagnosisDialog.h"
#include <QHBoxLayout>
#include <QFont>
#include <QTextCursor>
#include <QTextDocument>

namespace {
// QTextEdit renders HTML but not Markdown live, so convert the model's
// markdown to an HTML fragment (inner <body>) we can place in a chat layout.
QString markdownToHtmlBody(const QString &md)
{
    QTextDocument doc;
    doc.setMarkdown(md);
    const QString html = doc.toHtml();
    int s = html.indexOf("<body");
    if (s < 0) return md.toHtmlEscaped();
    s = html.indexOf('>', s) + 1;
    const int e = html.lastIndexOf("</body>");
    return html.mid(s, e - s);
}

// AI turn block: label + rendered response (no trailing separator).
QString aiBlock(const QString &md)
{
    return QStringLiteral("<p><b>🧠 Diagnostic AI</b></p>") + markdownToHtmlBody(md);
}
}

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
    connect(m_explainer, &WatchExplainer::tokenReceived,
            this, [this](const QString &token) {
                m_streamBuf += token;
                renderConversation();
            });
    connect(m_explainer, &WatchExplainer::explanationReady,
            this,        &DiagnosisDialog::onExplanationReady);
    connect(m_explainer, &WatchExplainer::errorOccurred,
            this,        &DiagnosisDialog::onErrorOccurred);
    connect(m_explainer, &WatchExplainer::ragStatusChanged,
            this, [this](bool active, int chunks) {
                if (active)
                    m_ragLabel->setText(tr("📚 RAG: %1 chunks from Witschi docs").arg(chunks));
                else
                    m_ragLabel->setText(tr("RAG: no context (vector.db not found)"));
                m_ragLabel->setVisible(true);
            });

    setLoading(true);
    m_explainer->explain(req);
}

// ── Private slots ─────────────────────────────────────────────────────────────

void DiagnosisDialog::onExplanationReady(const QString &)
{
    // Commit the streamed response as a finished turn.
    m_conversationHtml += aiBlock(m_streamBuf.trimmed()) + "<hr/>";
    m_streamBuf.clear();
    renderConversation();

    setLoading(false);
    m_inputEdit->setEnabled(true);
    m_sendButton->setEnabled(true);
    m_inputEdit->setFocus();
}

void DiagnosisDialog::onSendClicked()
{
    const QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty()) return;

    m_inputEdit->clear();
    m_inputEdit->setEnabled(false);
    m_sendButton->setEnabled(false);

    m_conversationHtml += QString(
        "<p align=\"right\">🙋 <b>You</b><br/>"
        "<span style=\"color:#1565c0;\">%1</span></p>").arg(text.toHtmlEscaped());
    renderConversation();

    setLoading(true);
    m_explainer->chat(text);
}

void DiagnosisDialog::onErrorOccurred(const QString &error)
{
    m_streamBuf.clear();
    setLoading(false);
    m_conversationHtml += tr(
        "<p style=\"color:gray;font-style:italic;\">Could not get AI explanation: %1<br/>"
        "Make sure Ollama is running (<code>ollama serve</code>).</p><hr/>")
        .arg(error.toHtmlEscaped());
    renderConversation();
    m_inputEdit->setEnabled(true);
    m_sendButton->setEnabled(true);
}

void DiagnosisDialog::renderConversation()
{
    // While a response is streaming, show it under the same label it will be
    // committed with, so the live text reads identically to the final turn.
    const QString live = m_streamBuf.isEmpty() ? QString() : aiBlock(m_streamBuf);
    m_explanationEdit->setHtml(m_conversationHtml + live);
    // keep view pinned to the newest text while streaming
    QTextCursor c = m_explanationEdit->textCursor();
    c.movePosition(QTextCursor::End);
    m_explanationEdit->setTextCursor(c);
    m_explanationEdit->ensureCursorVisible();
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

    // RAG status label (hidden until ragStatusChanged fires)
    m_ragLabel = new QLabel(this);
    m_ragLabel->setStyleSheet("color: gray; font-size: 10px;");
    m_ragLabel->setVisible(false);
    layout->addWidget(m_ragLabel);

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

    // Follow-up input row
    auto *inputRow = new QHBoxLayout();
    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setPlaceholderText(tr("Ask a follow-up question…"));
    m_inputEdit->setEnabled(false);
    m_sendButton = new QPushButton(tr("Send"), this);
    m_sendButton->setEnabled(false);
    m_sendButton->setAutoDefault(false);
    m_sendButton->setDefault(false);
    connect(m_sendButton, &QPushButton::clicked, this, &DiagnosisDialog::onSendClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed,  this, &DiagnosisDialog::onSendClicked);
    inputRow->addWidget(m_inputEdit, 1);
    inputRow->addWidget(m_sendButton);
    layout->addLayout(inputRow);

    // Close button
    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    m_closeButton = new QPushButton(tr("Close"), this);
    m_closeButton->setAutoDefault(false);
    m_closeButton->setDefault(false);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(m_closeButton);
    layout->addLayout(btnRow);
}

void DiagnosisDialog::setLoading(bool loading)
{
    m_progressBar->setVisible(loading);
    m_statusLabel->setVisible(loading);
}
