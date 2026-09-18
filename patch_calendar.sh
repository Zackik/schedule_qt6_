#!/bin/bash
sed -i 's/m_googleAuth/m_oauthManager/g' src/calendar/CalendarManager.cpp
