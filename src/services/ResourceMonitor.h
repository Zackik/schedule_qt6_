#pragma once
#include <QObject>
#include <QTimer>

class ResourceMonitor : public QObject {
    Q_OBJECT
public:
    explicit ResourceMonitor(QObject *parent = nullptr);
    double memoryUsageMB() const;

signals:
    void memoryUsageUpdated(double mb);

private slots:
    void refresh();

private:
    QTimer *m_timer;
    double m_currentMemoryMB;
};
