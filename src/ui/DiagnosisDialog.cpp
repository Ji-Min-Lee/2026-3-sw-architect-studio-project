#include "DiagnosisDialog.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QFont>
#include <QTextCursor>
#include <QTextDocument>
#include <QScrollBar>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QShortcut>
#include <QTimer>
#include <QRegularExpression>
#include <cmath>

namespace {

QString markdownToHtmlBody(const QString &md)
{
    QTextDocument doc;
    doc.setMarkdown(md);
    QString html = doc.toHtml();
    int s = html.indexOf("<body");
    if (s < 0) return md.toHtmlEscaped();
    s = html.indexOf('>', s) + 1;
    const int e = html.lastIndexOf("</body>");
    QString body = html.mid(s, e - s);
    // toHtml() bakes the palette default foreground into the markup; under an
    // OS dark theme that is white, which then disappears on this dialog's fixed
    // white background. Strip those colour declarations so the text inherits
    // the explicit dark colour set on the QTextEdit instead.
    body.remove(QRegularExpression(
        QStringLiteral("color\\s*:\\s*(#[0-9a-fA-F]{3,6}|black|white)\\s*;?"),
        QRegularExpression::CaseInsensitiveOption));
    return body;
}

QString aiBlock(const QString &md)
{
    return QStringLiteral("<p><b>🧠 Diagnostic AI</b></p>") + markdownToHtmlBody(md);
}

QString formatRagCollapsedTitles(const QVector<RagCitation> &citations)
{
    QStringList names;
    for (const RagCitation &cite : citations)
        names << cite.displayName;
    return names.join(QStringLiteral(" · "));
}

QString formatRagCitationsHtml(const QVector<RagCitation> &citations)
{
    QString html = QStringLiteral(
        "<html><body style='color:#222;font-size:10px;'>"
        "<ol style='margin:0;padding-left:18px;'>");

    for (const RagCitation &cite : citations) {
        html += QStringLiteral(
            "<li style='margin-bottom:6px;'>"
            "<b>%1</b><br/>"
            "<span style='color:#666;'>\"%2\"</span>"
            "</li>")
                    .arg(cite.displayName.toHtmlEscaped(),
                         cite.snippet.toHtmlEscaped());
    }
    html += QStringLiteral("</ol></body></html>");
    return html;
}

QString watchTypeLabel(const DiagnosisInput &input)
{
    return (input.watch_type == WatchType::Women)
               ? QStringLiteral("Women")
               : QStringLiteral("Men");
}

QString unifiedScrollBarStyle()
{
    return QStringLiteral(
        "QScrollBar:vertical {"
        "  background: #f0f0f0;"
        "  width: 10px;"
        "  margin: 0;"
        "  border: none;"
        "  border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #b0b0b0;"
        "  min-height: 24px;"
        "  border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: #909090;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0;"
        "  background: none;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "  background: none;"
        "}");
}

QFrame *makeSection(QWidget *parent, const char *objectName)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName(objectName);
    frame->setFrameShape(QFrame::NoFrame);
    return frame;
}

QString sectionStyle()
{
    return QStringLiteral(
        "QFrame#breakdownSection, QFrame#sourcesSection, QFrame#aiSection {"
        "  background: #f8f9fb;"
        "  border: 1px solid #dde1e6;"
        "  border-radius: 6px;"
        "}"
        "QToolButton {"
        "  border: none;"
        "  color: #333;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "  text-align: left;"
        "  padding: 2px 0;"
        "}"
        "QToolButton:hover { color: #1565c0; }");
}

} // namespace

DiagnosisDialog::DiagnosisDialog(const ExplainRequest &req,
                                 WatchExplainer       *explainer,
                                 QWidget              *parent)
    : QDialog(parent)
    , m_explainer(explainer)
{
    setWindowTitle(tr("AI Diagnosis Explanation"));
    setMinimumSize(540, 520);
    resize(560, 620);

    setupUi(req);

    connect(m_explainer, &WatchExplainer::tokenReceived,
            this, [this](const QString &token) {
                m_streamBuf += token;
                renderConversation();
            });
    connect(m_explainer, &WatchExplainer::explanationReady,
            this,        &DiagnosisDialog::onExplanationReady);
    connect(m_explainer, &WatchExplainer::errorOccurred,
            this,        &DiagnosisDialog::onErrorOccurred);
    connect(m_explainer, &WatchExplainer::ragCitationsReady,
            this,        &DiagnosisDialog::onRagCitationsReady);

    setLoading(true);
    m_explainer->explain(req);

    auto *scEsc = new QShortcut(Qt::Key_Escape, this);
    scEsc->setContext(Qt::WindowShortcut);
    connect(scEsc, &QShortcut::activated, this, &QDialog::close);
}

