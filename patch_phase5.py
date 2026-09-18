import re

with open('src/calendar/CalendarManager.h', 'r') as f:
    content = f.read()

content = content.replace('void syncEventToGoogle(const ScheduleEvent &event);', 'void syncEventToGoogle(const ScheduleEvent &event, int retryCount = 0);')
content = content.replace('void updateGoogleEvent(const ScheduleEvent &event);', 'void updateGoogleEvent(const ScheduleEvent &event, int retryCount = 0);')

with open('src/calendar/CalendarManager.h', 'w') as f:
    f.write(content)

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

def repl_sync(m):
    return """void CalendarManager::syncEventToGoogle(const ScheduleEvent &event, int retryCount) {
    qDebug() << "Syncing event to Google Calendar:" << event.title << "(Attempt:" << retryCount + 1 << ")";
    if (!isGoogleAuthenticated())
        return;

    QUrl apiURL("https://www.googleapis.com/calendar/v3/calendars/primary/events");
    QNetworkRequest request(apiURL);
    request.setRawHeader("Authorization", QByteArray("Bearer ") + m_oauthManager->accessToken().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject eventJson;
    eventJson["summary"] = event.title;
    eventJson["description"] = event.description + QString("\\n[Phân loại: %1 | Chi phí: %2 VNĐ]")
                                                       .arg(categoryToString(event.category))
                                                       .arg(event.cost);
    if (!event.location.isEmpty()) {
        eventJson["location"] = event.location;
    }

    QJsonObject startObj;
    startObj["dateTime"] = event.startTime.toUTC().toString(Qt::ISODate);
    eventJson["start"] = startObj;

    QJsonObject endObj;
    endObj["dateTime"] = event.endTime.toUTC().toString(Qt::ISODate);
    eventJson["end"] = endObj;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(eventJson).toJson());
    QString eventId = event.id;  // Capture ID by value

    connect(reply, &QNetworkReply::finished, this, [this, reply, eventId, event, retryCount]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(response);
            QJsonObject resObj = doc.object();

            // Find and update the event in m_localEvents
            for (int i = 0; i < m_localEvents.size(); ++i) {
                if (m_localEvents[i].id == eventId) {
                    m_localEvents[i].googleEventId = resObj["id"].toString();
                    m_localEvents[i].isSyncedGoogle = true;
                    saveEventsToDatabase();
                    break;
                }
            }
            emit googleSyncFinished(true, "Đã đẩy sự kiện lên Google Calendar thành công!");
        } else {
            QByteArray errBody = reply->readAll();
            qWarning() << "Google API Error:" << reply->errorString() << errBody;
            
            if (reply->error() == QNetworkReply::ServiceUnavailableError || errBody.contains("rateLimitExceeded") || errBody.contains("403")) {
                if (retryCount < 3) {
                    int delayMs = (1 << retryCount) * 1000; // Exponential backoff: 1s, 2s, 4s
                    qWarning() << "Rate limit reached. Retrying in" << delayMs << "ms...";
                    QTimer::singleShot(delayMs, this, [this, event, retryCount]() {
                        syncEventToGoogle(event, retryCount + 1);
                    });
                    reply->deleteLater();
                    return;
                }
            }

            QString details = reply->errorString();
            QJsonDocument errDoc = QJsonDocument::fromJson(errBody);
            if (!errDoc.isNull() && errDoc.object().contains("error")) {
                 QJsonObject errObj = errDoc.object()["error"].toObject();
                 details += " - " + errObj["message"].toString();
            }
            emit googleSyncFinished(false, "Lỗi đẩy sự kiện lên Google: " + details);
        }
        reply->deleteLater();
    });
}"""

content = re.sub(
    r'void CalendarManager::syncEventToGoogle\(const ScheduleEvent &event\) \{.*?reply->deleteLater\(\);\s*\}\);\s*\}',
    repl_sync,
    content,
    flags=re.DOTALL
)

def repl_update(m):
    return """void CalendarManager::updateGoogleEvent(const ScheduleEvent &event, int retryCount) {
    qDebug() << "Updating Google Calendar event:" << event.title << "Google ID:" << event.googleEventId << "(Attempt:" << retryCount + 1 << ")";
    if (!isGoogleAuthenticated() || event.googleEventId.isEmpty())
        return;

    QUrl apiURL("https://www.googleapis.com/calendar/v3/calendars/primary/events/" + event.googleEventId);
    QNetworkRequest request(apiURL);
    request.setRawHeader("Authorization", QByteArray("Bearer ") + m_oauthManager->accessToken().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject eventJson;
    eventJson["summary"] = event.title;
    eventJson["description"] = event.description + QString("\\n[Phân loại: %1 | Chi phí: %2 VNĐ]")
                                                       .arg(categoryToString(event.category))
                                                       .arg(event.cost);
    if (!event.location.isEmpty()) {
        eventJson["location"] = event.location;
    }

    QJsonObject startObj;
    startObj["dateTime"] = event.startTime.toUTC().toString(Qt::ISODate);
    eventJson["start"] = startObj;

    QJsonObject endObj;
    endObj["dateTime"] = event.endTime.toUTC().toString(Qt::ISODate);
    eventJson["end"] = endObj;

    QNetworkReply *reply = m_networkManager->put(request, QJsonDocument(eventJson).toJson());
    QString eventId = event.id;

    connect(reply, &QNetworkReply::finished, this, [this, reply, eventId, event, retryCount]() {
        if (reply->error() == QNetworkReply::NoError) {
            for (int i = 0; i < m_localEvents.size(); ++i) {
                if (m_localEvents[i].id == eventId) {
                    m_localEvents[i].isSyncedGoogle = true;
                    saveEventsToDatabase();
                    break;
                }
            }
            emit googleSyncFinished(true, "Đã cập nhật sự kiện trên Google Calendar!");
        } else {
            QByteArray errBody = reply->readAll();
            qWarning() << "Google API Error:" << reply->errorString() << errBody;
            
            if (reply->error() == QNetworkReply::ServiceUnavailableError || errBody.contains("rateLimitExceeded") || errBody.contains("403")) {
                if (retryCount < 3) {
                    int delayMs = (1 << retryCount) * 1000;
                    qWarning() << "Rate limit reached. Retrying in" << delayMs << "ms...";
                    QTimer::singleShot(delayMs, this, [this, event, retryCount]() {
                        updateGoogleEvent(event, retryCount + 1);
                    });
                    reply->deleteLater();
                    return;
                }
            }

            QString details = reply->errorString();
            QJsonDocument errDoc = QJsonDocument::fromJson(errBody);
            if (!errDoc.isNull() && errDoc.object().contains("error")) {
                 QJsonObject errObj = errDoc.object()["error"].toObject();
                 details += " - " + errObj["message"].toString();
            }
            emit googleSyncFinished(false, "Lỗi cập nhật sự kiện trên Google: " + details);
        }
        reply->deleteLater();
    });
}"""

content = re.sub(
    r'void CalendarManager::updateGoogleEvent\(const ScheduleEvent &event\) \{.*?reply->deleteLater\(\);\s*\}\);\s*\}',
    repl_update,
    content,
    flags=re.DOTALL
)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
