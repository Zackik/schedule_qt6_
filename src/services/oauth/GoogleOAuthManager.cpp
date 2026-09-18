#include "GoogleOAuthManager.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QDateTime>

GoogleOAuthManager::GoogleOAuthManager(QObject *parent)
    : QObject(parent), m_state(State::DISCONNECTED)
{
    m_oauth = new QOAuth2AuthorizationCodeFlow(this);
    m_replyHandler = new QOAuthHttpServerReplyHandler(this);
    m_oauth->setReplyHandler(m_replyHandler);

    // Default Google OAuth Endpoints
    m_oauth->setAuthorizationUrl(QUrl("https://accounts.google.com/o/oauth2/v2/auth"));
    m_oauth->setAccessTokenUrl(QUrl("https://oauth2.googleapis.com/token"));
    m_oauth->setScope("https://www.googleapis.com/auth/calendar.events https://www.googleapis.com/auth/userinfo.email");

    m_oauth->setModifyParametersFunction([](QAbstractOAuth::Stage stage, auto *parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAuthorization) {
            parameters->insert("access_type", "offline");
            parameters->insert("prompt", "consent");
        }
    });

    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::statusChanged, this, &GoogleOAuthManager::onOAuthStatusChanged);
    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);
    connect(m_oauth, &QAbstractOAuth::requestFailed, this, &GoogleOAuthManager::onQtNetworkAuthError);
    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::tokenChanged, this, &GoogleOAuthManager::onTokenChanged);
}

GoogleOAuthManager::~GoogleOAuthManager() = default;

void GoogleOAuthManager::setup(const QString &clientId, const QString &clientSecret, quint16 port)
{
    m_oauth->setClientIdentifier(clientId);
    m_oauth->setClientIdentifierSharedKey(clientSecret);
    
    // Ensure the reply handler is listening on the expected port
    if (m_replyHandler->port() != port || !m_replyHandler->isListening()) {
        m_replyHandler->listen(QHostAddress::Any, port);
    }
}

void GoogleOAuthManager::startAuthorization()
{
    if (m_state == State::CONNECTED || m_state == State::AUTHORIZING) {
        return; // Already connected or in progress
    }
    
    changeState(State::AUTHORIZING);
    
    if (!m_replyHandler->isListening()) {
        emit authFailed("Callback server failed to listen. Port may be in use.");
        changeState(State::INVALID);
        return;
    }
    
    m_oauth->grant();
}

void GoogleOAuthManager::handleCallback()
{
    // In QtNetworkAuth, the QOAuthHttpServerReplyHandler automatically intercepts 
    // the HTTP callback on the local port, extracts the authorization code, 
    // and requests the access token. 
    // This method exists to fulfill architectural design requirements.
    qDebug() << "GoogleOAuthManager::handleCallback - Handled implicitly by QOAuthHttpServerReplyHandler on port:" << m_replyHandler->port();
}

void GoogleOAuthManager::refreshAccessToken()
{
    if (m_state == State::DISCONNECTED || m_state == State::INVALID) {
        qWarning() << "Cannot refresh token from state:" << static_cast<int>(m_state);
        return;
    }
    
    changeState(State::REFRESHING);
    m_oauth->refreshAccessToken();
}

void GoogleOAuthManager::logout()
{
    m_oauth->setToken(QString());
    changeState(State::DISCONNECTED);
}

GoogleOAuthManager::State GoogleOAuthManager::currentState() const
{
    return m_state;
}

QString GoogleOAuthManager::accessToken() const
{
    return m_oauth->token();
}

bool GoogleOAuthManager::isAuthenticated() const
{
    return m_state == State::CONNECTED;
}

void GoogleOAuthManager::onOAuthStatusChanged(QAbstractOAuth::Status status)
{
    switch (status) {
        case QAbstractOAuth::Status::Granted:
            // Flow: AUTHORIZING/REFRESHING -> AUTHORIZED -> CONNECTED
            // Qt directly jumps to Granted once the token is received.
            changeState(State::AUTHORIZED);
            changeState(State::CONNECTED);
            emit authSuccess();
            break;
            
        case QAbstractOAuth::Status::NotAuthenticated:
            changeState(State::DISCONNECTED);
            break;
            
        case QAbstractOAuth::Status::TemporaryCredentialsReceived:
            // Mainly used for OAuth 1.0, not highly relevant for Google OAuth 2.0
            break;

        case QAbstractOAuth::Status::RefreshingToken:
            changeState(State::REFRESHING);
            break;
    }
}

void GoogleOAuthManager::onTokenChanged(const QString &token)
{
    if (token.isEmpty() && m_state == State::CONNECTED) {
        changeState(State::INVALID);
    }
}

void GoogleOAuthManager::onQtNetworkAuthError(QAbstractOAuth::Error error)
{
    QString errorMsg = QString("OAuth Request Failed. Code: %1").arg(static_cast<int>(error));
    onOAuthError(errorMsg);
}

void GoogleOAuthManager::onOAuthError(const QString &error)
{
    qWarning() << "GoogleOAuthManager Error:" << error;
    emit authFailed(error);
    changeState(State::INVALID);
}

void GoogleOAuthManager::changeState(State newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(m_state);
        
        qDebug() << "OAuth State Transitioned to:" << static_cast<int>(m_state);
    }
}
