#ifndef RAGRETRIEVER_H
#define RAGRETRIEVER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QNetworkAccessManager>
#include <QNetworkReply>

// Loads pre-computed embeddings from vector.db (generated offline by
// embed_docs.py) and retrieves the top-k most relevant text chunks for
// a given query by calling Ollama /api/embeddings and computing cosine
// similarity in-process.
//
// Usage:
//   RagRetriever rag;
//   rag.load("path/to/vector.db");          // once at startup
//   rag.retrieve("query text", model, 3);   // async
//   connect(&rag, &RagRetriever::retrieved, this, [](QStringList chunks){ ... });

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

signals:
    void retrieved(const QStringList &chunks);  // topK most relevant chunks
    void errorOccurred(const QString &msg);

private slots:
    void onEmbedReplyFinished(QNetworkReply *reply);

private:
    static float cosine(const QVector<float> &a, const QVector<float> &b);

    QNetworkAccessManager       *m_nam;
    QVector<QVector<float>>      m_embeddings;  // all chunk embeddings (in RAM)
    QStringList                  m_texts;        // parallel chunk texts
    bool                         m_loaded = false;
    int                          m_topK   = 3;

    static constexpr const char *kOllamaBase = "http://127.0.0.1:11434";
};

#endif // RAGRETRIEVER_H
