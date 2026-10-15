#include "ouraclient.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QTimer>
#include <QUrlQuery>
#include <QUuid>

namespace {

const char *const kAuthorize = "https://cloud.ouraring.com/oauth/authorize";
const char *const kToken = "https://api.ouraring.com/oauth/token";
const char *const kApi = "https://api.ouraring.com";
const char *const kScopes = "personal daily heartrate workout session tag spo2 heart_health";

constexpr int kRequestTimeoutMs = 60000;

struct Endpoint
{
    const char *kind;
    const char *path;
};

// The collections this app can show. daily_* are one document per day; sleep is
// the detailed period (stages, 5-minute heart rate and HRV).
const Endpoint kEndpoints[] = {
    { "daily_sleep", "/v2/usercollection/daily_sleep" },
    { "daily_readiness", "/v2/usercollection/daily_readiness" },
    { "daily_activity", "/v2/usercollection/daily_activity" },
    { "sleep", "/v2/usercollection/sleep" },
    { "daily_spo2", "/v2/usercollection/daily_spo2" },
    { "daily_stress", "/v2/usercollection/daily_stress" },
    { "daily_resilience", "/v2/usercollection/daily_resilience" },
    { "daily_cardiovascular_age", "/v2/usercollection/daily_cardiovascular_age" },
    { "workout", "/v2/usercollection/workout" },
    { "session", "/v2/usercollection/session" },
    { "tag", "/v2/usercollection/enhanced_tag" },
    { "sleep_time", "/v2/usercollection/sleep_time" },
};

QString randomState()
{
    QString id = QUuid::createUuid().toString();
    id.remove(QLatin1Char('{')).remove(QLatin1Char('}')).remove(QLatin1Char('-'));
    return id;
}

} // namespace

OuraCloudClient::OuraCloudClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_catcher, &QTcpServer::newConnection, this, &OuraCloudClient::onConnection);
}

// ---- sign-in ----------------------------------------------------------------

QUrl OuraCloudClient::beginAuthorization(QString *error)
{
    endAuthorization();
    if (!m_config.valid()) {
        if (error)
            *error = tr("Enter the client id and client secret of your Oura application first.");
        return QUrl();
    }
    if (!m_catcher.listen(QHostAddress::LocalHost, m_config.port)) {
        if (error)
            *error = tr("Could not listen on port %1 for Oura's redirect: %2. You can still paste the address "
                        "the browser ends up on.").arg(m_config.port).arg(m_catcher.errorString());
        // The sign-in itself can still proceed with the paste fallback.
    }
    m_state = randomState();
    // PKCE (RFC 7636): Oura recommends it, and it keeps a stolen code useless.
    m_verifier = randomState() + randomState();   // 64 characters from [0-9a-f]
    const QByteArray challenge = QCryptographicHash::hash(m_verifier.toLatin1(), QCryptographicHash::Sha256)
                                         .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    QUrl url{ QString::fromLatin1(kAuthorize) };
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("response_type"), QStringLiteral("code"));
    q.addQueryItem(QStringLiteral("client_id"), m_config.clientId);
    q.addQueryItem(QStringLiteral("redirect_uri"), m_config.redirectUri());
    q.addQueryItem(QStringLiteral("scope"), QString::fromLatin1(kScopes));
    q.addQueryItem(QStringLiteral("state"), m_state);
    q.addQueryItem(QStringLiteral("code_challenge"), QString::fromLatin1(challenge));
    q.addQueryItem(QStringLiteral("code_challenge_method"), QStringLiteral("S256"));
    url.setQuery(q);
    return url;
}

void OuraCloudClient::endAuthorization()
{
    if (m_catcher.isListening())
        m_catcher.close();
}

void OuraCloudClient::onConnection()
{
    while (QTcpSocket *socket = m_catcher.nextPendingConnection()) {
        connect(socket, &QTcpSocket::readyRead, socket, [this, socket]() {
            const QByteArray request = socket->readAll();
            // GET /callback?code=...&state=... HTTP/1.1
            const QList<QByteArray> parts = request.left(request.indexOf('\r')).split(' ');
            QString code;
            bool stateOk = false;
            if (parts.size() >= 2) {
                const QUrl url(QStringLiteral("http://localhost") + QString::fromLatin1(parts.at(1)));
                const QUrlQuery q(url);
                code = q.queryItemValue(QStringLiteral("code"));
                stateOk = q.queryItemValue(QStringLiteral("state")) == m_state;
            }
            const bool ok = !code.isEmpty() && stateOk;
            const QByteArray body = ok
                    ? "<html><body style='font-family:sans-serif'><h3>Oura Sync is connected</h3>"
                      "<p>You can return to the app.</p></body></html>"
                    : "<html><body style='font-family:sans-serif'><h3>Nothing to do here</h3></body></html>";
            socket->write("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\n"
                          "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" + body);
            socket->disconnectFromHost();
            if (ok)
                emit authorizationReceived(code);
        });
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    }
}

