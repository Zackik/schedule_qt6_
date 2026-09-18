#!/bin/bash
cat << 'INNER_EOF' > auth_patch.txt
void CalendarManager::setupGoogleOAuth(const QString &clientId, const QString &clientSecret) {
    if (!m_oauthManager) {
        m_oauthManager = new GoogleOAuthManager(this);
        
        connect(m_oauthManager, &GoogleOAuthManager::stateChanged, this, [this](GoogleOAuthManager::State state) {
            if (state == GoogleOAuthManager::State::CONNECTED) {
                emit googleAuthStatusChanged(true, "Đã xác thực Google thành công");
                fetchGoogleCalendarEvents();
            } else if (state == GoogleOAuthManager::State::DISCONNECTED || state == GoogleOAuthManager::State::INVALID) {
                emit googleAuthStatusChanged(false, "Chưa đăng nhập");
            }
        });
        connect(m_oauthManager, &GoogleOAuthManager::authFailed, this, [this](const QString &error) {
            emit googleAuthStatusChanged(false, "Lỗi: " + error);
        });
    }
    m_oauthManager->setup(clientId, clientSecret, 8080);
}

void CalendarManager::authenticateGoogle() {
    if (!m_oauthManager) {
        QString clientId = qEnvironmentVariable("GOOGLE_CLIENT_ID");
        QString clientSecret = qEnvironmentVariable("GOOGLE_CLIENT_SECRET");
        if (clientId.isEmpty() || clientSecret.isEmpty()) {
            qWarning() << "Google OAuth Client ID or Secret is missing in environment variables.";
            return;
        }
        setupGoogleOAuth(clientId, clientSecret);
        if (!m_oauthManager) return;
    }
    m_oauthManager->startAuthorization();
}

bool CalendarManager::isGoogleAuthenticated() const {
    return m_oauthManager && m_oauthManager->isAuthenticated();
}
INNER_EOF

# Replace lines 66 to 121 in CalendarManager.cpp
sed -i '66,121c\' src/calendar/CalendarManager.cpp
sed -i '65r auth_patch.txt' src/calendar/CalendarManager.cpp

