#pragma once

#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include "../services/oauth/GoogleOAuthManager.h"

class ResourceMonitor;
class StorageManager;

class SettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit SettingsWidget(class CalendarManager *calendarManager, QWidget *parent = nullptr);

    GoogleOAuthManager* oauthManager() const { return m_oauthManager; }

private slots:
    void updateResourceUI(double mb);
    void updateStorageUI();
    void onCleanupClicked();
    void onGoogleAuthClicked();
    void onOAuthStateChanged(GoogleOAuthManager::State newState);
    void onOAuthFailed(const QString &errorReason);

private:
    void setupUI();

    ResourceMonitor *m_resourceMonitor;
    StorageManager *m_storageManager;
    GoogleOAuthManager *m_oauthManager;

    QLabel *m_ramUsageLabel;
    QLabel *m_appDataLabel;
    QLabel *m_dbSizeLabel;
    QLabel *m_aiLogsLabel;
    QLabel *m_cacheSizeLabel;
    QLabel *m_totalStorageLabel;

    QPushButton *m_refreshBtn;
    QPushButton *m_cleanupBtn;

    QLabel *m_googleStatusLabel;
    QPushButton *m_googleAuthBtn;
};
