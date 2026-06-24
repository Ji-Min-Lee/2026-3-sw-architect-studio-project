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
    , m_rag(this)
{
    connect(m_nam, &QNetworkAccessManager::finished,
            this,  &WatchExplainer::onReplyFinished);
    connect(&m_rag, &RagRetriever::retrieved,
            this,   &WatchExplainer::onRagRetrieved);
    connect(&m_rag, &RagRetriever::errorOccurred,
            this,   &WatchExplainer::onRagError);

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
    body["keep_alive"] = "30m";
    body["options"]    = options;

    qInfo() << "[WatchExplainer] Warming up model:" << modelName;
    m_nam->post(request, QJsonDocument(body).toJson());
}

void WatchExplainer::loadRag(const QString &dbPath)
{
    if (m_rag.load(dbPath))
        qInfo() << "[WatchExplainer] RAG ready:" << m_rag.chunkCount() << "chunks from" << dbPath;
    else
        qWarning() << "[WatchExplainer] RAG load failed for" << dbPath
                   << "— will explain without context";
}

void WatchExplainer::explain(const ExplainRequest &req)
{
    if (m_pendingReply) {
        m_pendingReply->abort();
        m_pendingReply = nullptr;
    }
    m_timeout->stop();
    m_accumulated.clear();
    m_history = QJsonArray();
    m_currentModel = req.modelName;
    m_pendingIsChat = false;   // first turn always routes through the explain path

    // Conversation-wide style guide. Applies to the initial explanation and
    // every follow-up turn, keeping small models terse and non-repetitive.
    QJsonObject systemMsg;
    systemMsg["role"]    = "system";
    systemMsg["content"] =
        "You are an expert watchmaker assistant. Only answer questions about "
        "watches, timegrapher readings, watch repair, and horology. "
        "If the user asks about anything unrelated to watches, reply: "
        "'I can only help with watch-related questions.' "
        "Be concise: at most 3 short sentences, no preamble, no repetition.";
    m_history.append(systemMsg);

    if (m_rag.isLoaded()) {
        // retrieve relevant context first, then fire LLM in onRagRetrieved()
        m_pendingReq = req;
        const QString query = QString("watch diagnosis %1 rate %2 amplitude %3 beat error %4")
            .arg(req.result.label)
            .arg(req.input.metrics.rate.value_or(0.0),      0, 'f', 1)
            .arg(req.input.metrics.amplitude.value_or(0.0), 0, 'f', 0)
            .arg(req.input.metrics.beatError.value_or(0.0), 0, 'f', 2);
        m_rag.retrieve(query, "nomic-embed-text");
        m_timeout->start(kTimeoutMs);
        return;
    }
    explainWithContext(req, {});
    emit ragCitationsReady({});
}

void WatchExplainer::chat(const QString &userMessage)
{
    if (m_pendingReply) {
        m_pendingReply->abort();
        m_pendingReply = nullptr;
    }
    m_timeout->stop();
    m_accumulated.clear();

    // Meta-requests (translate, summarize, format) don't benefit from RAG —
    // injecting watchmaking chunks confuses the model on non-technical tasks.
    static const QStringList kMetaKeywords = {
        "translate", "번역", "summarize", "요약", "Korean", "한국어", "한글",
        "English", "영어", "shorter", "simpler", "explain again", "다시", "짧게"
    };
    bool isMeta = false;
    for (const QString &kw : kMetaKeywords)
        if (userMessage.contains(kw, Qt::CaseInsensitive)) { isMeta = true; break; }

    if (m_rag.isLoaded() && !isMeta) {
        // Retrieve chunks relevant to THIS follow-up question, then send in
        // onRagRetrieved(). Each turn gets its own question-specific context.
        m_pendingIsChat      = true;
        m_pendingChatMessage = userMessage;
        m_rag.retrieve(userMessage, "nomic-embed-text");
        m_timeout->start(kTimeoutMs);
        return;
    }
    sendChat(userMessage, {});
}

