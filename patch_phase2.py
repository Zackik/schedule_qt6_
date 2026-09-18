import re

with open('src/calendar/CalendarManager.h', 'r') as f:
    content = f.read()

content = content.replace('void fetchGoogleCalendarEvents();', 'void fetchGoogleCalendarEvents(const QDate &start = QDate(), const QDate &end = QDate(), const QString &pageToken = "");')

with open('src/calendar/CalendarManager.h', 'w') as f:
    f.write(content)

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

def repl_fetch(m):
    return """void CalendarManager::fetchGoogleCalendarEvents(const QDate &start, const QDate &end, const QString &pageToken) {
    qDebug() << "Fetching events from Google Calendar...";
    if (!isGoogleAuthenticated()) {
        qWarning() << "Google Sync: Chưa đăng nhập Google, không thể tải sự kiện.";
        return;
    }

    QUrl url("https://www.googleapis.com/calendar/v3/calendars/primary/events");
    QUrlQuery query;
    
    QDate queryStart = start.isValid() ? start : QDate::currentDate().addDays(-30);
    QDate queryEnd = end.isValid() ? end : QDate::currentDate().addDays(90);

    query.addQueryItem("timeMin", QDateTime(queryStart, QTime(0, 0)).toUTC().toString(Qt::ISODate));
    query.addQueryItem("timeMax", QDateTime(queryEnd, QTime(23, 59, 59)).toUTC().toString(Qt::ISODate));
    query.addQueryItem("singleEvents", "true");
    query.addQueryItem("orderBy", "startTime");
    if (!pageToken.isEmpty()) {
        query.addQueryItem("pageToken", pageToken);
    }
    
    url.setQuery(query);"""

content = re.sub(
    r'void CalendarManager::fetchGoogleCalendarEvents\(\) \{\s*qDebug\(\) << "Fetching events from Google Calendar\.\.\.";.*?url\.setQuery\(query\);',
    repl_fetch,
    content,
    flags=re.DOTALL
)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
