#include <QDesktopServices>
#include "src/calendar/CalendarManager.h"

#include <QProcessEnvironment>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUrlQuery>
#include <QUuid>

QJsonObject ScheduleEvent::toJson() const {
    QJsonObject obj;
    obj["id"] = id;
    obj["title"] = title;
    obj["description"] = description;
    obj["location"] = location;
    obj["startTime"] = startTime.toString(Qt::ISODate);
    obj["endTime"] = endTime.toString(Qt::ISODate);
    obj["category"] = CalendarManager::categoryToString(category);
    obj["priority"] = CalendarManager::priorityToString(priority);
    obj["cost"] = cost;
    obj["isNotified"] = isNotified;
    obj["isSyncedGoogle"] = isSyncedGoogle;
    obj["isCompleted"] = isCompleted;
    obj["googleEventId"] = googleEventId;
    return obj;
}

ScheduleEvent ScheduleEvent::fromJson(const QJsonObject &json) {
    ScheduleEvent event;
    event.id = json["id"].toString(QUuid::createUuid().toString(QUuid::WithoutBraces));
    event.title = json["title"].toString();
    event.description = json["description"].toString();
    event.location = json["location"].toString();
    event.startTime = QDateTime::fromString(json["startTime"].toString(), Qt::ISODate);
    event.endTime = QDateTime::fromString(json["endTime"].toString(), Qt::ISODate);
    event.category = CalendarManager::stringToCategory(json["category"].toString());
    event.priority = CalendarManager::stringToPriority(json["priority"].toString());
    event.cost = json["cost"].toDouble(0.0);
    event.isNotified = json["isNotified"].toBool(false);
    event.isSyncedGoogle = json["isSyncedGoogle"].toBool(false);
    event.isCompleted = json["isCompleted"].toBool(false);
    event.googleEventId = json["googleEventId"].toString();
    return event;
}

CalendarManager::CalendarManager(QObject *parent) : QObject(parent) {
    m_networkManager = new QNetworkAccessManager(this);

    // Xác định đường dẫn lưu file calendar.db
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataDir);
    m_saveFilePath = appDataDir + "/calendar.db";
    initDatabase();

    setupReminderTimer();
    setupPomodoro();
    loadEventsFromDatabase();
}

CalendarManager::~CalendarManager() {
    saveEventsToDatabase();
}

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

