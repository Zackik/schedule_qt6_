import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

def repl(m):
    return """
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
                        // For all-day events, Google sets the end date to the next day.
                        // We subtract 1 second to make it 23:59:59 of the correct day.
                        ev.endTime = QDateTime(d, QTime(0, 0)).addSecs(-1);
                    }
                }

                if (!ev.startTime.isValid())
                    ev.startTime = QDateTime::currentDateTime();
                if (!ev.endTime.isValid())
                    ev.endTime = ev.startTime.addSecs(3600);"""

content = re.sub(r'\s*QString startStr = startObj\["dateTime"\].*?if \(!ev\.endTime\.isValid\(\)\)\n\s*ev\.endTime = ev\.startTime\.addSecs\(3600\);', repl, content, flags=re.DOTALL)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
