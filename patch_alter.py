import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

target = """        bool hasIsCompleted = false;
        while (checkQuery.next()) {
            if (checkQuery.value("name").toString() == "is_completed") {
                hasIsCompleted = true;
                break;
            }
        }
        if (!hasIsCompleted) {
            query.exec("ALTER TABLE events ADD COLUMN is_completed INTEGER DEFAULT 0");
        }"""

replacement = """        bool hasIsCompleted = false;
        bool hasGoogleEventId = false;
        while (checkQuery.next()) {
            QString colName = checkQuery.value("name").toString();
            if (colName == "is_completed") hasIsCompleted = true;
            if (colName == "google_event_id") hasGoogleEventId = true;
        }
        if (!hasIsCompleted) {
            query.exec("ALTER TABLE events ADD COLUMN is_completed INTEGER DEFAULT 0");
        }
        if (!hasGoogleEventId) {
            query.exec("ALTER TABLE events ADD COLUMN google_event_id TEXT");
        }"""

content = content.replace(target, replacement)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