QString OuraCloudClient::completeAuthorization(const QString &codeOrUrl)
{
    QString code = codeOrUrl.trimmed();
    if (code.contains(QLatin1String("code="))) {
        // The whole address was pasted. Its state must be ours.
        const QUrl url(code.startsWith(QLatin1String("http")) ? code : QStringLiteral("http://localhost/?") + code);
        const QUrlQuery q(url);
        code = q.queryItemValue(QStringLiteral("code"));
        const QString state = q.queryItemValue(QStringLiteral("state"));
        if (!state.isEmpty() && !m_state.isEmpty() && state != m_state)
            return tr("That address belongs to a different sign-in. Start again.");
    }
    if (code.isEmpty())
        return tr("There is no authorization code in what was entered.");

    QUrlQuery form;
    form.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("authorization_code"));
    form.addQueryItem(QStringLiteral("code"), code);
    form.addQueryItem(QStringLiteral("redirect_uri"), m_config.redirectUri());
    form.addQueryItem(QStringLiteral("client_id"), m_config.clientId);
    form.addQueryItem(QStringLiteral("client_secret"), m_config.clientSecret);
    if (!m_verifier.isEmpty())
        form.addQueryItem(QStringLiteral("code_verifier"), m_verifier);

    int status = 0;
    QString error;
    const QByteArray body = exchange(QUrl(QString::fromLatin1(kToken)),
                                     form.toString(QUrl::FullyEncoded).toUtf8(), status, error, false);
    if (!error.isEmpty())
        return error;
    if (status != 200)
        return tr("Oura did not accept the code (HTTP %1): %2").arg(status).arg(QString::fromUtf8(body.left(200)));
    endAuthorization();
    return storeTokenReply(body, status);
}

namespace {

// The value of a top-level "key": "string" pair, found without a JSON parser.
QString looseString(const QString &text, const QString &key)
{
    const QRegularExpression re(QStringLiteral("\"%1\"\\s*:\\s*\"([^\"]*)\"").arg(QRegularExpression::escape(key)));
    const QRegularExpressionMatch m = re.match(text);
    return m.hasMatch() ? m.captured(1) : QString();
}

double looseNumber(const QString &text, const QString &key, double fallback)
{
    const QRegularExpression re(QStringLiteral("\"%1\"\\s*:\\s*(-?[0-9.eE+]+)").arg(QRegularExpression::escape(key)));
    const QRegularExpressionMatch m = re.match(text);
    return m.hasMatch() ? m.captured(1).toDouble() : fallback;
}

} // namespace

QString OuraCloudClient::storeTokenReply(const QByteArray &body, int status)
{
    Tokens t;
    double expiresIn = 86400;
    size_t errorAt = 0;
    const auto json = Oura::Json::parse(body.toStdString(), &errorAt);
    if (json && !json->str("access_token").empty()) {
        t.access = QString::fromStdString(json->str("access_token"));
        t.refresh = QString::fromStdString(json->str("refresh_token"));
        expiresIn = json->num("expires_in", expiresIn);
    } else {
        // Oura's reply carried a token that the strict parser would not take.
        // The reply is a flat object of strings and numbers, so read the three
        // fields that matter directly rather than refuse a good sign-in.
        const QString text = QString::fromUtf8(body);
        qWarning("ourasync: strict JSON parse of the token reply failed at byte %llu of %d (json=%d)",
                 static_cast<unsigned long long>(errorAt), body.size(), json ? 1 : 0);
        t.access = looseString(text, QStringLiteral("access_token"));
        t.refresh = looseString(text, QStringLiteral("refresh_token"));
        expiresIn = looseNumber(text, QStringLiteral("expires_in"), expiresIn);
        if (t.access.isEmpty()) {
            // Say what did come back, with any token values blanked.
            QString shown = text.left(300);
            shown.replace(QRegularExpression(QStringLiteral("(\"[A-Za-z_]*token[A-Za-z_]*\"\\s*:\\s*\")[^\"]*")),
                          QStringLiteral("\\1…"));
            return tr("Oura's reply did not contain a token (HTTP %1, parse error at byte %2): %3")
                    .arg(status).arg(static_cast<qulonglong>(errorAt)).arg(shown.isEmpty() ? tr("(empty)") : shown);
        }
    }
    if (t.refresh.isEmpty())
        t.refresh = m_tokens.refresh;
    t.expiresUtc = QDateTime::currentMSecsSinceEpoch() / 1000 + static_cast<qint64>(expiresIn);
    m_tokens = t;
    emit tokensChanged();
    return QString();
}

