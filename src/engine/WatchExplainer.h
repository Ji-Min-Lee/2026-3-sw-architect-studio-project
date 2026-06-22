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
#include <QTimer>
#include <QJsonArray>

#include "WatchDiagnostics.h"
#include "RagRetriever.h"

struct ExplainRequest {
    DiagnosisInput  input;
    DiagnosisResult result;
    QString         modelName = "qwen2.5:0.5b"; // default Ollama model
};

class WatchExplainer : public QObject
{
    Q_OBJECT
public:
    explicit WatchExplainer(QObject *parent = nullptr);

    void explain(const ExplainRequest &req);
    void chat(const QString &userMessage); // follow-up turn using existing history
    void warmup(const QString &modelName = "qwen2.5:0.5b"); // preload model into RAM
    void loadRag(const QString &dbPath);   // optional: load vector.db for context

    bool isOllamaAvailable() const { return m_available; }
    void checkAvailability();            // async ping to /api/tags

signals:
    void tokenReceived(const QString &token);   // streamed one token at a time
    void explanationReady(const QString &text); // full text when done
    void errorOccurred(const QString &errorMsg);
    void availabilityChanged(bool available);
    void modelsAvailable(const QStringList &models);
    void ragStatusChanged(bool active, int chunkCount);

private slots:
    void onReadyRead();
    void onReplyFinished(QNetworkReply *reply);
    void onTagsReplyFinished(QNetworkReply *reply);
    void onRagRetrieved(const QStringList &chunks);
    void onRagError(const QString &msg);

private:
    QString buildPrompt(const ExplainRequest &req,
                        const QStringList   &context) const;
    void    explainWithContext(const ExplainRequest &req,
                               const QStringList   &context);

    QNetworkAccessManager *m_nam;
    QNetworkReply         *m_pendingReply = nullptr;
    QTimer                *m_timeout;
    QString                m_accumulated;
    bool                   m_available = false;
    RagRetriever           m_rag;
    ExplainRequest         m_pendingReq;   // held while waiting for RAG
    QJsonArray             m_history;      // multi-turn: accumulated messages
    QString                m_currentModel;

    static constexpr const char *kOllamaBase    = "http://127.0.0.1:11434";
    static constexpr int         kTimeoutMs      = 120000;  // 2 min — RPi5 first-load is slow
};

#endif /* WATCHEXPLAINER_H */
