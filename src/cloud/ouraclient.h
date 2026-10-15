#pragma once

#include "clouddoc.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QTcpServer>
#include <QUrl>

#include <functional>

// The Oura Cloud API v2, for people who have API access.
//
// What this needs, and why it is more than a token field: Oura retired personal
// access tokens at the end of 2025. Every client now signs in with OAuth 2.0
// through an *application the user registers themselves* at
// developer.ouraring.com/applications (a client id, a client secret and a
// redirect URI), and Gen 3 and later rings need an active Oura Membership for the
// API to serve their data. A ring owner without a membership can still use the
// data export (see exportimport.h), which needs neither.
//
// The sign-in runs in the phone's own browser. Oura redirects to
// http://localhost:<port>/callback, which a tiny listener in this app catches;
// pasting the address from the browser's address bar is the fallback when the
// listener could not be reached.
//
// Like RingLink, the calls that wait do so inside a nested event loop, so the
// code that uses them reads top to bottom.
class OuraCloudClient : public QObject
{
    Q_OBJECT
public:
    struct Config
    {
        QString clientId;
        QString clientSecret;
        quint16 port = 8421;

        QString redirectUri() const { return QStringLiteral("http://localhost:%1/callback").arg(port); }
        bool valid() const { return !clientId.isEmpty() && !clientSecret.isEmpty(); }
    };

    struct Tokens
    {
        QString access;
        QString refresh;
        qint64 expiresUtc = 0;

        bool valid() const { return !refresh.isEmpty() || !access.isEmpty(); }
    };

    explicit OuraCloudClient(QObject *parent = nullptr);

    void setConfig(const Config &config) { m_config = config; }
    Config config() const { return m_config; }
    void setTokens(const Tokens &tokens) { m_tokens = tokens; }
    Tokens tokens() const { return m_tokens; }
    bool connected() const { return m_tokens.valid(); }

    // Starts the local listener and returns the URL to open in a browser. Empty
    // (with `error` set) when the port is taken.
    QUrl beginAuthorization(QString *error);
    void endAuthorization();

    // `codeOrUrl` is the authorization code, or the whole address the browser
    // was redirected to.
    QString completeAuthorization(const QString &codeOrUrl);

    struct FetchRequest
    {
        QString startDay; // YYYY-MM-DD
        QString endDay;
        bool heartRate = true;
    };
    // Empty on success, else a message for a person.
    QString fetch(const FetchRequest &request, Oura::CloudImport &out,
                  const std::function<void(const QString &)> &progress);

signals:
    // New tokens: the caller must persist them at once, a refresh token is single use.
    void tokensChanged();
    void authorizationReceived(const QString &code);

private slots:
    void onConnection();

private:
    QString ensureAccessToken();
    QString refreshTokens();
    QString storeTokenReply(const QByteArray &body);
    // GET with the bearer token; 401 triggers one refresh and a retry.
    QString getJson(const QUrl &url, Oura::Json &out, int &status);
    QByteArray exchange(const QUrl &url, const QByteArray &post, int &status, QString &error, bool bearer);

    Config m_config;
    Tokens m_tokens;
    QNetworkAccessManager m_network;
    QTcpServer m_catcher;
    QString m_state;
    QString m_verifier;   // PKCE code_verifier of the sign-in in progress
};