void DiagnosisDialog::refreshBreakdownLabelHeight()
{
    if (!m_breakdownLabel->isVisible())
        return;

    const int framePad = 24;  // left indent + horizontal padding
    const int w = qMax(200, (m_breakdownLabel->width() > 0
                                 ? m_breakdownLabel->width()
                                 : m_breakdownSection->width()) - framePad);

    QTextDocument doc;
    doc.setHtml(m_breakdownHtml);
    doc.setTextWidth(w);

    const int contentH = static_cast<int>(std::ceil(doc.size().height()));
    const int verticalPad = 16;  // matches tightened stylesheet padding (8 top + 8 bottom)
    m_breakdownLabel->setMinimumHeight(contentH + verticalPad);
    m_breakdownLabel->updateGeometry();
}

void DiagnosisDialog::onBreakdownClicked()
{
    setBreakdownExpanded(!m_breakdownExpanded);
}

void DiagnosisDialog::setBreakdownExpanded(bool expanded)
{
    m_breakdownExpanded = expanded;
    m_breakdownToggle->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);

    if (expanded) {
        m_breakdownSummaryLabel->setVisible(false);
        m_breakdownLabel->setText(m_breakdownHtml);
        m_breakdownLabel->setVisible(true);
        refreshBreakdownLabelHeight();
        QTimer::singleShot(0, this, [this]() { refreshBreakdownLabelHeight(); });
    } else {
        m_breakdownLabel->setVisible(false);
        m_breakdownSummaryLabel->setText(m_breakdownSummary);
        m_breakdownSummaryLabel->setVisible(true);
    }
}

void DiagnosisDialog::onRagCitationsReady(const QVector<RagCitation> &citations)
{
    if (citations.isEmpty()) {
        m_hasSources = false;
        m_citations.clear();
        m_sourcesSection->setVisible(false);
        return;
    }

    m_hasSources = true;
    m_citations = citations;
    m_sourcesHtml = formatRagCitationsHtml(citations);
    m_sourcesToggle->setText(
        tr("📚 RAG Sources (%1)").arg(citations.size()));
    m_sourcesSection->setVisible(true);
    setSourcesExpanded(false);
}

void DiagnosisDialog::onSourcesClicked()
{
    if (!m_hasSources)
        return;
    setSourcesExpanded(!m_sourcesExpanded);
}

void DiagnosisDialog::setSourcesExpanded(bool expanded)
{
    m_sourcesExpanded = expanded;
    m_sourcesToggle->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);

    if (expanded) {
        m_sourcesSummaryLabel->setVisible(false);
        m_sourcesLabel->setText(m_sourcesHtml);
        m_sourcesScroll->setVisible(true);
    } else {
        m_sourcesScroll->setVisible(false);
        m_sourcesSummaryLabel->setText(formatRagCollapsedTitles(m_citations));
        m_sourcesSummaryLabel->setVisible(true);
    }
}

void DiagnosisDialog::onExplanationReady(const QString &)
{
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
    const QString live = m_streamBuf.isEmpty() ? QString() : aiBlock(m_streamBuf);
    m_explanationEdit->setHtml(m_conversationHtml + live);
    QTextCursor c = m_explanationEdit->textCursor();
    c.movePosition(QTextCursor::End);
    m_explanationEdit->setTextCursor(c);
    m_explanationEdit->ensureCursorVisible();
}

