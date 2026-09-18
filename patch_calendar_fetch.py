import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

target = """    QDate queryStart = start.isValid() ? start : QDate::currentDate().addDays(-30);
    QDate queryEnd = end.isValid() ? end : QDate::currentDate().addDays(90);

    query.addQueryItem("timeMin", QDateTime(queryStart, QTime(0, 0)).toUTC().toString(Qt::ISODate));
    query.addQueryItem("timeMax", QDateTime(queryEnd, QTime(23, 59, 59)).toUTC().toString(Qt::ISODate));"""

replacement = """    QDate queryStart = start.isValid() ? start : QDate::currentDate().addDays(-15);
    QDate queryEnd = end.isValid() ? end : QDate::currentDate().addDays(15);

    query.addQueryItem("timeMin", QDateTime(queryStart, QTime(0, 0)).toUTC().toString(Qt::ISODate));
    // Time max = end + 1 day at 00:00:00 (lấy trọn vẹn ngày cuối cùng)
    query.addQueryItem("timeMax", QDateTime(queryEnd.addDays(1), QTime(0, 0)).toUTC().toString(Qt::ISODate));"""

content = content.replace(target, replacement)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
