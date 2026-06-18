#ifndef DIAGNOSISDIALOG_H
#define DIAGNOSISDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

#include "../engine/WatchDiagnostics.h"
#include "../engine/WatchExplainer.h"

class DiagnosisDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DiagnosisDialog(const ExplainRequest &req,
                             WatchExplainer       *explainer,
                             QWidget              *parent = nullptr);

private slots:
    void onExplanationReady(const QString &text);
    void onErrorOccurred(const QString &error);

private:
    void setupUi(const DiagnosisResult &result);
    void setLoading(bool loading);

    WatchExplainer *m_explainer;

    QLabel       *m_titleLabel;
    QLabel       *m_statusLabel;
    QTextEdit    *m_explanationEdit;
    QProgressBar *m_progressBar;
    QPushButton  *m_closeButton;
};

#endif /* DIAGNOSISDIALOG_H */