void CalendarManager::fetchGoogleCalendarEvents(const QDate &start, const QDate &end, const QString &pageToken) {
    qDebug() << "Fetching events from Google Calendar...";
    if (!isGoogleAuthenticated()) {
        qWarning() << "Google Sync: Chưa đăng nhập Google, không thể tải sự kiện.";
        return;
    }

    QUrl url("https://www.googleapis.com/calendar/v3/calendars/primary/events");
    QUrlQuery query;
    
    QDate queryStart = start.isValid() ? start : QDate::currentDate().addDays(-15);
    QDate queryEnd = end.isValid() ? end : QDate::currentDate().addDays(15);

    query.addQueryItem("timeMin", QDateTime(queryStart, QTime(0, 0)).toUTC().toString(Qt::ISODate));
    // Time max = end + 1 day at 00:00:00 (lấy trọn vẹn ngày cuối cùng)
    query.addQueryItem("timeMax", QDateTime(queryEnd.addDays(1), QTime(0, 0)).toUTC().toString(Qt::ISODate));
    query.addQueryItem("singleEvents", "true");
    query.addQueryItem("orderBy", "startTime");
    if (!pageToken.isEmpty()) {
        query.addQueryItem("pageToken", pageToken);
    } else {
        // Lần gọi đầu tiên (không có pageToken), reset danh sách fetched IDs
        m_syncFetchedGoogleIds.clear();
    }
    
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QByteArray("Bearer ") + m_oauthManager->accessToken().toUtf8());

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, start, end, queryStart, queryEnd]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonArray items = doc.object()["items"].toArray();
            QList<ScheduleEvent> fetchedList;
            
            for (const auto &val : items) {
                QJsonObject itemObj = val.toObject();
                // Bỏ qua các sự kiện đã bị xóa trên Google (cancelled)
                if (itemObj["status"].toString() == "cancelled") continue;
                
                ScheduleEvent ev;
                ev.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                ev.googleEventId = itemObj["id"].toString();
                ev.title = itemObj["summary"].toString("Không có tiêu đề");
                ev.description = itemObj["description"].toString();
                ev.location = itemObj["location"].toString();

                QJsonObject startObj = itemObj["start"].toObject();
                QJsonObject endObj = itemObj["end"].toObject();
                
                QString startStr = startObj.contains("dateTime") ? startObj["dateTime"].toString() : startObj["date"].toString();
                QString endStr = endObj.contains("dateTime") ? endObj["dateTime"].toString() : endObj["date"].toString();

                ev.startTime = QDateTime::fromString(startStr, Qt::ISODate);
                if (!ev.startTime.isValid()) {
                    QDate d = QDate::fromString(startStr, Qt::ISODate);
                    if (d.isValid()) {
                        ev.startTime = QDateTime(d, QTime(0, 0));
                    } else {
                        // Fallback fallback
                        d = QDate::fromString(startStr, "yyyy-MM-dd");
                        if (d.isValid()) ev.startTime = QDateTime(d, QTime(0, 0));
                    }
                }
                
                ev.endTime = QDateTime::fromString(endStr, Qt::ISODate);
                if (!ev.endTime.isValid()) {
                    QDate d = QDate::fromString(endStr, Qt::ISODate);
                    if (d.isValid()) {
                        ev.endTime = QDateTime(d, QTime(0, 0)).addSecs(-1);
                    } else {
                        // Fallback fallback
                        d = QDate::fromString(endStr, "yyyy-MM-dd");
                        if (d.isValid()) ev.endTime = QDateTime(d, QTime(0, 0)).addSecs(-1);
                    }
                }

                if (!ev.startTime.isValid())
                    ev.startTime = QDateTime::currentDateTime();
                if (!ev.endTime.isValid())
                    ev.endTime = ev.startTime.addSecs(3600);

                ev.category = Category::Work;
                ev.priority = Priority::Medium;
                ev.isSyncedGoogle = true;

                // Compare Google <-> Local
                bool exists = false;
                for (int i = 0; i < m_localEvents.size(); ++i) {
                    bool matchById = (m_localEvents[i].googleEventId == ev.googleEventId && !ev.googleEventId.isEmpty());
                    bool matchByContent = (m_localEvents[i].googleEventId.isEmpty() && 
                                           m_localEvents[i].title == ev.title && 
                                           m_localEvents[i].startTime == ev.startTime && 
                                           m_localEvents[i].endTime == ev.endTime);
                    
                    if (matchById || matchByContent) {
                        exists = true;
                        
                        // Nếu matchByContent, chúng ta vừa tìm thấy event local tương ứng trên Google
                        if (matchByContent) {
                            m_localEvents[i].googleEventId = ev.googleEventId;
                            m_localEvents[i].isSyncedGoogle = true;
                        }
                        m_syncFetchedGoogleIds.insert(ev.googleEventId);
                        if (!m_localEvents[i].isSyncedGoogle) {
                            // Local có thay đổi chưa đồng bộ -> Bỏ qua không đè dữ liệu từ Google
                            qDebug() << "Bỏ qua ghi đè từ Google do Local có thay đổi:" << ev.title;
                        } else {
                            // Local sạch -> Cập nhật từ Google
                            m_localEvents[i].title = ev.title;
                            m_localEvents[i].description = ev.description;
                            m_localEvents[i].location = ev.location;
                            m_localEvents[i].startTime = ev.startTime;
                            m_localEvents[i].endTime = ev.endTime;
                            m_localEvents[i].isSyncedGoogle = true;
                        }
                        break;
                    }
                }

                if (!exists) {
                    m_localEvents.append(ev);
                    fetchedList.append(ev);
                    m_syncFetchedGoogleIds.insert(ev.googleEventId);
                }
            }

            QString nextPageToken = doc.object()["nextPageToken"].toString();
            if (!nextPageToken.isEmpty()) {
                qDebug() << "Fetching next page of Google events...";
                fetchGoogleCalendarEvents(queryStart, queryEnd, nextPageToken);
            } else {
                // Đã tải xong tất cả các trang
                // Xoá các event trên local (trong khoảng thời gian sync) mà KHÔNG có mặt trên Google (bị xoá trên Google)
                QDateTime syncStart(queryStart, QTime(0, 0));
                QDateTime syncEnd(queryEnd.addDays(1), QTime(0, 0));
                
                for (int i = m_localEvents.size() - 1; i >= 0; --i) {
                    const auto& e = m_localEvents[i];
                    if (!e.googleEventId.isEmpty()) {
                        // Nếu event này thuộc khoảng sync
                        if (!(e.endTime <= syncStart || e.startTime >= syncEnd)) {
                            // Và không được Google trả về -> Nó đã bị xoá trên Google
                            if (!m_syncFetchedGoogleIds.contains(e.googleEventId)) {
                                qDebug() << "Xoá sự kiện cục bộ vì đã bị xoá trên Google:" << e.title;
                                m_localEvents.removeAt(i);
                            }
                        }
                    }
                }
                
                saveEventsToDatabase();
                emit eventsReloaded();
                emit googleEventsFetched(fetchedList);
                
                // Now push local changes to Google
                syncAllLocalEventsToGoogle();
                
                emit googleSyncFinished(
                    true, QString("Đã đồng bộ xong dữ liệu 2 chiều (Lấy mới %1 sự kiện từ Google)").arg(fetchedList.size()));
            }
        } else {
            
            QByteArray errBody = reply->readAll();
            qWarning() << "Google API Error:" << reply->errorString() << errBody;
            QString details = reply->errorString();
            QJsonDocument errDoc = QJsonDocument::fromJson(errBody);
            if (!errDoc.isNull() && errDoc.object().contains("error")) {
                 QJsonObject errObj = errDoc.object()["error"].toObject();
                 details += " - " + errObj["message"].toString();
            }
            emit googleSyncFinished(false, "Lỗi lấy dữ liệu từ Google: " + details);

        }
        reply->deleteLater();
    });
}

