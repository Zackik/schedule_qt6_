#pragma once

#include <QObject>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QString>

/**
 * @brief Manages the Google OAuth 2.0 flow using a strict state machine.
 */
class GoogleOAuthManager : public QObject {
    Q_OBJECT
public:
    enum class State {
        DISCONNECTED,
        AUTHORIZING,
        AUTHORIZED,
        CONNECTED,
        REFRESHING,
        INVALID
    };
    Q_ENUM(State)

    explicit GoogleOAuthManager(QObject *parent = nullptr);
    ~GoogleOAuthManager() override;

    void setup(const QString &clientId, const QString &clientSecret, quint16 port = 8080);
    
    // Core OAuth Actions
    void startAuthorization();
    void handleCallback();
    void refreshAccessToken();
    void logout();

    // Getters
    State currentState() const;
    QString accessToken() const;
    bool isAuthenticated() const;

signals:
    void stateChanged(GoogleOAuthManager::State newState);
    void authSuccess();
    void authFailed(const QString &errorReason);

private slots:
    void onOAuthStatusChanged(QAbstractOAuth::Status status);
    void onOAuthError(const QString &error);
    void onQtNetworkAuthError(QAbstractOAuth::Error error);
    void onTokenChanged(const QString &token);

private:
    void changeState(State newState);

    QOAuth2AuthorizationCodeFlow *m_oauth;
    QOAuthHttpServerReplyHandler *m_replyHandler;
    State m_state;
};