void DiagnosisDialog::setupUi(const ExplainRequest &req)
{
    setStyleSheet(sectionStyle() + unifiedScrollBarStyle());

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(16, 16, 16, 12);

    m_titleLabel = new QLabel(req.result.label, this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    QColor bg = DiagnosisColor(req.result.level);
    m_titleLabel->setStyleSheet(
        QString("color: white; background: %1; padding: 8px 12px; border-radius: 4px;")
            .arg(bg.name()));
    layout->addWidget(m_titleLabel);

    m_breakdownHtml = formatBreakdownTableHtml(req.result, req.input);
    m_breakdownSummary = formatBreakdownCollapsedSummary(req.result, req.input);

    m_breakdownSection = makeSection(this, "breakdownSection");
    auto *breakdownLayout = new QVBoxLayout(m_breakdownSection);
    breakdownLayout->setContentsMargins(12, 10, 12, 12);
    breakdownLayout->setSpacing(8);

    m_breakdownToggle = new QToolButton(m_breakdownSection);
    m_breakdownToggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_breakdownToggle->setArrowType(Qt::RightArrow);
    m_breakdownToggle->setText(
        tr("📊 Measurement vs thresholds (%1 watch)")
            .arg(watchTypeLabel(req.input)));
    m_breakdownToggle->setCheckable(false);
    m_breakdownToggle->setCursor(Qt::PointingHandCursor);
    connect(m_breakdownToggle, &QToolButton::clicked,
            this, &DiagnosisDialog::onBreakdownClicked);
    breakdownLayout->addWidget(m_breakdownToggle);

    m_breakdownSummaryLabel = new QLabel(m_breakdownSection);
    m_breakdownSummaryLabel->setWordWrap(true);
    m_breakdownSummaryLabel->setStyleSheet(
        "QLabel { color: #444; font-size: 10px; font-family: monospace; padding-left: 18px; }");
    breakdownLayout->addWidget(m_breakdownSummaryLabel);

    m_breakdownLabel = new QLabel(m_breakdownSection);
    m_breakdownLabel->setTextFormat(Qt::RichText);
    m_breakdownLabel->setWordWrap(false);
    m_breakdownLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_breakdownLabel->setVisible(false);
    m_breakdownLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_breakdownLabel->setStyleSheet(
        "QLabel { background: #ffffff; border: 1px solid #e8eaed; border-radius: 4px;"
        "padding: 8px 12px 8px 30px; }");
    breakdownLayout->addWidget(m_breakdownLabel);

    layout->addWidget(m_breakdownSection);
    setBreakdownExpanded(false);

    m_sourcesSection = makeSection(this, "sourcesSection");
    auto *sourcesLayout = new QVBoxLayout(m_sourcesSection);
    sourcesLayout->setContentsMargins(12, 10, 12, 10);
    sourcesLayout->setSpacing(6);

    m_sourcesToggle = new QToolButton(m_sourcesSection);
    m_sourcesToggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_sourcesToggle->setArrowType(Qt::RightArrow);
    m_sourcesToggle->setText(tr("📚 RAG Sources"));
    m_sourcesToggle->setCheckable(false);
    m_sourcesToggle->setCursor(Qt::PointingHandCursor);
    connect(m_sourcesToggle, &QToolButton::clicked,
            this, &DiagnosisDialog::onSourcesClicked);
    sourcesLayout->addWidget(m_sourcesToggle);

    m_sourcesSummaryLabel = new QLabel(m_sourcesSection);
    m_sourcesSummaryLabel->setWordWrap(true);
    m_sourcesSummaryLabel->setVisible(false);
    m_sourcesSummaryLabel->setStyleSheet(
        "QLabel { color: #666; font-size: 10px; padding-left: 18px; }");
    sourcesLayout->addWidget(m_sourcesSummaryLabel);

    m_sourcesLabel = new QLabel;
    m_sourcesLabel->setTextFormat(Qt::RichText);
    m_sourcesLabel->setWordWrap(true);
    m_sourcesLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_sourcesLabel->setStyleSheet(
        "QLabel { background: #ffffff; border: none; padding: 4px; }");

    m_sourcesScroll = new QScrollArea(m_sourcesSection);
    m_sourcesScroll->setWidget(m_sourcesLabel);
    m_sourcesScroll->setWidgetResizable(true);
    m_sourcesScroll->setFrameShape(QFrame::NoFrame);
    m_sourcesScroll->setMaximumHeight(150);
    m_sourcesScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_sourcesScroll->setVisible(false);
    m_sourcesScroll->setStyleSheet(
        "QScrollArea { background: #ffffff; border: 1px solid #e8eaed;"
        "border-radius: 4px; margin-left: 18px; }");
    sourcesLayout->addWidget(m_sourcesScroll);

    m_sourcesSection->setVisible(false);
    layout->addWidget(m_sourcesSection);

    m_aiSection = makeSection(this, "aiSection");
    auto *aiLayout = new QVBoxLayout(m_aiSection);
    aiLayout->setContentsMargins(12, 10, 12, 10);
    aiLayout->setSpacing(6);

    m_statusLabel = new QLabel(tr("Asking AI watchmaker…"), m_aiSection);
    m_statusLabel->setStyleSheet("color: gray; font-style: italic;");
    aiLayout->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar(m_aiSection);
    m_progressBar->setRange(0, 0);
    m_progressBar->setFixedHeight(4);
    m_progressBar->setTextVisible(false);
    aiLayout->addWidget(m_progressBar);

    m_explanationEdit = new QTextEdit(m_aiSection);
    m_explanationEdit->setReadOnly(true);
    m_explanationEdit->setFrameShape(QFrame::NoFrame);
    m_explanationEdit->setMinimumHeight(180);
    m_explanationEdit->setStyleSheet(
        "QTextEdit { background: #ffffff; color: #1a1a1a; border: 1px solid #e8eaed;"
        "border-radius: 4px; padding: 8px; }");
    aiLayout->addWidget(m_explanationEdit, 1);

    layout->addWidget(m_aiSection, 1);

    auto *inputRow = new QHBoxLayout();
    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setPlaceholderText(tr("Ask a follow-up question…"));
    m_inputEdit->setEnabled(false);
    // Pin a light field so it stands out against the dialog under OS dark mode.
    m_inputEdit->setStyleSheet(
        "QLineEdit { background: #ffffff; color: #1a1a1a; border: 1px solid #c4c8cc;"
        "border-radius: 4px; padding: 6px 8px; }"
        "QLineEdit:focus { border: 1px solid #1565c0; }"
        "QLineEdit:disabled { background: #f0f0f0; color: #9aa0a6; }");
    m_sendButton = new QPushButton(tr("Send"), this);
    m_sendButton->setEnabled(false);
    m_sendButton->setAutoDefault(false);
    m_sendButton->setDefault(false);
    connect(m_sendButton, &QPushButton::clicked, this, &DiagnosisDialog::onSendClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed,  this, &DiagnosisDialog::onSendClicked);
    inputRow->addWidget(m_inputEdit, 1);
    inputRow->addWidget(m_sendButton);
    layout->addLayout(inputRow);

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

void DiagnosisDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    refreshBreakdownLabelHeight();
}
