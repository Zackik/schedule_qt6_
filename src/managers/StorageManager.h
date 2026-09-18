#pragma once
#include <QObject>
#include <QString>

class StorageManager : public QObject {
    Q_OBJECT
public:
    explicit StorageManager(QObject *parent = nullptr);

    qint64 calculateDirectorySize(const QString &path) const;
    qint64 databaseSizeBytes() const;
    qint64 cacheSizeBytes() const;
    qint64 logsSizeBytes() const;
    qint64 applicationDataSizeBytes() const;
    qint64 totalStorageUsed() const;

    bool cleanupOldLogs(int daysToKeep = 30);
    bool clearCache();

private:
    QString m_appDataPath;
    QString m_cachePath;
    QString m_dbPath;
};
