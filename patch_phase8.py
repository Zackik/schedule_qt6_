import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

def repl_fetch_end(m):
    return """            if (!nextPageToken.isEmpty()) {
                qDebug() << "Fetching next page of Google events...";
                fetchGoogleCalendarEvents(start, end, nextPageToken);
            } else {
                saveEventsToDatabase();
                emit eventsReloaded();
                emit googleEventsFetched(fetchedList);
                
                // Now push local changes to Google
                syncAllLocalEventsToGoogle();
                
                emit googleSyncFinished(
                    true, QString("Đã đồng bộ xong dữ liệu 2 chiều (Lấy mới %1 sự kiện từ Google)").arg(fetchedList.size()));
            }"""

content = re.sub(
    r'            if \(!nextPageToken\.isEmpty\(\)\) \{.*?emit googleSyncFinished\([\s\S]*?\)\);\s*\}',
    repl_fetch_end,
    content,
    flags=re.DOTALL
)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)

with open('src/ui/ScheduleWidget.cpp', 'r') as f:
    content = f.read()

content = content.replace('m_calendarManager->syncAllLocalEventsToGoogle();\n    m_calendarManager->fetchGoogleCalendarEvents(start, end);', 'm_calendarManager->fetchGoogleCalendarEvents(start, end);')

with open('src/ui/ScheduleWidget.cpp', 'w') as f:
    f.write(content)
