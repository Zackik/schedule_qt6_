# Project Audit Report

## 1. Current Architecture
SmartSchedule is built using C++17 and Qt 6. It follows a monolithic architecture structure with several manager/service classes tightly coupled.
- **UI Layer:** Qt Widgets (`MainWindow`, `ScheduleWidget`, `SettingsWidget`, `AIChibiWidget`).
- **Managers:** `CalendarManager` is the core, handling Google Calendar API calls, SQLite local storage logic, and local Pomodoro state. `GoogleOAuthManager` handles OAuth 2.0 PKCE flow. `StorageManager` caches resources. `ResourceMonitor` tracks system CPU/RAM.
- **AI Integration:** `GeminiService` handles REST calls to Google's Gemini endpoint.

## 2. Module Dependencies
- **Qt Modules:** Core, Gui, Widgets, Network, NetworkAuth, Sql, PrintSupport, Multimedia (optional).
- **Third Party:** spdlog, GoogleTest (via FetchContent).
- **External Services:** Google OAuth 2.0 endpoint, Google Calendar API (v3), Google Gemini API (v1beta).

## 3. Known Bugs (Resolved)
- SQLite migration threw errors on start-up (`initDatabase`) because it attempted to blindly append columns without checking.
- Google Calendar Sync failed to update local counterparts when a remote Google update occurred due to faulty condition logic.
- Misconfigured CMake test targets omitted linking network layers for GoogleOAuthManager tests.

## 4. Potential Bugs
- `CalendarManager` is highly monolithic and risks breaking Single Responsibility Principle (SRP). Future features may strain this file.

## 5. Security Issues
- **Status:** PASS. No hardcoded credentials were found across grep scans. `.env.example` was cleaned. Uses secure Qt settings load (`QSettings` & Environment).

## 6. Build Issues
- **Status:** PASS. Local build system `cmake -B build` executes without warnings in CI. `Qml` missing references were completely stripped.

## 7. Test Issues
- **Status:** PASS. Test files were linked properly, preventing `QNetworkAccessManager` crash errors. File-dependent logic tests were isolated by `.clear()` commands.

## 8. OAuth Issues
- **Status:** PASS. Resolved previous bugs. Parameters `access_type=offline` and `prompt=consent` correctly inserted via `setModifyParametersFunction`. State-machine issues addressed and `RefreshingToken` is now explicitly handled in `GoogleOAuthManager::onOAuthStatusChanged()`.

## 9. Database Issues
- **Status:** PASS. Database utilizes QSQLITE natively. `PRAGMA table_info` introduced to safeguard version migrations against duplicate column crashes.

## 10. CI/CD Issues
- **Status:** NO KNOWN BLOCKERS. GitHub Actions YAML verified previously.

## 11. UX Issues
- Connection timeout messaging on Google API could be elaborated to distinct Error 401/403 visual messages in future scopes. 

## 12. Technical Debt
- High coupling inside `CalendarManager`. Network protocols (Google Sync) and Database IO are interleaved directly with internal list modifications. 

## 13. Recommended Redesign
- Separate `CalendarManager` into:
  - `DatabaseRepository`: For SQL transactions and mapping.
  - `GoogleCalendarSyncService`: For mapping Local <-> Google DTOs and initiating network calls.
  - `CalendarEngine`: For in-memory filtering and Pomodoro states.
