#include "ResourceMonitor.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

#ifdef Q_OS_LINUX
// Linux specifics are handled via /proc
#elif defined(Q_OS_WIN)
#include <psapi.h>
#include <windows.h>
#elif defined(Q_OS_MAC)
#include <mach/mach.h>
#endif

ResourceMonitor::ResourceMonitor(QObject *parent) : QObject(parent), m_currentMemoryMB(0.0) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ResourceMonitor::refresh);
    m_timer->start(5000);  // 5 seconds
    refresh();
}

void ResourceMonitor::refresh() {
    m_currentMemoryMB = memoryUsageMB();
    emit memoryUsageUpdated(m_currentMemoryMB);
}

double ResourceMonitor::memoryUsageMB() const {
#ifdef Q_OS_LINUX
    QFile file("/proc/self/status");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QRegularExpression re("\\s+");
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("VmRSS:")) {
                QStringList parts = line.split(re, Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    return parts[1].toDouble() / 1024.0;  // kB to MB
                }
            }
        }
    }
    return 0.0;
#elif defined(Q_OS_WIN)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
    }
    return 0.0;
#elif defined(Q_OS_MAC)
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&t_info),
                  &t_info_count) == KERN_SUCCESS) {
        return static_cast<double>(t_info.resident_size) / (1024.0 * 1024.0);
    }
    return 0.0;
#else
    return 0.0;
#endif
}
