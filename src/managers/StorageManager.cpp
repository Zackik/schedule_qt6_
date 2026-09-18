#include "StorageManager.h"

#include <spdlog/spdlog.h>

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

StorageManager::StorageManager(QObject *parent) : QObject(parent) {
    m_appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    m_cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_dbPath = m_appDataPath + "/smartschedule.db";

    // Ensure directories exist
    QDir().mkpath(m_appDataPath);
    QDir().mkpath(m_cachePath);
}

qint64 StorageManager::calculateDirectorySize(const QString &path) const {
    qint64 size = 0;
    QDir dir(path);
    if (!dir.exists())
        return 0;

    QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        size += it.fileInfo().size();
    }
    return size;
}

qint64 StorageManager::databaseSizeBytes() const {
    QFileInfo dbInfo(m_dbPath);
    if (dbInfo.exists()) {
        return dbInfo.size();
    }
    return 0;
}

qint64 StorageManager::cacheSizeBytes() const {
    return calculateDirectorySize(m_cachePath);
}

qint64 StorageManager::logsSizeBytes() const {
    // Assuming logs are stored in a specific logs directory
    return calculateDirectorySize(m_appDataPath + "/logs");
}

qint64 StorageManager::applicationDataSizeBytes() const {
    return calculateDirectorySize(m_appDataPath);
}

qint64 StorageManager::totalStorageUsed() const {
    return applicationDataSizeBytes() + cacheSizeBytes();
}

bool StorageManager::cleanupOldLogs(int daysToKeep) {
    // 1. Cleanup database logs
    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        QSqlQuery query;
        QDateTime cutoff = QDateTime::currentDateTime().addDays(-daysToKeep);
        query.prepare("DELETE FROM ai_interaction_logs WHERE created_at < :cutoff");
        query.bindValue(":cutoff", cutoff.toString(Qt::ISODate));
        if (!query.exec()) {
            spdlog::error("Failed to delete old db logs: {}",
                          query.lastError().text().toStdString());
        } else {
            spdlog::info("Cleaned up database AI logs older than {} days.", daysToKeep);
        }
    }

    // 2. Cleanup file logs
    QString logsPath = m_appDataPath + "/logs";
    QDir logsDir(logsPath);
    if (logsDir.exists()) {
        QFileInfoList entries = logsDir.entryInfoList(QDir::Files);
        QDateTime cutoff = QDateTime::currentDateTime().addDays(-daysToKeep);
        for (const QFileInfo &fileInfo : entries) {
            if (fileInfo.lastModified() < cutoff) {
                QFile::remove(fileInfo.absoluteFilePath());
            }
        }
        spdlog::info("Cleaned up old log files.");
    }
    return true;
}

bool StorageManager::clearCache() {
    QDir cacheDir(m_cachePath);
    if (cacheDir.exists()) {
        cacheDir.removeRecursively();
        cacheDir.mkpath(".");
        spdlog::info("Cleared application cache.");
        return true;
    }
    return false;
}