void CalendarManager::syncEventToGoogle(const ScheduleEvent &event, int retryCount) {
    qDebug() << "Syncing event to Google Calendar:" << event.title << "(Attempt:" << retryCount + 1 << ")";
    if (!isGoogleAuthenticated())
        return;

    QUrl apiURL("https://www.googleapis.com/calendar/v3/calendars/primary/events");
    QNetworkRequest request(apiURL);
    request.setRawHeader("Authorization", QByteArray("Bearer ") + m_oauthManager->accessToken().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject eventJson;
    eventJson["summary"] = event.title;
    eventJson["description"] = event.description + QString("\n[Phân loại: %1 | Chi phí: %2 VNĐ]")
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
}

void CalendarManager::updateGoogleEvent(const ScheduleEvent &event, int retryCount) {
    qDebug() << "Updating Google Calendar event:" << event.title << "Google ID:" << event.googleEventId << "(Attempt:" << retryCount + 1 << ")";
    if (!isGoogleAuthenticated() || event.googleEventId.isEmpty())
        return;

    QUrl apiURL("https://www.googleapis.com/calendar/v3/calendars/primary/events/" + event.googleEventId);
    QNetworkRequest request(apiURL);
    request.setRawHeader("Authorization", QByteArray("Bearer ") + m_oauthManager->accessToken().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject eventJson;
    eventJson["summary"] = event.title;
    eventJson["description"] = event.description + QString("\n[Phân loại: %1 | Chi phí: %2 VNĐ]")
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
}

void CalendarManager::deleteGoogleEvent(const QString &googleEventId) {
    qDebug() << "Deleting event from Google Calendar, Google ID:" << googleEventId;
    if (!isGoogleAuthenticated() || googleEventId.isEmpty())
        return;

    QUrl apiURL(QString("https://www.googleapis.com/calendar/v3/calendars/primary/events/%1")
                    .arg(googleEventId));
    QNetworkRequest request(apiURL);
    request.setRawHeader("Authorization", QByteArray("Bearer ") + m_oauthManager->accessToken().toUtf8());

    QNetworkReply *reply = m_networkManager->deleteResource(request);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

void CalendarManager::syncAllLocalEventsToGoogle() {
    if (!isGoogleAuthenticated()) {
        qWarning() << "Google Sync: Chưa đăng nhập Google, không thể đồng bộ sự kiện lên máy chủ.";
        return;
    }

    for (const auto &ev : m_localEvents) {
        if (!ev.isSyncedGoogle) {
            if (ev.googleEventId.isEmpty()) {
                syncEventToGoogle(ev);
            } else {
                updateGoogleEvent(ev);
            }
        }
    }
}

void CalendarManager::addLocalEvent(const ScheduleEvent &event) {
    ScheduleEvent newEv = event;
    if (newEv.id.isEmpty()) {
        newEv.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    newEv.isSyncedGoogle = false; // Mark dirty
    m_localEvents.append(newEv);
    saveEventsToDatabase();
    emit eventAdded(newEv);

    if (isGoogleAuthenticated()) {
        syncEventToGoogle(m_localEvents.last());
    }
}

void CalendarManager::updateEvent(const QString &id, const ScheduleEvent &newEvent, bool isUndo) {
    for (int i = 0; i < m_localEvents.size(); ++i) {
        if (m_localEvents[i].id == id) {
            if (!isUndo) {
                m_undoStack.push({UndoActionType::Updated, m_localEvents[i]});
                emit undoAvailabilityChanged(true);
            }

            m_localEvents[i] = newEvent;
            m_localEvents[i].isSyncedGoogle = false; // Mark dirty
            saveEventsToDatabase();
            emit eventUpdated(newEvent);

            if (isGoogleAuthenticated()) {
                if (newEvent.googleEventId.isEmpty()) {
                    syncEventToGoogle(newEvent);
                } else {
                    updateGoogleEvent(newEvent);
                }
            }
            return;
        }
    }
}

void CalendarManager::deleteEvent(const QString &id, bool isUndo) {
    for (int i = 0; i < m_localEvents.size(); ++i) {
        if (m_localEvents[i].id == id) {
            if (!isUndo) {
                m_undoStack.push({UndoActionType::Deleted, m_localEvents[i]});
                emit undoAvailabilityChanged(true);
            }

            if (!m_localEvents[i].googleEventId.isEmpty()) {
                deleteGoogleEvent(m_localEvents[i].googleEventId);
            }
            m_localEvents.removeAt(i);
            saveEventsToDatabase();
            emit eventDeleted(id);
            return;
        }
    }
}

void CalendarManager::undoLastAction() {
    if (m_undoStack.isEmpty())
        return;

    UndoAction action = m_undoStack.pop();
    emit undoAvailabilityChanged(!m_undoStack.isEmpty());

    if (action.type == UndoActionType::Updated) {
        // Khôi phục lại sự kiện cập nhật (vd: Drag & Drop nhầm)
        updateEvent(action.previousState.id, action.previousState, true);
    } else if (action.type == UndoActionType::Deleted) {
        // Khôi phục sự kiện đã xóa
        ScheduleEvent restoredEvent = action.previousState;
        restoredEvent.googleEventId = "";  // Xóa ID Google vì nó đã bị xóa trên Google Calendar
        restoredEvent.isSyncedGoogle = false;

        m_localEvents.append(restoredEvent);
        saveEventsToDatabase();
        emit eventAdded(restoredEvent);

        if (isGoogleAuthenticated()) {
            syncEventToGoogle(restoredEvent);
        }
    }
}

QList<ScheduleEvent> CalendarManager::getEventsForDate(const QDate &date) const {
    QList<ScheduleEvent> list;
    QDateTime dayStart(date, QTime(0, 0));
    QDateTime dayEnd = dayStart.addDays(1);

    for (const auto &e : m_localEvents) {
        if (e.startTime < dayEnd && e.endTime > dayStart) {
            list.append(e);
        }
    }
    return list;
}

QList<ScheduleEvent> CalendarManager::findConflicts(const ScheduleEvent &newEvent) const {
    QList<ScheduleEvent> conflicts;
    for (const auto &e : m_localEvents) {
        if (e.id == newEvent.id)
            continue;
        if (newEvent.startTime < e.endTime && newEvent.endTime > e.startTime) {
            conflicts.append(e);
        }
    }
    return conflicts;
}

void CalendarManager::initDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "calendar_db");
    db.setDatabaseName(m_saveFilePath);
    if (!db.open()) {
        qWarning() << "Failed to open SQLite database:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS events ("
        "id TEXT PRIMARY KEY, "
        "title TEXT, "
        "description TEXT, "
        "location TEXT, "
        "start_time TEXT, "
        "end_time TEXT, "
        "category TEXT, "
        "priority TEXT, "
        "cost REAL, "
        "is_notified INTEGER, "
        "is_synced_google INTEGER, "
        "is_completed INTEGER DEFAULT 0, "
        "google_event_id TEXT"
        ")");
    if (!success) {
        qWarning() << "Failed to create events table:" << query.lastError().text();
    } else {
        // Safe check before ALTER TABLE
        QSqlQuery checkQuery(db);
        checkQuery.exec("PRAGMA table_info(events)");
        bool hasIsCompleted = false;
        bool hasGoogleEventId = false;
        while (checkQuery.next()) {
            QString colName = checkQuery.value("name").toString();
            if (colName == "is_completed") hasIsCompleted = true;
            if (colName == "google_event_id") hasGoogleEventId = true;
        }
        if (!hasIsCompleted) {
            query.exec("ALTER TABLE events ADD COLUMN is_completed INTEGER DEFAULT 0");
        }
        if (!hasGoogleEventId) {
            query.exec("ALTER TABLE events ADD COLUMN google_event_id TEXT");
        }
    }
}

void CalendarManager::saveEventsToDatabase() {
    QSqlDatabase db = QSqlDatabase::database("calendar_db");
    if (!db.isOpen())
        return;

    db.transaction();
    QSqlQuery query(db);
    query.exec("DELETE FROM events");

    query.prepare(
        "INSERT INTO events (id, title, description, location, start_time, end_time, category, "
        "priority, cost, is_notified, is_synced_google, is_completed, google_event_id) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    for (const auto &event : m_localEvents) {
        query.addBindValue(event.id);
        query.addBindValue(event.title);
        query.addBindValue(event.description);
        query.addBindValue(event.location);
        query.addBindValue(event.startTime.toString(Qt::ISODate));
        query.addBindValue(event.endTime.toString(Qt::ISODate));
        query.addBindValue(categoryToString(event.category));
        query.addBindValue(priorityToString(event.priority));
        query.addBindValue(event.cost);
        query.addBindValue(event.isNotified ? 1 : 0);
        query.addBindValue(event.isSyncedGoogle ? 1 : 0);
        query.addBindValue(event.isCompleted ? 1 : 0);
        query.addBindValue(event.googleEventId);
        query.exec();
    }
    db.commit();
}

void CalendarManager::loadEventsFromDatabase() {
    QSqlDatabase db = QSqlDatabase::database("calendar_db");
    if (!db.isOpen())
        return;

    m_localEvents.clear();
    QSqlQuery query("SELECT * FROM events", db);
    while (query.next()) {
        ScheduleEvent event;
        event.id = query.value("id").toString();
        event.title = query.value("title").toString();
        event.description = query.value("description").toString();
        event.location = query.value("location").toString();
        event.startTime = QDateTime::fromString(query.value("start_time").toString(), Qt::ISODate);
        event.endTime = QDateTime::fromString(query.value("end_time").toString(), Qt::ISODate);
        event.category = stringToCategory(query.value("category").toString());
        event.priority = stringToPriority(query.value("priority").toString());
        event.cost = query.value("cost").toDouble();
        event.isNotified = query.value("is_notified").toInt() == 1;
        event.isSyncedGoogle = query.value("is_synced_google").toInt() == 1;
        event.isCompleted = query.value("is_completed").toInt() == 1;
        event.googleEventId = query.value("google_event_id").toString();
        
        // Deduplicate
        bool duplicate = false;
        for (const auto& existing : m_localEvents) {
            if (!event.googleEventId.isEmpty() && existing.googleEventId == event.googleEventId) {
                duplicate = true;
                break;
            }
            if (event.googleEventId.isEmpty() && existing.title == event.title && 
                existing.startTime == event.startTime && existing.endTime == event.endTime) {
                duplicate = true;
                break;
            }
        }
        
        if (!duplicate) {
            m_localEvents.append(event);
        }
    }
}

void CalendarManager::setupReminderTimer() {
    m_reminderTimer = new QTimer(this);
    connect(m_reminderTimer, &QTimer::timeout, this, &CalendarManager::checkUpcomingReminders);
    m_reminderTimer->start(30000);  // Kiểm tra mỗi 30 giây
}

void CalendarManager::checkUpcomingReminders() {
    QDateTime now = QDateTime::currentDateTime();
    for (auto &event : m_localEvents) {
        qint64 secsTo = now.secsTo(event.startTime);
        // Nhắc nhở nếu sự kiện diễn ra trong vòng 15 phút tới
        if (secsTo > 0 && secsTo <= 900 && !event.isNotified) {
            event.isNotified = true;
            QString msg = QString("Sự kiện '%1' sắp bắt đầu lúc %2 (%3 phút nữa)!")
                              .arg(event.title)
                              .arg(event.startTime.toString("HH:mm"))
                              .arg(secsTo / 60);
            emit notificationTriggered("⏰ Nhắc Nhở Lịch Trình", msg);
        }
    }
}

void CalendarManager::setupPomodoro() {
    m_pomodoroTimer = new QTimer(this);
    connect(m_pomodoroTimer, &QTimer::timeout, [this]() {
        if (m_remainingSeconds > 0) {
            m_remainingSeconds--;
            emit pomodoroTick(m_remainingSeconds, m_pomodoroState);
        } else {
            m_pomodoroTimer->stop();
            if (m_pomodoroState == PomodoroState::Working) {
                m_completedPomodoros++;
                emit pomodoroFinished(PomodoroState::Working);
                // Sau 4 hiệp tập trung thì nghỉ dài 15p, ngược lại nghỉ ngắn
                if (m_completedPomodoros % 4 == 0) {
                    m_pomodoroState = PomodoroState::LongBreak;
                    m_remainingSeconds = 15 * 60;
                } else {
                    m_pomodoroState = PomodoroState::ShortBreak;
                    m_remainingSeconds = m_breakDurationMinutes * 60;
                }
                m_pomodoroTimer->start(1000);
            } else {
                emit pomodoroFinished(m_pomodoroState);
                m_pomodoroState = PomodoroState::Stopped;
                m_remainingSeconds = m_workDurationMinutes * 60;
            }
        }
    });
}

void CalendarManager::startPomodoro(int workMinutes, int breakMinutes) {
    m_workDurationMinutes = workMinutes;
    m_breakDurationMinutes = breakMinutes;
    m_remainingSeconds = workMinutes * 60;
    m_pomodoroState = PomodoroState::Working;
    m_pomodoroTimer->start(1000);
}

void CalendarManager::pausePomodoro() {
    if (m_pomodoroTimer->isActive()) {
        m_pomodoroTimer->stop();
    }
}

void CalendarManager::resumePomodoro() {
    if (!m_pomodoroTimer->isActive() && m_pomodoroState != PomodoroState::Stopped) {
        m_pomodoroTimer->start(1000);
    }
}

void CalendarManager::resetPomodoro() {
    m_pomodoroTimer->stop();
    m_pomodoroState = PomodoroState::Stopped;
    m_remainingSeconds = m_workDurationMinutes * 60;
    emit pomodoroTick(m_remainingSeconds, m_pomodoroState);
}

QString CalendarManager::categoryToString(Category cat) {
    switch (cat) {
        case Category::Study:
            return "Study";
        case Category::Work:
            return "Work";
        case Category::Personal:
            return "Personal";
        case Category::Health:
            return "Health";
        case Category::Other:
            return "Other";
    }
    return "Other";
}

Category CalendarManager::stringToCategory(const QString &str) {
    if (str.compare("Study", Qt::CaseInsensitive) == 0 ||
        str.compare("Học tập", Qt::CaseInsensitive) == 0)
        return Category::Study;
    if (str.compare("Work", Qt::CaseInsensitive) == 0 ||
        str.compare("Công việc", Qt::CaseInsensitive) == 0)
        return Category::Work;
    if (str.compare("Personal", Qt::CaseInsensitive) == 0 ||
        str.compare("Cá nhân", Qt::CaseInsensitive) == 0)
        return Category::Personal;
    if (str.compare("Health", Qt::CaseInsensitive) == 0 ||
        str.compare("Sức khỏe", Qt::CaseInsensitive) == 0)
        return Category::Health;
    return Category::Other;
}

QString CalendarManager::priorityToString(Priority prio) {
    switch (prio) {
        case Priority::Low:
            return "Low";
        case Priority::Medium:
            return "Medium";
        case Priority::High:
            return "High";
    }
    return "Medium";
}

Priority CalendarManager::stringToPriority(const QString &str) {
    if (str.compare("Low", Qt::CaseInsensitive) == 0 ||
        str.compare("Thấp", Qt::CaseInsensitive) == 0)
        return Priority::Low;
    if (str.compare("High", Qt::CaseInsensitive) == 0 ||
        str.compare("Cao", Qt::CaseInsensitive) == 0)
        return Priority::High;
    return Priority::Medium;
}
