import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

target = "fetchGoogleCalendarEvents(start, end, nextPageToken);"
replacement = "fetchGoogleCalendarEvents(queryStart, queryEnd, nextPageToken);"

content = content.replace(target, replacement)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
