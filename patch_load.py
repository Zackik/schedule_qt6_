import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

target = """        event.googleEventId = query.value("google_event_id").toString();
        m_localEvents.append(event);
    }
}"""

replacement = """        event.googleEventId = query.value("google_event_id").toString();
        
        // Deduplicate
        bool duplicate = false;
        for (const auto& existing : m_localEvents) {
            if (!event.googleEventId.isEmpty() && existing.googleEventId == event.googleEventId) {
                duplicate = true;
                break;
            }
            if (event.googleEventId.isEmpty() && existing.title == event.title && 
                existing.startTime == event.startTime && existing.endTime == event.endTime) {
                duplicate = true;
                break;
            }
        }
        
        if (!duplicate) {
            m_localEvents.append(event);
        }
    }
}"""

content = content.replace(target, replacement)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
