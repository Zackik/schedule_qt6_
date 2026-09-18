import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

def repl_fetch_response(m):
    return """            QByteArray data = reply->readAll();
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
                QString startStr = startObj["dateTime"].toString(startObj["date"].toString());
                QString endStr = endObj["dateTime"].toString(endObj["date"].toString());

                ev.startTime = QDateTime::fromString(startStr, Qt::ISODate);
                if (!ev.startTime.isValid()) {
                    QDate d = QDate::fromString(startStr, Qt::ISODate);
                    if (d.isValid()) ev.startTime = QDateTime(d, QTime(0, 0));
                }
                
                ev.endTime = QDateTime::fromString(endStr, Qt::ISODate);
                if (!ev.endTime.isValid()) {
                    QDate d = QDate::fromString(endStr, Qt::ISODate);
                    if (d.isValid()) {
                        ev.endTime = QDateTime(d, QTime(0, 0)).addSecs(-1);
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
                    if (m_localEvents[i].googleEventId == ev.googleEventId && !ev.googleEventId.isEmpty()) {
                        exists = true;
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
                }
            }

            QString nextPageToken = doc.object()["nextPageToken"].toString();
            if (!nextPageToken.isEmpty()) {
                qDebug() << "Fetching next page of Google events...";
                fetchGoogleCalendarEvents(start, end, nextPageToken);
            } else {
                saveEventsToDatabase();
                emit eventsReloaded();
                emit googleEventsFetched(fetchedList);
                emit googleSyncFinished(
                    true, QString("Đã đồng bộ xong dữ liệu từ Google Calendar (Lấy mới %1 sự kiện)").arg(fetchedList.size()));
            }"""

content = re.sub(
    r'            QByteArray data = reply->readAll\(\);.*?emit googleSyncFinished\([\s\S]*?\)\);',
    repl_fetch_response,
    content,
    flags=re.DOTALL
)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
