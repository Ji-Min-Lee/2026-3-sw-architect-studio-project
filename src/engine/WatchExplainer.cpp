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
{
    connect(m_nam, &QNetworkAccessManager::finished,
            this,  &WatchExplainer::onReplyFinished);
}

// ── Public ────────────────────────────────────────────────────────────────────

void WatchExplainer::explain(const ExplainRequest &req)
{
    QNetworkRequest request(QUrl(QString("%1/api/chat").arg(kOllamaBase)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setAttribute(QNetworkRequest::User, QVariant("explain"));

    QJsonObject userMsg;
    userMsg["role"]    = "user";
    userMsg["content"] = buildPrompt(req);

    QJsonObject body;
    body["model"]  = req.modelName;
    body["stream"] = false;
    body["messages"] = QJsonArray{ userMsg };

    m_nam->post(request, QJsonDocument(body).toJson());
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

    const QString tag = reply->request()
                            .attribute(QNetworkRequest::User).toString();

    if (tag == "tags") {
        onTagsReplyFinished(reply);
        return;
    }

    // "explain" response
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(tr("Ollama not reachable: %1").arg(reply->errorString()));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QString text = doc["message"]["content"].toString().trimmed();
    if (text.isEmpty())
        text = doc["response"].toString().trimmed();   // older Ollama format

    if (text.isEmpty()) {
        emit errorOccurred(tr("Empty response from model."));
        return;
    }

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
        "You are an expert mechanical watch watchmaker and timegrapher technician.\n"
        "A %1 watch was measured on a timegrapher. Here are the results:\n"
        "  Rate:        %2 s/d  (acceptable: -5 to +15 s/d for men's, -5 to +25 for ladies')\n"
        "  Amplitude:   %3°     (acceptable: ≥270° for Excellent, ≥220° for Good)\n"
        "  Beat Error:  %4 ms   (acceptable: ≤0.5 ms for Excellent, ≤0.8 ms for Good)\n\n"
        "Overall diagnosis: %5\n\n"
        "In 3–5 sentences, explain:\n"
        "1. Which measurement(s) caused this diagnosis and why.\n"
        "2. What mechanical condition likely causes this in real watches.\n"
        "3. What a watchmaker should check or service to improve the reading.\n"
        "Be concise and practical. Do not repeat the numbers back verbatim."
    )
    .arg(watchType)
    .arg(in.rate_spd,        0, 'f', 1)
    .arg(in.amplitude_deg,   0, 'f', 0)
    .arg(in.beat_error_ms,   0, 'f', 2)
    .arg(levelStr);
}
