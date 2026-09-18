# Changelog

## [2.0.0-audited]

### Added
- **SQLite Safe Migrations**: Built a dedicated `PRAGMA table_info(events)` routine inside `CalendarManager::initDatabase()` to dynamically verify schema presence before injecting `ALTER TABLE` commands.
- **Calendar Event Date Filters Test**: Built deterministic memory tests `CalendarManagerTest.EventDateFiltering` using `.clear()` memory logic ensuring isolated logic runs independently from persistent file systems.
- **Google OAuth Refined States**: Explicit state handler logic mapped for `RefreshingToken`.

### Fixed
- **Google 2-Way Sync Duplicates/Loss**: Fixed `CalendarManager::fetchGoogleCalendarEvents`. Remote updates applied against Google Calendar are now systematically overwritten back into existing mapped local states without dropping them.
- **Test Build Linkages**: CMake target `SmartSchedule_Tests` correctly bounds `Qt6::Widgets`, `Qt6::Network`, `Qt6::NetworkAuth` providing clean networking functionality to unit suites.

### Audited (Unmodified Core)
- **Local SQLite Save Methods**: `saveEventsToDatabase()` currently deletes and bulk inserts due to simple object scope. While sub-optimal for massive scales, it handles current performance requirements optimally and was left untouched to avoid breaking product logic.
- **Gemini Architecture**: Validated as functionally sound and appropriately isolated inside `GeminiService`.
- **UI Logic / Hotkeys**: Left precisely according to previous business mandates.

### Security
- Swept entirety of current source tree.
- Zero remaining instances of hardcoded Client Secrets or Gemini Tokens.
- Cleaned legacy configuration layouts ensuring `.env` reliance is optional or excluded from source control.
