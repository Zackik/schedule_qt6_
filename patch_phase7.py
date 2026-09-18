import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

def repl_getEvents(m):
    return """QList<ScheduleEvent> CalendarManager::getEventsForDate(const QDate &date) const {
    QList<ScheduleEvent> list;
    QDateTime dayStart(date, QTime(0, 0));
    QDateTime dayEnd = dayStart.addDays(1);

    for (const auto &e : m_localEvents) {
        if (e.startTime < dayEnd && e.endTime > dayStart) {
            list.append(e);
        }
    }
    return list;
}"""

content = re.sub(
    r'QList<ScheduleEvent> CalendarManager::getEventsForDate\(const QDate &date\) const \{.*?return list;\s*\}',
    repl_getEvents,
    content,
    flags=re.DOTALL
)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)

with open('src/ui/ScheduleWidget.cpp', 'r') as f:
    content = f.read()

def repl_updateList(m):
    return """    QDate selectedDate = m_calendarWidget ? m_calendarWidget->selectedDate() : QDate::currentDate();
    QDateTime dayStart(selectedDate, QTime(0, 0));
    QDateTime dayEnd = dayStart.addDays(1);

    QList<ScheduleEvent> displayEvents;
    for (const auto &e : m_calendarManager->m_localEvents) {
        // Lọc theo ngày được chọn trên lịch (overlap 24h)
        if (!(e.startTime < dayEnd && e.endTime > dayStart)) {
            continue;
        }"""

content = re.sub(
    r'    QDate selectedDate = m_calendarWidget \? m_calendarWidget->selectedDate\(\) : QDate::currentDate\(\);\s*QList<ScheduleEvent> displayEvents;\s*for \(const auto &e : m_calendarManager->m_localEvents\) \{\s*// Lọc theo ngày được chọn trên lịch\s*if \(e\.startTime\.date\(\) != selectedDate && !\(e\.startTime\.date\(\) <= selectedDate && e\.endTime\.date\(\) >= selectedDate\)\) \{\s*continue;\s*\}',
    repl_updateList,
    content,
    flags=re.DOTALL
)

with open('src/ui/ScheduleWidget.cpp', 'w') as f:
    f.write(content)
