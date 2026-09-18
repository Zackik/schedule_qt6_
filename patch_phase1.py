import re

with open('src/calendar/CalendarManager.h', 'r') as f:
    content = f.read()

# Add matches method to ScheduleEvent
matches_code = """
    bool matches(const ScheduleEvent &other) const {
        return title == other.title &&
               description == other.description &&
               location == other.location &&
               startTime == other.startTime &&
               endTime == other.endTime;
    }
"""
content = re.sub(r'static ScheduleEvent fromJson\(const QJsonObject &json\);', 
                 'static ScheduleEvent fromJson(const QJsonObject &json);\n' + matches_code, 
                 content)

with open('src/calendar/CalendarManager.h', 'w') as f:
    f.write(content)
    
with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

# Fix updateEvent to mark isSyncedGoogle = false
content = re.sub(
    r'm_localEvents\[i\] = newEvent;\n\s*saveEventsToDatabase\(\);',
    'm_localEvents[i] = newEvent;\n            m_localEvents[i].isSyncedGoogle = false; // Mark dirty\n            saveEventsToDatabase();',
    content
)

# Fix addLocalEvent to mark isSyncedGoogle = false
content = re.sub(
    r'newEv\.id = QUuid::createUuid\(\)\.toString\(QUuid::WithoutBraces\);\n\s*\}',
    'newEv.id = QUuid::createUuid().toString(QUuid::WithoutBraces);\n    }\n    newEv.isSyncedGoogle = false; // Mark dirty',
    content
)

# Fix syncAllLocalEventsToGoogle loop
def repl_sync_loop(m):
    return """for (const auto &ev : m_localEvents) {
        if (!ev.isSyncedGoogle) {
            if (ev.googleEventId.isEmpty()) {
                syncEventToGoogle(ev);
            } else {
                updateGoogleEvent(ev);
            }
        }
    }"""

content = re.sub(
    r'for \(const auto &ev : m_localEvents\) \{\s*if \(!ev\.isSyncedGoogle && ev\.googleEventId\.isEmpty\(\)\) \{\s*syncEventToGoogle\(ev\);\s*\} else if \(ev\.isSyncedGoogle && !ev\.googleEventId\.isEmpty\(\)\) \{\s*updateGoogleEvent\(ev\);\s*\}\s*\}',
    repl_sync_loop,
    content
)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
