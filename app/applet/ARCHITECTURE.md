# Architecture Report

## Overview
SmartSchedule is a desktop calendar application leveraging C++17 and the Qt 6 framework. It follows an MVC-inspired monolithic service structure utilizing Qt's native cross-platform toolchains.

## Core Services

### 1. Presentation Layer (Qt Widgets)
- `MainWindow`: Application orchestrator framing individual views.
- `ScheduleWidget`: Houses the active local timeline, syncing inputs, Pomodoro views, and the undo mechanism.
- `SettingsWidget`: Secures variable injections, manages configuration properties such as `google_client_secret` and `gemini_api_key`.
- `AIChibiWidget` / `ChatBubbleWidget`: Non-blocking frameless UI components dedicated to AI interaction overlays.

### 2. Logic & Control Layer
- `CalendarManager`: Central node operating the system's memory list `m_localEvents`. Owns timers for background notifications and Pomodoro increments. Emits signals for UI updating.
- `GoogleOAuthManager`: Encapsulates `QOAuth2AuthorizationCodeFlow`. Binds to a local HTTP server (`QOAuthHttpServerReplyHandler`) on port 8080 to silently capture OAuth redirect grants without manual copy-pasting.

### 3. API & Data Access Layer
- **Local Persistence:** Integrated SQLite wrapper (`QSqlDatabase`) running through `CalendarManager`. Bootstraps local caching table `events` and synchronizes structural migrations explicitly via `PRAGMA`.
- **Google API:** Consumes standard REST JSON structures utilizing `QNetworkAccessManager` wrapped securely under `Bearer` header authorizations managed by OAuth tokens.
- **Gemini API:** Directly interacts with `https://generativelanguage.googleapis.com` through `GeminiService` extracting JSON payloads via nested queries.

## Data Models
- **`ScheduleEvent` Struct:** Stores immutable identifiers (`id`, `googleEventId`), timespans (`startTime`, `endTime`), and enumerations (`Category`, `Priority`).

## Application State Machine (OAuth)
The architecture follows a strict state progression managed explicitly through `changeState(State)` mapping UI to underlying QtNetworkAuth logic:
- `DISCONNECTED`
- `AUTHORIZING`
- `AUTHORIZED`
- `CONNECTED`
- `REFRESHING`
- `INVALID`

## Build System
- **CMake (`CMakeLists.txt`):** The definitive build orchestrator. Triggers `AutoMOC`, `AutoUIC`, bounds Qt modules efficiently (Core, Gui, Widgets, Network, NetworkAuth, Sql), and injects remote module dependencies recursively via `FetchContent` (e.g., `googletest`, `spdlog`).