QString OuraCloudClient::refreshTokens()
{
    if (m_tokens.refresh.isEmpty())
        return tr("Not connected to Oura. Connect again.");
    QUrlQuery form;
    form.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("refresh_token"));
    form.addQueryItem(QStringLiteral("refresh_token"), m_tokens.refresh);
    form.addQueryItem(QStringLiteral("client_id"), m_config.clientId);
    form.addQueryItem(QStringLiteral("client_secret"), m_config.clientSecret);
    int status = 0;
    QString error;
    const QByteArray body = exchange(QUrl(QString::fromLatin1(kToken)),
                                     form.toString(QUrl::FullyEncoded).toUtf8(), status, error, false);
    if (!error.isEmpty())
        return error;
    if (status != 200)
        return tr("Oura refused to renew the sign-in (HTTP %1). Connect again.").arg(status);
    return storeTokenReply(body, status);
}

QString OuraCloudClient::ensureAccessToken()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000;
    if (!m_tokens.access.isEmpty() && m_tokens.expiresUtc > now + 60)
        return QString();
    return refreshTokens();
}

// ---- HTTP -------------------------------------------------------------------

QByteArray OuraCloudClient::exchange(const QUrl &url, const QByteArray &post, int &status, QString &error,
                                     bool bearer)
{
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    if (bearer)
        request.setRawHeader("Authorization", "Bearer " + m_tokens.access.toUtf8());
    QNetworkReply *reply;
    if (post.isNull()) {
        reply = m_network.get(request);
    } else {
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
        reply = m_network.post(request, post);
    }

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timeout.start(kRequestTimeoutMs);
    loop.exec();

    if (!reply->isFinished()) {
        reply->abort();
        reply->deleteLater();
        error = tr("Oura did not answer in time. Check the network connection.");
        status = 0;
        return QByteArray();
    }
    status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();
    if (status == 0)
        error = tr("Could not reach Oura: %1").arg(reply->errorString());
    reply->deleteLater();
    return body;
}

QString OuraCloudClient::getJson(const QUrl &url, Oura::Json &out, int &status)
{
    QString error = ensureAccessToken();
    if (!error.isEmpty())
        return error;
    for (int attempt = 0; attempt < 2; ++attempt) {
        const QByteArray body = exchange(url, QByteArray(), status, error, true);
        if (!error.isEmpty())
            return error;
        if (status == 401 && attempt == 0) {
            error = refreshTokens();
            if (!error.isEmpty())
                return error;
            continue;
        }
        if (status == 200) {
            const auto json = Oura::Json::parse(body.toStdString());
            if (!json)
                return tr("Oura's reply could not be read.");
            out = *json;
            return QString();
        }
        return QString(); // other statuses are for the caller to judge
    }
    return QString();
}

// ---- fetching ---------------------------------------------------------------

QString OuraCloudClient::fetchPersonal(Personal &out)
{
    Oura::Json reply;
    int status = 0;
    const QString error = getJson(QUrl(QString::fromLatin1(kApi) + QStringLiteral("/v2/usercollection/personal_info")),
                                  reply, status);
    if (!error.isEmpty())
        return error;
    if (status != 200)
        return tr("Oura answered HTTP %1 for the profile.").arg(status);
    if (auto age = reply.optNum("age"); age && *age >= 5 && *age <= 120)
        out.age = static_cast<int>(*age);
    if (auto weight = reply.optNum("weight"); weight && *weight >= 20 && *weight <= 400)
        out.weightKg = *weight;
    out.sex = QString::fromStdString(reply.str("biological_sex"));
    return QString();
}

void OuraCloudClient::revokeAccess()
{
    if (m_tokens.access.isEmpty())
        return;
    QUrl url(QStringLiteral("https://api.ouraring.com/oauth/revoke"));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("access_token"), m_tokens.access);
    url.setQuery(q);
    int status = 0;
    QString error;
    exchange(url, QByteArray(""), status, error, false);
}

