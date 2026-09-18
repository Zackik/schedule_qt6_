import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

content = content.replace('addDays(30)', 'addDays(90)')
content = content.replace('addDays(-7)', 'addDays(-30)')

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
