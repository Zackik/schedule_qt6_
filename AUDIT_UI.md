# SMART SCHEDULE — UI/UX REDESIGN AUDIT (PHASE 0)

## 1. Executive Summary
- **Project**: SmartSchedule — Qt6 / C++ Modern Desktop Application
- **Core Objective**: Redesign UI/UX to Modern + Minimal + Futuristic + Professional + Luxury Light Theme without any backend/functional regression.
- **Current State**: Top-level `QTabWidget` with mixed inline styling, inconsistent color palettes, cluttered layouts on small viewports (<1100px), and lack of a cohesive design system and navigation hierarchy.

---

## 2. Architecture & File Inventory

### UI Layer (`src/ui/`)
- `MainWindow.h` / `MainWindow.cpp` / `MainWindow.ui`:
  - Current role: Template placeholder, unused in `main.cpp`.
  - Target role: Modern Application Frame hosting the persistent Sidebar, Top Header, and Main Content Container (`QStackedWidget`).
- `ScheduleWidget.h` / `ScheduleWidget.cpp`:
  - Current role: Primary central hub hosting all functional modules (`Calendar`, `Form`, `Events List`, `Gemini AI`, `Pomodoro`, `Stats/Charts`, `Settings`, `Logs`).
  - Target role: Main content coordinator containing view pages, styled with unified QSS and responsive layouts.
- `SettingsWidget.h` / `SettingsWidget.cpp`:
  - System settings widget displaying RAM usage (`ResourceMonitor`), Storage & Database size (`StorageManager`), and Google OAuth status.
- `companion/AIChibiWidget.h` / `companion/AIChibiWidget.cpp`:
  - Floating desktop mascot widget with sprite animation controller.
- `companion/ChatBubbleWidget.h` / `companion/ChatBubbleWidget.cpp`:
  - Floating speech bubble for AI notifications and proactive reminders.
- `CompletionChartWidget` & `DragDropCalendarWidget` (embedded in `ScheduleWidget.cpp`):
  - Custom paint widgets for weekly completion rates and calendar month view with drag-and-drop.

### Business & Service Layer (NEVER MODIFY FUNCTIONALITY)
- `CalendarManager`: SQLite persistence, event CRUD, conflict detection, undo stack, Pomodoro timer, Google Calendar sync, notifications.
- `GeminiService`: Google Gemini REST API client for natural language parsing, schedule optimization, daily briefing, and chat.
- `AssistantService`: Proactive reminder loop, global hotkey registration (`GlobalHotkeyManager`).
- `ResourceMonitor`: Linux `/proc/self/statm` RAM monitor.
- `StorageManager`: Database, logs, and cache folder size calculation and cleanup.

---

## 3. Comprehensive User Flows & Signal/Slot Map

