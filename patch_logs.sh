#!/bin/bash
sed -i '/void CalendarManager::syncEventToGoogle(const ScheduleEvent &event) {/a\    qDebug() << "Syncing event to Google Calendar:" << event.title;' src/calendar/CalendarManager.cpp
sed -i '/void CalendarManager::updateGoogleEvent(const ScheduleEvent &event) {/a\    qDebug() << "Updating Google Calendar event:" << event.title << "Google ID:" << event.googleEventId;' src/calendar/CalendarManager.cpp
sed -i '/void CalendarManager::deleteGoogleEvent(const QString &googleEventId) {/a\    qDebug() << "Deleting event from Google Calendar, Google ID:" << googleEventId;' src/calendar/CalendarManager.cpp
