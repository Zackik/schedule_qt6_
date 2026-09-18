#include "../calendar/CalendarManager.h"
#include "SettingsWidget.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QString>
#include <QVBoxLayout>
#include <QSettings>

#include "../managers/StorageManager.h"
#include "../services/ResourceMonitor.h"

SettingsWidget::SettingsWidget(CalendarManager *calendarManager, QWidget *parent) : QWidget(parent) {
    m_resourceMonitor = new ResourceMonitor(this);
    m_storageManager = new StorageManager(this);
    m_oauthManager = calendarManager->oauthManager();
    if (!m_oauthManager) {
        calendarManager->setupGoogleOAuth("","");
        m_oauthManager = calendarManager->oauthManager();
    }

    setupUI();

    connect(m_resourceMonitor, &ResourceMonitor::memoryUsageUpdated, this,
            &SettingsWidget::updateResourceUI);
    connect(m_refreshBtn, &QPushButton::clicked, this, &SettingsWidget::updateStorageUI);
    connect(m_cleanupBtn, &QPushButton::clicked, this, &SettingsWidget::onCleanupClicked);
    
    connect(m_googleAuthBtn, &QPushButton::clicked, this, &SettingsWidget::onGoogleAuthClicked);
    connect(m_oauthManager, &GoogleOAuthManager::stateChanged, this, &SettingsWidget::onOAuthStateChanged);
    connect(m_oauthManager, &GoogleOAuthManager::authFailed, this, &SettingsWidget::onOAuthFailed);

    updateStorageUI();
    
    // Auto-check connection status on startup if credentials exist
    QSettings settings;
    QString cId = settings.value("google_client_id", "").toString();
    QString cSecret = settings.value("google_client_secret", "").toString();
    if (!cId.isEmpty() && !cSecret.isEmpty()) {
        m_oauthManager->setup(cId, cSecret, 8080);
    }
}

void SettingsWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // --- Google OAuth Group ---
    QGroupBox *oauthGroup = new QGroupBox(tr("🌐 Google Calendar OAuth 2.0"), this);
    QFormLayout *oauthLayout = new QFormLayout(oauthGroup);
    
    m_googleStatusLabel = new QLabel(tr("Disconnected"), this);
    m_googleStatusLabel->setStyleSheet("font-weight: bold; color: gray;");
    
    m_googleAuthBtn = new QPushButton(tr("🔑 Kết nối Google Calendar"), this);
    m_googleAuthBtn->setStyleSheet("background-color: #ea4335; color: white; font-weight: bold; padding: 6px; border-radius: 4px;");
    
    oauthLayout->addRow(tr("Trạng thái:"), m_googleStatusLabel);
    oauthLayout->addRow(m_googleAuthBtn);
    
    mainLayout->addWidget(oauthGroup);

    // --- Resource Group ---
    QGroupBox *resourceGroup = new QGroupBox(tr("Quản lý Dữ liệu & Tài nguyên"), this);
    QFormLayout *formLayout = new QFormLayout(resourceGroup);

    m_ramUsageLabel = new QLabel(tr("Đang tính toán..."), this);
    m_appDataLabel = new QLabel(tr("Đang tính toán..."), this);
    m_dbSizeLabel = new QLabel(tr("Đang tính toán..."), this);
    m_aiLogsLabel = new QLabel(tr("Đang tính toán..."), this);
    m_cacheSizeLabel = new QLabel(tr("Đang tính toán..."), this);
    m_totalStorageLabel = new QLabel(tr("Đang tính toán..."), this);

    formLayout->addRow(tr("RAM Usage:"), m_ramUsageLabel);
    formLayout->addRow(tr("Application Data:"), m_appDataLabel);
    formLayout->addRow(tr("SQLite Database:"), m_dbSizeLabel);
    formLayout->addRow(tr("AI Logs:"), m_aiLogsLabel);
    formLayout->addRow(tr("Cache:"), m_cacheSizeLabel);
    formLayout->addRow(tr("Total Storage:"), m_totalStorageLabel);

    m_refreshBtn = new QPushButton(tr("Làm mới"), this);
    formLayout->addRow(m_refreshBtn);
    mainLayout->addWidget(resourceGroup);

    m_cleanupBtn = new QPushButton(tr("Xóa Lịch sử & Nhật ký"), this);
    m_cleanupBtn->setStyleSheet("color: red;");
    mainLayout->addWidget(m_cleanupBtn);

    QLabel *privacyLabel =
        new QLabel(tr("All resource and storage information is measured locally.\nNo resource "
                      "statistics are transmitted to external servers."),
                   this);
    privacyLabel->setStyleSheet("color: gray; font-size: 10px;");
    mainLayout->addWidget(privacyLabel);

    mainLayout->addStretch();
}

