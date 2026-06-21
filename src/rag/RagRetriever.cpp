#include "RagRetriever.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QVariant>
#include <cmath>
#include <algorithm>

RagRetriever::RagRetriever(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    connect(m_nam, &QNetworkAccessManager::finished,
            this,  &RagRetriever::onEmbedReplyFinished);
}

// ── Public ────────────────────────────────────────────────────────────────────

bool RagRetriever::load(const QString &dbPath)
{
    m_embeddings.clear();
    m_texts.clear();
    m_loaded = false;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "rag");
        db.setDatabaseName(dbPath);
        if (!db.open()) {
            qWarning() << "[RagRetriever] Cannot open db:" << db.lastError().text();
            QSqlDatabase::removeDatabase("rag");
            return false;
        }

        QSqlQuery q(db);
        q.exec("SELECT text, embedding FROM chunks ORDER BY id");
        while (q.next()) {
            m_texts << q.value(0).toString();

            const QByteArray blob = q.value(1).toByteArray();
            const int n = blob.size() / sizeof(float);
            QVector<float> vec(n);
            memcpy(vec.data(), blob.constData(), blob.size());
            m_embeddings << vec;
        }
    } // q and db destroyed here before removeDatabase
    QSqlDatabase::removeDatabase("rag");

    m_loaded = !m_texts.isEmpty();
    qInfo() << "[RagRetriever] Loaded" << m_texts.size() << "chunks from" << dbPath;
    return m_loaded;
}

void RagRetriever::retrieve(const QString &query,
                             const QString &modelName,
                             int            topK)
{
    if (!m_loaded) {
        emit errorOccurred("RAG database not loaded");
        return;
    }
    m_topK = topK;

    QNetworkRequest req(QUrl(QString("%1/api/embeddings").arg(kOllamaBase)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setAttribute(QNetworkRequest::User, QVariant(query));  // carry query for debug

    QJsonObject body;
    body["model"]  = modelName;
    body["prompt"] = query;

    m_nam->post(req, QJsonDocument(body).toJson());
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void RagRetriever::onEmbedReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[RagRetriever] Embed error:" << reply->errorString();
        emit errorOccurred(reply->errorString());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray arr = doc["embedding"].toArray();
    if (arr.isEmpty()) {
        emit errorOccurred("Empty embedding from Ollama");
        return;
    }

    QVector<float> queryVec(arr.size());
    for (int i = 0; i < arr.size(); ++i)
        queryVec[i] = static_cast<float>(arr[i].toDouble());

    // score all chunks
    QVector<std::pair<float, int>> scores;
    scores.reserve(m_embeddings.size());
    for (int i = 0; i < m_embeddings.size(); ++i)
        scores.append({ cosine(queryVec, m_embeddings[i]), i });

    std::partial_sort(scores.begin(),
                      scores.begin() + std::min(m_topK, (int)scores.size()),
                      scores.end(),
                      [](const auto &a, const auto &b){ return a.first > b.first; });

    QStringList result;
    for (int i = 0; i < std::min(m_topK, (int)scores.size()); ++i)
        result << m_texts[scores[i].second];

    emit retrieved(result);
}

// ── Private ───────────────────────────────────────────────────────────────────

float RagRetriever::cosine(const QVector<float> &a, const QVector<float> &b)
{
    if (a.size() != b.size() || a.isEmpty()) return 0.0f;
    double dot = 0, na = 0, nb = 0;
    for (int i = 0; i < a.size(); ++i) {
        dot += a[i] * b[i];
        na  += a[i] * a[i];
        nb  += b[i] * b[i];
    }
    if (na == 0 || nb == 0) return 0.0f;
    return static_cast<float>(dot / (std::sqrt(na) * std::sqrt(nb)));
}
