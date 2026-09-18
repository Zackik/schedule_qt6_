import re

with open('src/calendar/CalendarManager.cpp', 'r') as f:
    content = f.read()

target1 = """    if (!pageToken.isEmpty()) {
        query.addQueryItem("pageToken", pageToken);
    }"""
replacement1 = """    if (!pageToken.isEmpty()) {
        query.addQueryItem("pageToken", pageToken);
    } else {
        // Lần gọi đầu tiên (không có pageToken), reset danh sách fetched IDs
        m_syncFetchedGoogleIds.clear();
    }"""
content = content.replace(target1, replacement1)

target2 = """                // Nếu matchByContent, chúng ta vừa tìm thấy event local tương ứng trên Google
                        if (matchByContent) {
                            m_localEvents[i].googleEventId = ev.googleEventId;
                            m_localEvents[i].isSyncedGoogle = true;
                        }"""
replacement2 = """                // Nếu matchByContent, chúng ta vừa tìm thấy event local tương ứng trên Google
                        if (matchByContent) {
                            m_localEvents[i].googleEventId = ev.googleEventId;
                            m_localEvents[i].isSyncedGoogle = true;
                        }
                        m_syncFetchedGoogleIds.insert(ev.googleEventId);"""
content = content.replace(target2, replacement2)

target3 = """                if (!exists) {
                    m_localEvents.append(ev);
                    fetchedList.append(ev);
                }"""
replacement3 = """                if (!exists) {
                    m_localEvents.append(ev);
                    fetchedList.append(ev);
                    m_syncFetchedGoogleIds.insert(ev.googleEventId);
                }"""
content = content.replace(target3, replacement3)

target4 = """            if (!nextPageToken.isEmpty()) {
                qDebug() << "Fetching next page of Google events...";
                fetchGoogleCalendarEvents(start, end, nextPageToken);
            } else {
                saveEventsToDatabase();
                emit eventsReloaded();
                emit googleEventsFetched(fetchedList);"""
replacement4 = """            if (!nextPageToken.isEmpty()) {
                qDebug() << "Fetching next page of Google events...";
                fetchGoogleCalendarEvents(start, end, nextPageToken);
            } else {
                // Đã tải xong tất cả các trang
                // Xoá các event trên local (trong khoảng thời gian sync) mà KHÔNG có mặt trên Google (bị xoá trên Google)
                QDateTime syncStart(queryStart, QTime(0, 0));
                QDateTime syncEnd(queryEnd.addDays(1), QTime(0, 0));
                
                for (int i = m_localEvents.size() - 1; i >= 0; --i) {
                    const auto& e = m_localEvents[i];
                    if (!e.googleEventId.isEmpty()) {
                        // Nếu event này thuộc khoảng sync
                        if (!(e.endTime <= syncStart || e.startTime >= syncEnd)) {
                            // Và không được Google trả về -> Nó đã bị xoá trên Google
                            if (!m_syncFetchedGoogleIds.contains(e.googleEventId)) {
                                qDebug() << "Xoá sự kiện cục bộ vì đã bị xoá trên Google:" << e.title;
                                m_localEvents.removeAt(i);
                            }
                        }
                    }
                }
                
                saveEventsToDatabase();
                emit eventsReloaded();
                emit googleEventsFetched(fetchedList);"""
content = content.replace(target4, replacement4)

with open('src/calendar/CalendarManager.cpp', 'w') as f:
    f.write(content)
