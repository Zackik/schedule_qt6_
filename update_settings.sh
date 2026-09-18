#!/bin/bash
sed -i 's/explicit SettingsWidget(QWidget \*parent = nullptr);/explicit SettingsWidget(class CalendarManager \*calendarManager, QWidget \*parent = nullptr);/g' src/ui/SettingsWidget.h
sed -i 's/SettingsWidget::SettingsWidget(QWidget \*parent) : QWidget(parent)/SettingsWidget::SettingsWidget(CalendarManager \*calendarManager, QWidget \*parent) : QWidget(parent)/g' src/ui/SettingsWidget.cpp
sed -i 's/m_oauthManager = new GoogleOAuthManager(this);/m_oauthManager = calendarManager->oauthManager();\n    if (!m_oauthManager) {\n        calendarManager->setupGoogleOAuth("","");\n        m_oauthManager = calendarManager->oauthManager();\n    }/g' src/ui/SettingsWidget.cpp
sed -i 's/auto \*systemSettings = new SettingsWidget(this);/auto \*systemSettings = new SettingsWidget(m_calendarManager, this);/g' src/ui/ScheduleWidget.cpp
