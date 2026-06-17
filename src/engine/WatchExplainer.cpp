#include "WatchExplainer.h"
#include <algorithm>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>

WatchExplainer::WatchExplainer(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_timeout(new QTimer(this))
{
    connect(m_nam, &QNetworkAccessManager::finished,
            this,  &WatchExplainer::onReplyFinished);

    m_timeout->setSingleShot(true);
    connect(m_timeout, &QTimer::timeout, this, [this]() {
        qWarning() << "[WatchExplainer] Request timed out after" << kTimeoutMs/1000 << "s";
        if (m_pendingReply) {
            m_pendingReply->abort();
            m_pendingReply = nullptr;
        }
        emit errorOccurred(tr("Request timed out (model may still be loading — try again)."));
    });
}

// ── Public ────────────────────────────────────────────────────────────────────

void WatchExplainer::warmup(const QString &modelName)
{
    QNetworkRequest request(QUrl(QString("%1/api/generate").arg(kOllamaBase)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setAttribute(QNetworkRequest::User, QVariant("warmup"));

    QJsonObject options;
    options["num_ctx"] = 512;

    QJsonObject body;
    body["model"]      = modelName;
    body["prompt"]     = "";
    body["keep_alive"] = "10m";
    body["options"]    = options;

    qInfo() << "[WatchExplainer] Warming up model:" << modelName;
    m_nam->post(request, QJsonDocument(body).toJson());
}

void WatchExplainer::explain(const ExplainRequest &req)
{
    QNetworkRequest request(QUrl(QString("%1/api/chat").arg(kOllamaBase)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setAttribute(QNetworkRequest::User, QVariant("explain"));

    QJsonObject userMsg;
    userMsg["role"]    = "user";
    userMsg["content"] = buildPrompt(req);

    QJsonObject options;
    options["num_ctx"] = 512;

    QJsonObject body;
    body["model"]    = req.modelName;
    body["stream"]   = true;   // stream tokens as they are generated
    body["options"]  = options;
    body["messages"] = QJsonArray{ userMsg };

    m_accumulated.clear();
    qInfo() << "[WatchExplainer] Sending streaming request, model:" << req.modelName;
    m_pendingReply = m_nam->post(request, QJsonDocument(body).toJson());
    m_timeout->start(kTimeoutMs);

    connect(m_pendingReply, &QNetworkReply::readyRead,
            this,           &WatchExplainer::onReadyRead);
}

void WatchExplainer::checkAvailability()
{
    QNetworkRequest request(QUrl(QString("%1/api/tags").arg(kOllamaBase)));
    request.setAttribute(QNetworkRequest::User, QVariant("tags"));
    m_nam->get(request);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void WatchExplainer::onReadyRead()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;

    // Ollama streams newline-delimited JSON objects, one per token.
    // Each line: {"message":{"role":"assistant","content":"token"},"done":false}
    while (reply->canReadLine()) {
        const QByteArray line = reply->readLine().trimmed();
        if (line.isEmpty()) continue;

        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (doc.isNull()) continue;

        const QString token = doc["message"]["content"].toString();
        if (!token.isEmpty()) {
            m_accumulated += token;
            emit tokenReceived(token);
        }
    }
}

void WatchExplainer::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    m_timeout->stop();
    m_pendingReply = nullptr;

    const QString tag = reply->request()
                            .attribute(QNetworkRequest::User).toString();

    if (tag == "tags") {
        onTagsReplyFinished(reply);
        return;
    }
    if (tag == "warmup") {
        qInfo() << "[WatchExplainer] Model warmup complete";
        return;
    }

    // "explain" — stream finished
    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            qWarning() << "[WatchExplainer] Network error:" << reply->errorString();
            emit errorOccurred(tr("Ollama not reachable: %1").arg(reply->errorString()));
        }
        return;
    }

    if (m_accumulated.isEmpty()) {
        qWarning() << "[WatchExplainer] Empty response from model";
        emit errorOccurred(tr("Empty response from model."));
        return;
    }

    qInfo() << "[WatchExplainer] Stream complete, length:" << m_accumulated.size() << "chars";
    emit explanationReady(m_accumulated);
}

void WatchExplainer::onTagsReplyFinished(QNetworkReply *reply)
{
    bool ok = (reply->error() == QNetworkReply::NoError);
    if (ok != m_available) {
        m_available = ok;
        emit availabilityChanged(m_available);
    }
    if (!ok) return;

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    // sort by size ascending so index 0 is the smallest (fastest) model
    struct ModelEntry { QString name; qint64 size; };
    QList<ModelEntry> entries;
    for (const QJsonValue &v : doc["models"].toArray())
        entries.append({ v["name"].toString(), v["size"].toInteger() });
    std::sort(entries.begin(), entries.end(),
              [](const ModelEntry &a, const ModelEntry &b){ return a.size < b.size; });
    QStringList models;
    for (const ModelEntry &e : entries)
        models << e.name;
    if (!models.isEmpty())
        emit modelsAvailable(models);
}

// ── Prompt builder ────────────────────────────────────────────────────────────

QString WatchExplainer::buildPrompt(const ExplainRequest &req) const
{
    const DiagnosisInput  &in  = req.input;
    const DiagnosisResult &res = req.result;

    QString levelStr;
    switch (res.level) {
        case DiagnosisLevel::Excellent:    levelStr = "Excellent";     break;
        case DiagnosisLevel::Good:         levelStr = "Good";          break;
        case DiagnosisLevel::NeedsService: levelStr = "Needs Service"; break;
        default:                           levelStr = "Unknown";       break;
    }

    QString watchType = (in.watch_type == WatchType::Women) ? "ladies'" : "men's";

    return QString(
        "You are a watchmaker. A %1 watch timegrapher reading:\n"
        "Rate %2 s/d, Amplitude %3 deg, Beat Error %4 ms. Diagnosis: %5.\n"
        "In 3 sentences: why this diagnosis, likely mechanical cause, what to service."
    )
    .arg(watchType)
    .arg(in.rate_spd,        0, 'f', 1)
    .arg(in.amplitude_deg,   0, 'f', 0)
    .arg(in.beat_error_ms,   0, 'f', 2)
    .arg(levelStr);
}
