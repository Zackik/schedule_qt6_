import re

with open('src/calendar/CalendarManager.h', 'r') as f:
    content = f.read()

target = "    QList<ScheduleEvent> m_localEvents;"
replacement = "    QList<ScheduleEvent> m_localEvents;\n    QSet<QString> m_syncFetchedGoogleIds;"

content = content.replace(target, replacement)

with open('src/calendar/CalendarManager.h', 'w') as f:
    f.write(content)
