#ifndef RAGRETRIEVER_H
#define RAGRETRIEVER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QNetworkAccessManager>
#include <QNetworkReply>

struct RagCitation {
    QString source;       // db label, e.g. witschi-training
    QString displayName;  // human-readable title
    QString snippet;      // short excerpt shown in the UI
    QString text;         // full chunk text (prompt injection)
};

// Loads pre-computed embeddings from vector.db (generated offline by
// embed_docs.py) and retrieves the top-k most relevant text chunks for
// a given query by calling Ollama /api/embeddings and computing cosine
// similarity in-process.

class RagRetriever : public QObject
{
    Q_OBJECT
public:
    explicit RagRetriever(QObject *parent = nullptr);

    bool load(const QString &dbPath);          // load embeddings from SQLite
    bool isLoaded() const { return m_loaded; }
    int  chunkCount() const { return m_texts.size(); }

    // Async: embeds query via Ollama then emits retrieved()
    void retrieve(const QString &query,
                  const QString &modelName,
                  int            topK = 3);

    static QString displayNameForSource(const QString &sourceLabel);
    static QString makeSnippet(const QString &text, int maxChars = 120);

signals:
    void retrieved(const QVector<RagCitation> &citations);
    void errorOccurred(const QString &msg);

private slots:
    void onEmbedReplyFinished(QNetworkReply *reply);

private:
    static float cosine(const QVector<float> &a, const QVector<float> &b);

    QNetworkAccessManager       *m_nam;
    QVector<QVector<float>>      m_embeddings;
    QStringList                  m_texts;
    QStringList                  m_sources;
    bool                         m_loaded = false;
    int                          m_topK   = 3;

    static constexpr const char *kOllamaBase = "http://127.0.0.1:11434";
};

#endif // RAGRETRIEVER_H
