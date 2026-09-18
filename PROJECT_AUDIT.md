# Project Audit Report

## 1. Current Architecture
SmartSchedule is built using C++17 and Qt 6. It follows a monolithic architecture structure with several manager/service classes tightly coupled.
- **UI Layer:** Qt Widgets (`MainWindow`, `ScheduleWidget`, `SettingsWidget`, `AIChibiWidget`).
- **Managers:** `CalendarManager` is the core, handling Google Calendar API calls, SQLite local storage logic, and local Pomodoro state. `GoogleOAuthManager` handles OAuth 2.0 PKCE flow. `StorageManager` caches resources. `ResourceMonitor` tracks system CPU/RAM.
- **AI Integration:** `GeminiService` handles REST calls to Google's Gemini endpoint.

## 2. Recent Sync Architecture Improvements
- **Rate Limit Protection:** Implemented exponential backoff for Google API calls to prevent 403 Rate Limit Exceeded.
- **Pagination Support:** Added `nextPageToken` processing to `fetchGoogleCalendarEvents` to handle dense calendars correctly.
- **Comparison Engine:** Introduced `isSyncedGoogle` dirty flag mapping to prevent spamming Google Calendar with redundant UPDATE requests for local events that haven't actually changed.
- **24-hour Day View Constraint:** Enforced proper midnight-crossing logic in `getEventsForDate` via `event.start < dayEnd AND event.end > dayStart` replacing the flawed `date == selectedDate`.
- **Visible Range Sync:** Modified `handleGoogleSync` to capture the current grid UI date range (e.g. 6 visible weeks) rather than a rigid 30-day block, saving API calls.
