import re

with open('src/ui/ScheduleWidget.cpp', 'r') as f:
    content = f.read()

target = """void ScheduleWidget::handleGoogleSync() {
    QDate start = QDate::currentDate().addDays(-30);
    QDate end = QDate::currentDate().addDays(90);
    
    if (m_calendarWidget) {
        int month = m_calendarWidget->monthShown();
        int year = m_calendarWidget->yearShown();
        QDate firstDay(year, month, 1);
        start = firstDay.addDays(-14); // Buffer for previous month days on grid
        end = firstDay.addDays(firstDay.daysInMonth() + 14); // Buffer for next month days
    }
    
    m_calendarManager->fetchGoogleCalendarEvents(start, end);
}"""

replacement = """void ScheduleWidget::handleGoogleSync() {
    QDate start = QDate::currentDate().addDays(-15);
    QDate end = QDate::currentDate().addDays(15);
    
    m_calendarManager->fetchGoogleCalendarEvents(start, end);
}"""

content = content.replace(target, replacement)

with open('src/ui/ScheduleWidget.cpp', 'w') as f:
    f.write(content)
