#pragma once

#include <QCalendarWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "src/ai/GeminiService.h"
#include "src/calendar/CalendarManager.h"

class ScheduleWidget : public QWidget {
    Q_OBJECT
public:
    explicit ScheduleWidget(CalendarManager *manager, GeminiService *gemini,
                            QWidget *parent = nullptr);

private slots:
    void handleAddEvent();
    void handleUndo();
    void updateUndoButton(bool canUndo);
    void handleGenerateDailySummary();
    void handleExportWeeklyPdf();
    void handleSmartAiAdd();
    void handleGenerateBriefing();
    void handleOptimizeSchedule();
    void handleSendAiChat();
    void updateEventListView();
    void updateStatistics();
    void refreshCalendarGrid();
    void handleDateSelected(const QDate &date);
    void handleGoogleSync();
    void handleSaveSettings();
    void handleExportCsv();

private:
    CalendarManager *m_calendarManager;
    GeminiService *m_geminiService;

    QTabWidget *m_mainTabs;

    // --- Tab 1: Quản lý Lịch & Sự kiện ---
    QCalendarWidget *m_calendarWidget;
    QLineEdit *m_titleInput;
    QLineEdit *m_descInput;
    QLineEdit *m_locInput;
    QDateTimeEdit *m_startDateTime;
    QDateTimeEdit *m_endDateTime;
    QComboBox *m_categoryCombo;
    QComboBox *m_priorityCombo;
    QDoubleSpinBox *m_costSpinBox;
    QLabel *m_conflictWarningLabel;
    QListWidget *m_eventListView;
    QComboBox *m_filterCategoryCombo;
    QComboBox *m_sortCombo;
    QLineEdit *m_searchEventInput;
    QPushButton *m_undoBtn;
    QShortcut *m_undoShortcut;

    QPushButton *m_aiSummaryBtn;
    QPushButton *m_exportWeeklyPdfBtn;
    QTextEdit *m_aiSummaryDisplay;

    // --- Tab 2: Pomodoro Focus ---
    QLabel *m_pomoTimerLabel;
    QLabel *m_pomoStateLabel;
    QLabel *m_pomoCountLabel;
    QPushButton *m_startPomoBtn;
    QPushButton *m_pausePomoBtn;
    QPushButton *m_resetPomoBtn;
    QProgressBar *m_pomoProgressBar;

    // --- Tab 3: Trợ lý AI Gemini ---
    QLineEdit *m_aiSmartInput;
    QPushButton *m_aiSmartAddBtn;
    QListWidget *m_aiParsedEventsList;
    QPushButton *m_aiConfirmAddBtn;
    QList<ScheduleEvent> m_pendingAiEvents;

    QTextEdit *m_aiBriefingDisplay;
    QLineEdit *m_aiChatInput;
    QTextEdit *m_aiChatHistory;

    // --- Tab 4: Thống kê & Chi phí ---
    QLabel *m_statTotalEvents;
    QLabel *m_statTotalCost;
    QLabel *m_statWorkCount;
    QLabel *m_statStudyCount;
    QLabel *m_statPersonalCount;
    QLabel *m_statHealthCount;
    QProgressBar *m_statWorkProgress;
    QProgressBar *m_statStudyProgress;
    QProgressBar *m_statPersonalProgress;
    QProgressBar *m_statHealthProgress;
    QWidget *m_completionChart;

    // --- Tab 5: Cài đặt & Google Calendar ---
    QLineEdit *m_googleClientIdInput;
    QLineEdit *m_googleClientSecretInput;
    QLabel *m_googleStatusLabel;
    QLineEdit *m_geminiApiKeyInput;
    QPushButton *m_googleLoginBtn;
    QPushButton *m_googleSyncBtn;

    // --- Tab 6: Logs ---
    QTextEdit *m_logsDisplay;

    // --- Dashboard & Navigation ---
    QListWidget *m_sidebarList;
    QLabel *m_headerTitleLabel;
    QLabel *m_headerSubtitleLabel;
    QLabel *m_headerSyncStatus;

    // --- Dashboard Overview Widgets ---
    QLabel *m_dashGreeting;
    QLabel *m_dashNextEvent;
    QLabel *m_dashTotalEvents;
    QLabel *m_dashCompletedEvents;
    QLabel *m_dashPomoCount;
    QLabel *m_dashTotalCost;
    QListWidget *m_dashTodayList;

    void setupUI();
    QWidget *createSidebar();
    QWidget *createHeaderToolbar();
    QWidget *createDashboardTab();
    QWidget *createScheduleTab();
    QWidget *createPomodoroTab();
    QWidget *createAiTab();
    QWidget *createStatsTab();
    QWidget *createSettingsTab();
    QWidget *createLogsTab();
    void refreshDashboard();
    void applyModernStyle();
};
