#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <functional>

#include "src/calendar/CalendarManager.h"

class GeminiService : public QObject {
    Q_OBJECT
public:
    explicit GeminiService(const QString &apiKey = QString(), QObject *parent = nullptr);

    void setApiKey(const QString &apiKey);
    QString apiKey() const {
        return m_apiKey;
    }

    // Gửi prompt tự do
    void askGemini(const QString &prompt,
                   std::function<void(bool success, QString response)> callback);

    // Phân tích lịch trình tự nhiên (AI Smart Parser)
    void parseNaturalLanguageSchedule(
        const QString &userText, const QDateTime &referenceDateTime,
        std::function<void(bool success, QList<ScheduleEvent> events, QString explanation)>
            callback);

    // Tạo báo cáo tóm tắt ngày / tuần (AI Daily & Weekly Briefing)
    void generateDailyBriefing(const QList<ScheduleEvent> &events, const QDate &targetDate,
                               std::function<void(bool success, QString briefing)> callback);

    // Gợi ý tối ưu hóa thời gian & lịch tập trung Pomodoro
    void optimizeSchedule(const QList<ScheduleEvent> &events, const QDate &targetDate,
                          std::function<void(bool success, QString advice)> callback);

private:
    QString m_apiKey;
    QNetworkAccessManager *m_networkManager;

    void sendGeminiRequest(const QString &systemInstruction, const QString &prompt,
                           std::function<void(bool success, QString result)> callback);
};
