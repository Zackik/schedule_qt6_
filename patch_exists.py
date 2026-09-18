import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

target = """                bool exists = false;
                for (int i = 0; i < m_localEvents.size(); ++i) {
                    if (m_localEvents[i].googleEventId == ev.googleEventId && !ev.googleEventId.isEmpty()) {
                        exists = true;"""

replacement = """                bool exists = false;
                for (int i = 0; i < m_localEvents.size(); ++i) {
                    bool matchById = (m_localEvents[i].googleEventId == ev.googleEventId && !ev.googleEventId.isEmpty());
                    bool matchByContent = (m_localEvents[i].googleEventId.isEmpty() && 
                                           m_localEvents[i].title == ev.title && 
                                           m_localEvents[i].startTime == ev.startTime && 
                                           m_localEvents[i].endTime == ev.endTime);
                    
                    if (matchById || matchByContent) {
                        exists = true;
                        
                        // Nếu matchByContent, chúng ta vừa tìm thấy event local tương ứng trên Google
                        if (matchByContent) {
                            m_localEvents[i].googleEventId = ev.googleEventId;
                            m_localEvents[i].isSyncedGoogle = true;
                        }"""

content = content.replace(target, replacement)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