QString OuraCloudClient::fetch(const FetchRequest &request, Oura::CloudImport &out,
                               const std::function<void(const QString &)> &progress)
{
    if (!m_config.valid() || !m_tokens.valid())
        return tr("Not connected to Oura.");

    QStringList skipped;
    bool anyAnswered = false;
    for (const Endpoint &e : kEndpoints) {
        progress(tr("Fetching %1…").arg(QString::fromLatin1(e.kind)));
        QString next;
        int pages = 0;
        do {
            QUrl url(QString::fromLatin1(kApi) + QString::fromLatin1(e.path));
            QUrlQuery q;
            q.addQueryItem(QStringLiteral("start_date"), request.startDay);
            q.addQueryItem(QStringLiteral("end_date"), request.endDay);
            if (!next.isEmpty())
                q.addQueryItem(QStringLiteral("next_token"), next);
            url.setQuery(q);

            Oura::Json reply;
            int status = 0;
            const QString error = getJson(url, reply, status);
            if (!error.isEmpty())
                return error;
            // 403 is a refused collection (no membership, or the scope was unticked at
            // sign-in). Oura answers 401 for a collection whose scope the sign-in
            // does not cover, too, even though the token itself is good: getJson
            // has already renewed it once, and other collections have answered.
            if (status == 403 || (status == 401 && anyAnswered)) {
                skipped << QString::fromLatin1(e.kind);
                break;
            }
            if (status == 401)
                return tr("Oura did not accept the sign-in (HTTP 401). Disconnect and connect again.");
            if (status == 429)
                return tr("Oura says too many requests were made. Try again in a few minutes.");
            if (status == 404) {
                break; // a collection this account does not have
            }
            if (status != 200)
                return tr("Oura answered HTTP %1 for %2.").arg(status).arg(QString::fromLatin1(e.kind));
            anyAnswered = true;
            next = QString::fromStdString(Oura::addApiPage(out, e.kind, reply));
        } while (!next.isEmpty() && ++pages < 500);
    }

    if (request.heartRate) {
        // Heart rate comes in windows: a day of five-minute samples is small, a
        // year is not. One week at a time.
        const QDateTime start = QDateTime::fromString(request.startDay, Qt::ISODate);
        const QDateTime end = QDateTime::fromString(request.endDay, Qt::ISODate).addDays(1);
        for (QDateTime from = start; from < end; from = from.addDays(7)) {
            const QDateTime to = qMin(from.addDays(7), end);
            progress(tr("Fetching heart rate from %1…").arg(from.toString(Qt::ISODate).left(10)));
            QString next;
            int pages = 0;
            do {
                QUrl url(QString::fromLatin1(kApi) + QStringLiteral("/v2/usercollection/heartrate"));
                QUrlQuery q;
                q.addQueryItem(QStringLiteral("start_datetime"), from.toUTC().toString(QStringLiteral("yyyy-MM-ddT00:00:00+00:00")));
                q.addQueryItem(QStringLiteral("end_datetime"), to.toUTC().toString(QStringLiteral("yyyy-MM-ddT00:00:00+00:00")));
                if (!next.isEmpty())
                    q.addQueryItem(QStringLiteral("next_token"), next);
                url.setQuery(q);
                Oura::Json reply;
                int status = 0;
                const QString error = getJson(url, reply, status);
                if (!error.isEmpty())
                    return error;
                if (status == 403 || (status == 401 && anyAnswered)) {
                    skipped << QStringLiteral("heartrate");
                    from = end; // the whole series is refused
                    break;
                }
                if (status == 429)
                    return tr("Oura says too many requests were made. Try again in a few minutes.");
                if (status != 200)
                    break;
                if (const Oura::Json *data = reply.get("data")) {
                    for (const Oura::Json &p : data->items()) {
                        const auto t = Oura::parseIso8601(p.str("timestamp"));
                        const int bpm = static_cast<int>(p.num("bpm"));
                        if (t && bpm >= 25 && bpm <= 250)
                            out.hr.push_back({ *t, bpm });
                    }
                }
                const Oura::Json *n = reply.get("next_token");
                next = n && n->isString() ? QString::fromStdString(n->asString()) : QString();
            } while (!next.isEmpty() && ++pages < 200);
        }
    }

    if (!skipped.isEmpty())
        out.notes.push_back(tr("Oura refused: %1 (no membership, or the permission was declined).")
                                    .arg(skipped.join(QStringLiteral(", "))).toStdString());
    return QString();
}