void WatchExplainer::sendChat(const QString &userMessage, const QStringList &context)
{
    if (context.isEmpty())
        qInfo() << "[WatchExplainer] Chat: no RAG context";
    else
        qInfo() << "[WatchExplainer] Chat RAG: injected" << context.size() << "chunks";

    emit ragStatusChanged(!context.isEmpty(), context.size());

    QJsonObject userMsg;
    userMsg["role"]    = "user";
    userMsg["content"] = userMessage + formatContextBlock(context);
    m_history.append(userMsg);

    QJsonObject options;
    options["num_ctx"]        = 2048; // multi-turn: history accumulates each turn
    options["num_thread"]     = 2;
    options["num_predict"]    = 512;  // full answer, but bounds runaway length
    options["repeat_penalty"] = 1.3;  // small models loop on RPi5 — penalise repeats
    options["repeat_last_n"]  = 256;  // window the repeat penalty looks back over

    QJsonObject body;
    body["model"]      = m_currentModel;
    body["stream"]     = true;
    body["keep_alive"] = "30m";  // keep weights resident between turns
    body["options"]    = options;
    body["messages"]   = m_history;

    QNetworkRequest request(QUrl(QString("%1/api/chat").arg(kOllamaBase)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setAttribute(QNetworkRequest::User, QVariant("explain"));

    qInfo() << "[WatchExplainer] Chat follow-up, history size:" << m_history.size();
    m_pendingReply = m_nam->post(request, QJsonDocument(body).toJson());
    m_timeout->start(kTimeoutMs);

    connect(m_pendingReply, &QNetworkReply::readyRead,
            this,           &WatchExplainer::onReadyRead);
}

void WatchExplainer::onRagRetrieved(const QVector<RagCitation> &citations)
{
    m_timeout->stop();
    QStringList chunks;
    chunks.reserve(citations.size());
    for (const RagCitation &cite : citations)
        chunks << cite.text;
    emit ragCitationsReady(citations);
    if (m_pendingIsChat) {
        m_pendingIsChat = false;
        sendChat(m_pendingChatMessage, chunks);
    } else {
        explainWithContext(m_pendingReq, chunks);
    }
}

void WatchExplainer::onRagError(const QString &msg)
{
    m_timeout->stop();
    qWarning() << "[WatchExplainer] RAG retrieval failed:" << msg << "— proceeding without context";
    if (m_pendingIsChat) {
        m_pendingIsChat = false;
        sendChat(m_pendingChatMessage, {});  // keep existing sources panel as-is
    } else {
        explainWithContext(m_pendingReq, {});
        emit ragCitationsReady({});
    }
}

void WatchExplainer::explainWithContext(const ExplainRequest &req, const QStringList &context)
{
    if (context.isEmpty())
        qInfo() << "[WatchExplainer] RAG: no context (vector.db not loaded or retrieval failed)";
    else
        qInfo() << "[WatchExplainer] RAG: injected" << context.size() << "chunks into prompt";

    emit ragStatusChanged(!context.isEmpty(), context.size());

    QNetworkRequest request(QUrl(QString("%1/api/chat").arg(kOllamaBase)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setAttribute(QNetworkRequest::User, QVariant("explain"));

    QJsonObject userMsg;
    userMsg["role"]    = "user";
    userMsg["content"] = buildPrompt(req, context);
    m_history.append(userMsg);

    QJsonObject options;
    options["num_ctx"]        = 2048; // multi-turn: leave room for follow-up history
    options["num_thread"]     = 2;    // leave 2 cores free for audio/DSP on RPi5
    options["num_predict"]    = 512;  // full answer, but bounds runaway length
    options["repeat_penalty"] = 1.3;  // small models loop on RPi5 — penalise repeats
    options["repeat_last_n"]  = 256;  // window the repeat penalty looks back over

    QJsonObject body;
    body["model"]      = req.modelName;
    body["stream"]     = true;   // stream tokens as they are generated
    body["keep_alive"] = "30m";  // keep weights resident across the session
    body["options"]    = options;
    body["messages"]   = m_history;

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
    QJsonObject assistantMsg;
    assistantMsg["role"]    = "assistant";
    assistantMsg["content"] = m_accumulated;
    m_history.append(assistantMsg);
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
        if (!e.name.contains("embed", Qt::CaseInsensitive))  // skip embedding-only models
            models << e.name;
    if (!models.isEmpty())
        emit modelsAvailable(models);
}

// ── Prompt builder ────────────────────────────────────────────────────────────

// Render RAG chunks as an ASCII-only background block appended to a prompt.
QString WatchExplainer::formatContextBlock(const QStringList &context) const
{
    if (context.isEmpty())
        return {};

    QString block = "\nBackground from watchmaking manual:\n";
    for (const QString &chunk : context) {
        QString clean;
        for (QChar c : chunk.left(200))
            if (c.isPrint() && c.unicode() < 128) clean += c;
        if (!clean.trimmed().isEmpty())
            block += clean.trimmed() + "\n";
    }
    return block;
}

QString WatchExplainer::buildPrompt(const ExplainRequest &req,
                                     const QStringList   &context) const
{
    const DiagnosisInput  &in  = req.input;
    const DiagnosisResult &res = req.result;

    QString watchType = (in.watch_type == WatchType::Women) ? "ladies'" : "men's";

    const QString contextBlock = formatContextBlock(context);

    // Partial-unknown: one or more metrics not yet measurable — ask AI to interpret
    if (res.level == DiagnosisLevel::Unknown) {
        QString rateStr = in.metrics.rate
            ? QString("%1 s/d").arg(*in.metrics.rate, 0, 'f', 1) : "not measurable";
        QString ampStr  = in.metrics.amplitude
            ? QString("%1 deg").arg(*in.metrics.amplitude, 0, 'f', 0) : "not measurable";
        QString beatStr = in.metrics.beatError
            ? QString("%1 ms").arg(*in.metrics.beatError, 0, 'f', 2) : "not measurable";
        QString scopeLines;
        if (in.metrics.ticTocAsymmetryDeg)
            scopeLines += QString("\nTic/Toc amplitude asymmetry: %1 deg")
                              .arg(*in.metrics.ticTocAsymmetryDeg, 0, 'f', 1);
        if (in.metrics.rateJitterMs)
            scopeLines += QString("\nRate scatter (jitter): %1 ms")
                              .arg(*in.metrics.rateJitterMs, 0, 'f', 2);
        if (in.metrics.escapementDeltaMs)
            scopeLines += QString("\nEscapement beat-to-beat variation: %1 ms")
                              .arg(*in.metrics.escapementDeltaMs, 0, 'f', 2);

        return QString(
            "You are a watchmaker. A %1 watch timegrapher reading:\n"
            "Rate %2, Amplitude %3, Beat Error %4.%5\n"
            "One or more values cannot be measured yet. Be concise — "
            "2 short sentences, under 40 words total: "
            "what mechanical condition could cause this, and what to check.%6"
        )
        .arg(watchType)
        .arg(rateStr).arg(ampStr).arg(beatStr)
        .arg(scopeLines)
        .arg(contextBlock);
    }

    QString levelStr;
    switch (res.level) {
        case DiagnosisLevel::Excellent:    levelStr = "Excellent";     break;
        case DiagnosisLevel::Good:         levelStr = "Good";          break;
        case DiagnosisLevel::NeedsService: levelStr = "Needs Service"; break;
        default:                           levelStr = "Unknown";       break;
    }

    QString scopeLines;
    if (in.metrics.ticTocAsymmetryDeg)
        scopeLines += QString("\nTic/Toc amplitude asymmetry: %1 deg")
                          .arg(*in.metrics.ticTocAsymmetryDeg, 0, 'f', 1);
    if (in.metrics.rateJitterMs)
        scopeLines += QString("\nRate scatter (jitter): %1 ms")
                          .arg(*in.metrics.rateJitterMs, 0, 'f', 2);
    if (in.metrics.escapementDeltaMs)
        scopeLines += QString("\nEscapement beat-to-beat variation: %1 ms")
                          .arg(*in.metrics.escapementDeltaMs, 0, 'f', 2);

    return QString(
        "You are a watchmaker. A %1 watch timegrapher reading:\n"
        "Rate %2 s/d, Amplitude %3 deg, Beat Error %4 ms. Diagnosis: %5.%6\n"
        "Be concise — 3 short sentences, under 60 words total: "
        "why this diagnosis, likely mechanical cause, what to service.%7"
    )
    .arg(watchType)
    .arg(in.metrics.rate.value_or(0.0),       0, 'f', 1)
    .arg(in.metrics.amplitude.value_or(0.0),  0, 'f', 0)
    .arg(in.metrics.beatError.value_or(0.0),  0, 'f', 2)
    .arg(levelStr)
    .arg(scopeLines)
    .arg(contextBlock);
}
