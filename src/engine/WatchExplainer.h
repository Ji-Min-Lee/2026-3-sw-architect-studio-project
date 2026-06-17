/* WatchExplainer.h
 *
 * AI Step 2: given a diagnosis result and the raw measurements that
 * produced it, ask a locally-running Ollama LLM to explain the finding
 * and suggest corrective actions.
 *
 * Inference is fully on-device via Ollama (localhost:11434).
 * The class emits explanationReady() asynchronously so the UI is never
 * blocked; call explain() and connect explanationReady / errorOccurred.
 */
#ifndef WATCHEXPLAINER_H
#define WATCHEXPLAINER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "WatchDiagnostics.h"

struct ExplainRequest {
    DiagnosisInput  input;
    DiagnosisResult result;
    QString         modelName = "phi3:mini";   // default Ollama model
};

class WatchExplainer : public QObject
{
    Q_OBJECT
public:
    explicit WatchExplainer(QObject *parent = nullptr);

    void explain(const ExplainRequest &req);

    bool isOllamaAvailable() const { return m_available; }
    void checkAvailability();            // async ping to /api/tags

signals:
    void explanationReady(const QString &text);
    void errorOccurred(const QString &errorMsg);
    void availabilityChanged(bool available);

private slots:
    void onReplyFinished(QNetworkReply *reply);
    void onTagsReplyFinished(QNetworkReply *reply);

private:
    QString buildPrompt(const ExplainRequest &req) const;

    QNetworkAccessManager *m_nam;
    bool                   m_available = false;

    static constexpr const char *kOllamaBase = "http://127.0.0.1:11434";
};

#endif /* WATCHEXPLAINER_H */
