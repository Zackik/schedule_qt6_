#include <QSet>
#pragma once

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "src/services/oauth/GoogleOAuthManager.h"

#include <QObject>
#include <QStandardPaths>
#include <QString>
#include <QSystemTrayIcon>
#include <QTimer>

enum class Category {
    Study,     // Học tập
    Work,      // Công việc
    Personal,  // Cá nhân
    Health,    // Sức khỏe
    Other      // Khác
};

enum class Priority {
    Low,     // Thấp
    Medium,  // Trung bình
    High     // Cao (Khẩn cấp)
};

enum class PomodoroState { Stopped, Working, ShortBreak, LongBreak };

struct ScheduleEvent {
    QString id;
    QString title;
    QString description;
    QString location;
    QDateTime startTime;
    QDateTime endTime;
    Category category = Category::Work;
    Priority priority = Priority::Medium;
    double cost = 0.0;
    bool isNotified = false;
    bool isSyncedGoogle = false;
    bool isCompleted = false;
    QString googleEventId;

    QJsonObject toJson() const;
    static ScheduleEvent fromJson(const QJsonObject &json);

    bool matches(const ScheduleEvent &other) const {
        return title == other.title &&
               description == other.description &&
               location == other.location &&
               startTime == other.startTime &&
               endTime == other.endTime;
    }

};

#include <QStack>

enum class UndoActionType { Deleted, Updated };

struct UndoAction {
    UndoActionType type;
    ScheduleEvent previousState;
};

class CalendarManager : public QObject {
    Q_OBJECT
public:
    explicit CalendarManager(QObject *parent = nullptr);
    ~CalendarManager() override;
    GoogleOAuthManager* oauthManager() const { return m_oauthManager; }

    // Quản lý sự kiện cục bộ
    QList<ScheduleEvent> m_localEvents;
    QSet<QString> m_syncFetchedGoogleIds;
    void addLocalEvent(const ScheduleEvent &event);
    void updateEvent(const QString &id, const ScheduleEvent &newEvent, bool isUndo = false);
    void deleteEvent(const QString &id, bool isUndo = false);

    // Undo
    void undoLastAction();
    bool canUndo() const {
        return !m_undoStack.isEmpty();
    }
    QStack<UndoAction> m_undoStack;
    QList<ScheduleEvent> getEventsForDate(const QDate &date) const;
    QList<ScheduleEvent> findConflicts(const ScheduleEvent &newEvent) const;

    // Lưu trữ cục bộ (SQLite)
    void initDatabase();
    void saveEventsToDatabase();
    void loadEventsFromDatabase();

    // Google Calendar API v3 & OAuth 2.0
    void setupGoogleOAuth(const QString &clientId, const QString &clientSecret);
    void authenticateGoogle();
    bool isGoogleAuthenticated() const;
    void fetchGoogleCalendarEvents(const QDate &start = QDate(), const QDate &end = QDate(), const QString &pageToken = "");
    void syncEventToGoogle(const ScheduleEvent &event, int retryCount = 0);
    void updateGoogleEvent(const ScheduleEvent &event, int retryCount = 0);
    void deleteGoogleEvent(const QString &googleEventId);
    void syncAllLocalEventsToGoogle();

    // Pomodoro Timer
    void startPomodoro(int workMinutes = 25, int breakMinutes = 5);
    void pausePomodoro();
    void resumePomodoro();
    void resetPomodoro();
    PomodoroState getPomodoroState() const {
        return m_pomodoroState;
    }
    int getRemainingSeconds() const {
        return m_remainingSeconds;
    }
    int getCompletedPomodoros() const {
        return m_completedPomodoros;
    }

    // Helpers chuyển đổi chuỗi
    static QString categoryToString(Category cat);
    static Category stringToCategory(const QString &str);
    static QString priorityToString(Priority prio);
    static Priority stringToPriority(const QString &str);

signals:
    void eventAdded(const ScheduleEvent &event);
    void eventUpdated(const ScheduleEvent &event);
    void eventDeleted(const QString &id);
    void eventsReloaded();
    void undoAvailabilityChanged(bool canUndo);
    void googleAuthStatusChanged(bool isAuthenticated, const QString &userEmail);
    void googleSyncFinished(bool success, const QString &message);
    void googleEventsFetched(const QList<ScheduleEvent> &events);
    void pomodoroTick(int remainingSeconds, PomodoroState state);
    void pomodoroFinished(PomodoroState completedState);
    void notificationTriggered(const QString &title, const QString &message);

private:
    GoogleOAuthManager *m_oauthManager = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;
    QTimer *m_reminderTimer = nullptr;
    QTimer *m_pomodoroTimer = nullptr;

    int m_remainingSeconds = 1500;
    int m_workDurationMinutes = 25;
    int m_breakDurationMinutes = 5;
    int m_completedPomodoros = 0;
    PomodoroState m_pomodoroState = PomodoroState::Stopped;

    QString m_saveFilePath;

    void setupReminderTimer();
    void setupPomodoro();
    void checkUpcomingReminders();
};
