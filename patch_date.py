import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

target = """                QJsonObject startObj = itemObj["start"].toObject();
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
                }"""

replacement = """                QJsonObject startObj = itemObj["start"].toObject();
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
                }"""

content = content.replace(target, replacement)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