void SettingsWidget::updateResourceUI(double mb) {
    m_ramUsageLabel->setText(QString::number(mb, 'f', 1) + " MB");
}

void SettingsWidget::updateStorageUI() {
    auto toMB = [](qint64 bytes) -> QString {
        return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
    };
    m_appDataLabel->setText(toMB(m_storageManager->applicationDataSizeBytes()));
    m_dbSizeLabel->setText(toMB(m_storageManager->databaseSizeBytes()));
    m_cacheSizeLabel->setText(toMB(m_storageManager->cacheSizeBytes()));
    m_aiLogsLabel->setText(toMB(m_storageManager->logsSizeBytes()));
    m_totalStorageLabel->setText(toMB(m_storageManager->totalStorageUsed()));
}

void SettingsWidget::onCleanupClicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(
        this, tr("Xác nhận xóa"),
        tr("Thao tác này sẽ xóa:\n- AI interaction logs\n- old reports\n- cache\n- temporary "
           "data\n\nCác sự kiện lịch và task sẽ KHÔNG bị xóa.\n\nBạn có chắc chắn muốn tiếp tục?"),
        QMessageBox::Cancel | QMessageBox::Ok);

    if (reply == QMessageBox::Ok) {
        m_storageManager->cleanupOldLogs(30);
        m_storageManager->clearCache();
        updateStorageUI();
        QMessageBox::information(this, tr("Thành công"), tr("Đã dọn dẹp dung lượng hệ thống."));
    }
}

void SettingsWidget::onGoogleAuthClicked() {
    QSettings settings;
    QString cId = settings.value("google_client_id", "").toString();
    QString cSecret = settings.value("google_client_secret", "").toString();

    if (cId.isEmpty() || cSecret.isEmpty()) {
        QMessageBox::warning(this, tr("Chưa cấu hình API"), tr("Vui lòng nhập Client ID và Secret ở phần cài đặt API."));
        return;
    }

    m_oauthManager->setup(cId, cSecret, 8080);
    m_oauthManager->startAuthorization();
}

void SettingsWidget::onOAuthStateChanged(GoogleOAuthManager::State newState) {
    switch (newState) {
        case GoogleOAuthManager::State::DISCONNECTED:
            m_googleStatusLabel->setText(tr("Disconnected"));
            m_googleStatusLabel->setStyleSheet("font-weight: bold; color: gray;");
            break;
        case GoogleOAuthManager::State::AUTHORIZING:
            m_googleStatusLabel->setText(tr("Authorizing..."));
            m_googleStatusLabel->setStyleSheet("font-weight: bold; color: #f39c12;");
            break;
        case GoogleOAuthManager::State::AUTHORIZED:
            m_googleStatusLabel->setText(tr("Authorized"));
            m_googleStatusLabel->setStyleSheet("font-weight: bold; color: #2980b9;");
            break;
        case GoogleOAuthManager::State::CONNECTED:
            m_googleStatusLabel->setText(tr("Connected"));
            m_googleStatusLabel->setStyleSheet("font-weight: bold; color: #27ae60;");
            break;
        case GoogleOAuthManager::State::REFRESHING:
            m_googleStatusLabel->setText(tr("Refreshing Token..."));
            m_googleStatusLabel->setStyleSheet("font-weight: bold; color: #8e44ad;");
            break;
        case GoogleOAuthManager::State::INVALID:
            m_googleStatusLabel->setText(tr("Invalid Token / Error"));
            m_googleStatusLabel->setStyleSheet("font-weight: bold; color: #e74c3c;");
            break;
    }
}

void SettingsWidget::onOAuthFailed(const QString &errorReason) {
    QMessageBox::critical(this, tr("Lỗi Xác Thực"), tr("Không thể kết nối Google Calendar:\n%1").arg(errorReason));
}
