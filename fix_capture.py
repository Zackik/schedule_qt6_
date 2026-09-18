import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

target = "connect(reply, &QNetworkReply::finished, this, [this, reply, start, end]() {"
replacement = "connect(reply, &QNetworkReply::finished, this, [this, reply, start, end, queryStart, queryEnd]() {"

content = content.replace(target, replacement)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
