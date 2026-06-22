#ifndef DIAGNOSISDIALOG_H
#define DIAGNOSISDIALOG_H

#include <QDialog>
#include <QResizeEvent>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QToolButton>
#include <QScrollArea>
#include <QFrame>
#include <QVBoxLayout>

#include "../engine/WatchDiagnostics.h"
#include "../engine/WatchExplainer.h"
#include "../rag/RagRetriever.h"

class DiagnosisDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DiagnosisDialog(const ExplainRequest &req,
                             WatchExplainer       *explainer,
                             QWidget              *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onExplanationReady(const QString &text);
    void onErrorOccurred(const QString &error);
    void onSendClicked();
    void onRagCitationsReady(const QVector<RagCitation> &citations);
    void onSourcesClicked();
    void onBreakdownClicked();

private:
    void setupUi(const ExplainRequest &req);
    void setLoading(bool loading);
    void renderConversation();
    void setSourcesExpanded(bool expanded);
    void setBreakdownExpanded(bool expanded);
    void refreshBreakdownLabelHeight();

    WatchExplainer *m_explainer;

    QString m_conversationHtml;
    QString m_streamBuf;
    QString m_sourcesHtml;
    QString m_breakdownHtml;
    QString m_breakdownSummary;
    QVector<RagCitation> m_citations;
    bool    m_sourcesExpanded   = false;
    bool    m_breakdownExpanded = false;
    bool    m_hasSources        = false;

    QLabel       *m_titleLabel;
    QFrame       *m_breakdownSection;
    QToolButton  *m_breakdownToggle;
    QLabel       *m_breakdownSummaryLabel;
    QLabel       *m_breakdownLabel;
    QFrame       *m_sourcesSection;
    QToolButton  *m_sourcesToggle;
    QLabel       *m_sourcesSummaryLabel;
    QScrollArea  *m_sourcesScroll;
    QLabel       *m_sourcesLabel;
    QFrame       *m_aiSection;
    QLabel       *m_statusLabel;
    QProgressBar *m_progressBar;
    QTextEdit    *m_explanationEdit;
    QLineEdit    *m_inputEdit;
    QPushButton  *m_sendButton;
    QPushButton  *m_closeButton;
};

#endif /* DIAGNOSISDIALOG_H */
