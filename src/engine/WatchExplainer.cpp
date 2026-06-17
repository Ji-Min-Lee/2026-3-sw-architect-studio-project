#include "WatchExplainer.h"
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
            m_pendingReply->abort();   // triggers finished() with OperationCanceledError
            m_pendingReply = nullptr;
        }
        emit errorOccurred(tr("Request timed out (model may still be loading — try again)."));
    });
}

// ── Public ────────────────────────────────────────────────────────────────────

void WatchExplainer::warmup(const QString &modelName)
{
    // POST /api/generate with empty prompt — Ollama loads the model into RAM
    // without generating any tokens. Fire-and-forget (no timeout needed).
    QNetworkRequest request(QUrl(QString("%1/api/generate").arg(kOllamaBase)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setAttribute(QNetworkRequest::User, QVariant("warmup"));

    QJsonObject options;
    options["num_ctx"] = 512;

    QJsonObject body;
    body["model"]      = modelName;
    body["prompt"]     = "";
    body["keep_alive"] = "10m";
    body["options"]    = options;  // keep model in RAM for 10 min after last use

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
    options["num_ctx"] = 512;   // KV cache: 1536 MiB (4096) → 192 MiB (512)

    QJsonObject body;
    body["model"]   = req.modelName;
    body["stream"]  = false;
    body["options"] = options;
    body["messages"] = QJsonArray{ userMsg };

    qInfo() << "[WatchExplainer] Sending request to Ollama, model:" << req.modelName;
    m_pendingReply = m_nam->post(request, QJsonDocument(body).toJson());
    m_timeout->start(kTimeoutMs);
}

void WatchExplainer::checkAvailability()
{
    QNetworkRequest request(QUrl(QString("%1/api/tags").arg(kOllamaBase)));
    request.setAttribute(QNetworkRequest::User, QVariant("tags"));
    m_nam->get(request);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

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

    // "explain" response
    if (reply->error() != QNetworkReply::NoError) {
        // OperationCanceledError means our timeout already emitted errorOccurred
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            qWarning() << "[WatchExplainer] Network error:" << reply->errorString();
            emit errorOccurred(tr("Ollama not reachable: %1").arg(reply->errorString()));
        }
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QString text = doc["message"]["content"].toString().trimmed();
    if (text.isEmpty())
        text = doc["response"].toString().trimmed();   // older Ollama format

    if (text.isEmpty()) {
        qWarning() << "[WatchExplainer] Empty response from model";
        emit errorOccurred(tr("Empty response from model."));
        return;
    }

    qInfo() << "[WatchExplainer] Response received, length:" << text.size() << "chars";
    emit explanationReady(text);
}

void WatchExplainer::onTagsReplyFinished(QNetworkReply *reply)
{
    bool ok = (reply->error() == QNetworkReply::NoError);
    if (ok != m_available) {
        m_available = ok;
        emit availabilityChanged(m_available);
    }
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
