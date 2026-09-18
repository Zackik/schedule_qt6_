#include <gtest/gtest.h>
#include "../src/calendar/CalendarManager.h"
#include <QDateTime>
#include <QUuid>

TEST(CalendarManagerTest, CategoryConversion) {
    EXPECT_EQ(CalendarManager::categoryToString(Category::Study), "Study");
    EXPECT_EQ(CalendarManager::stringToCategory("Công việc"), Category::Work);
    EXPECT_EQ(CalendarManager::stringToCategory("Cá nhân"), Category::Personal);
}

TEST(CalendarManagerTest, PriorityConversion) {
    EXPECT_EQ(CalendarManager::priorityToString(Priority::High), "High");
    EXPECT_EQ(CalendarManager::stringToPriority("Thấp"), Priority::Low);
    EXPECT_EQ(CalendarManager::stringToPriority("Cao"), Priority::High);
}

TEST(CalendarManagerTest, EventDateFiltering) {
    CalendarManager manager;
    ScheduleEvent e1, e2;
    e1.id = "1";
    e1.startTime = QDateTime::currentDateTime();
    e1.endTime = e1.startTime.addSecs(3600);
    e2.id = "2";
    e2.startTime = QDateTime::currentDateTime().addDays(2);
    e2.endTime = e2.startTime.addSecs(3600);
    manager.m_localEvents.clear();
    manager.m_localEvents.append(e1);
    manager.m_localEvents.append(e2);

    auto list = manager.getEventsForDate(QDate::currentDate());
    for(const auto& e: list) {
        std::cout << "Event ID: " << e.id.toStdString() << ", Start: " << e.startTime.toString().toStdString() << ", End: " << e.endTime.toString().toStdString() << std::endl;
    }
    EXPECT_EQ(list.size(), 1);
    if(list.size() > 0) {
        EXPECT_EQ(list[0].id, "1");
    }
}