| User Flow | UI Triggers | Business Logic / Slots | State / Event Updates |
|---|---|---|---|
| **Add Event** | `m_titleInput`, `m_startDateTime`, `m_endDateTime`, `m_costSpinBox`, `addBtn` | `ScheduleWidget::handleAddEvent()` -> `m_calendarManager->addLocalEvent()` | Emits `eventAdded()`, refreshes calendar cells & list view |
| **Delete Event** | `deleteBtn` | `m_calendarManager->deleteEvent(id)` | Emits `eventDeleted()`, saves undo action to `m_undoStack` |
| **Undo Action** | `m_undoBtn`, `Ctrl+Z` shortcut | `ScheduleWidget::handleUndo()` -> `m_calendarManager->undo()` | Emits `undoAvailabilityChanged(bool)` |
| **Mark Complete** | `completeBtn` | `m_calendarManager->updateEvent(id, updated)` | Toggles `isCompleted`, refreshes views |
| **Drag & Drop Reschedule** | Drag from `m_eventListView` -> Drop on `DragDropCalendarWidget` | `dragCalendar->onEventDropped(eventId, newDate)` -> `updateEvent()` | Preserves event duration & time, recalculates date |
| **Search & Filter** | `m_searchEventInput`, `m_filterCategoryCombo`, `m_sortCombo` | `ScheduleWidget::updateEventListView()` | Filters by title/desc/location/category, sorts by time/priority |
| **Google OAuth Login** | `m_googleLoginBtn` | `m_calendarManager->setupGoogleOAuth()`, `authenticateGoogle()` | Emits `googleAuthStatusChanged(bool, msg)` |
| **Google 2-Way Sync** | `m_googleSyncBtn` | `ScheduleWidget::handleGoogleSync()` -> `syncWithGoogle()` | Emits `googleSyncFinished(bool, msg)` |
| **Push Single to Google**| `pushGoogleBtn` | `m_calendarManager->syncEventToGoogle(e)` | Syncs single item |
| **Smart AI Schedule** | `m_aiSmartInput`, `m_aiSmartAddBtn` | `m_geminiService->parseNaturalLanguageSchedule()` | Populates `m_aiParsedEventsList`, enables `m_aiConfirmAddBtn` |
| **Daily AI Briefing** | `genBriefingBtn` | `m_geminiService->generateDailyBriefing()` | Renders markdown summary in `m_aiBriefingDisplay` |
| **AI Schedule Optimizer**| `optBtn` | `m_geminiService->optimizeSchedule()` | Renders optimization suggestions |
| **AI Free Chat** | `m_aiChatInput`, `sendChatBtn` | `m_geminiService->askGemini()` | Appends to `m_aiChatHistory` |
| **Pomodoro Work/Break** | `m_startPomoBtn`, `m_pausePomoBtn`, `m_resetPomoBtn` | `startPomodoro()`, `pausePomodoro()`, `resetPomodoro()` | Emits `pomodoroTick(secs, state)`, `pomodoroFinished(state)` |
| **Export CSV** | `exportCsvBtn` | `ScheduleWidget::handleExportCsv()` | Saves formatted CSV of all events |
| **Export Weekly PDF** | `m_exportWeeklyPdfBtn` | `ScheduleWidget::handleExportWeeklyPdf()` | Generates HTML weekly schedule and prints via `QPrinter` |
| **System Storage Cleanup**| `m_cleanupBtn` | `m_storageManager->clearCache()`, `clearAiLogs()` | Updates storage indicators |

---

## 4. Current UI/UX Deficiencies & Redesign Strategy

1. **Top-Level Navigation**:
   - *Current*: Cluttered horizontal tabs with emoji titles.
   - *Target*: Modern sleek Sidebar with App Brand Logo, navigation items (`Dashboard`, `Schedule`, `Tasks`, `Focus`, `Analytics`, `AI Assistant`, `Settings`, `Logs`), active indicator, soft hover states, and collapse support.
2. **Dashboard Missing**:
   - *Current*: User is dumped directly into complex calendar grid and raw input form.
   - *Target*: Executive Dashboard greeting the user with today's date, stats overview, quick tasks, next up agenda, and one-click Pomodoro/Briefing.
3. **Color Palette & Visual Tokens**:
   - *Current*: Chaotic color mixing (`#ea4335`, `#2563eb`, `#10b981`, `#8b5cf6`, `#f59e0b`, `#ef4444`).
   - *Target*: Cohesive Luxury Light Theme:
     - Neutral Canvas: `#F8FAFC` (Slate-50) / `#F1F5F9` (Slate-100)
     - Surfaces/Cards: `#FFFFFF` with refined subtle 1px border `#E2E8F0` and soft elevation
     - Primary Accent: `#4F46E5` (Indigo-600) / `#4338CA` (Indigo-700)
     - Secondary / Soft Accent: `#EEF2FF` (Indigo-50)
     - Text Primary: `#0F172A` (Slate-900)
     - Text Secondary: `#475569` (Slate-600)
     - Text Muted: `#94A3B8` (Slate-400)
     - Status Success: `#10B981` (Emerald-500) / `#ECFDF5`
     - Status Warning: `#F59E0B` (Amber-500) / `#FFFBEB`
     - Status Danger: `#EF4444` (Rose-500) / `#FEF2F2`
4. **Spacing & Typography Scale**:
   - Major Second typographic scale (12px caption, 13px small, 14px body, 16px subhead, 20px title, 26px display).
   - Strict 4px/8px grid system (margins 16px/24px, gaps 12px/16px, padding 12px/16px).
5. **Responsiveness**:
   - Minimum window target 960x640 up to 2560x1440.
   - Responsive layouts with `QSplitter`, dynamic card sizing, and scroll areas preventing any clipping.
