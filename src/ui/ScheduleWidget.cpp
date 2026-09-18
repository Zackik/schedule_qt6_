#include "src/ui/ScheduleWidget.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QHeaderView>
#include <QLocale>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QScrollArea>
#include <QSettings>
#include <QTableView>
#include <QTextDocument>
#include <QTextStream>
#include <algorithm>

#include "src/ui/SettingsWidget.h"

class CompletionChartWidget : public QWidget {
public:
    explicit CompletionChartWidget(CalendarManager *manager, QWidget *parent = nullptr)
        : QWidget(parent), m_manager(manager) {
        setMinimumHeight(200);
    }

    void updateChart() {
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setPen(QPen(QColor("#E2E8F0"), 1));
        painter.setBrush(QColor("#FFFFFF"));
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);

        QDate today = QDate::currentDate();
        int dayOfWeek = today.dayOfWeek();
        QDate startOfWeek = today.addDays(1 - dayOfWeek);

        QList<ScheduleEvent> weekEvents;
        for (int i = 0; i < 7; ++i) {
            weekEvents.append(m_manager->getEventsForDate(startOfWeek.addDays(i)));
        }

        std::map<Category, int> totalTasks;
        std::map<Category, int> completedTasks;

        totalTasks[Category::Study] = 0;
        totalTasks[Category::Work] = 0;
        totalTasks[Category::Personal] = 0;
        totalTasks[Category::Health] = 0;
        totalTasks[Category::Other] = 0;

        for (const auto &ev : weekEvents) {
            totalTasks[ev.category]++;
            if (ev.isCompleted)
                completedTasks[ev.category]++;
        }

        QList<Category> categories = {Category::Work, Category::Study, Category::Personal,
                                      Category::Health, Category::Other};
        QList<QColor> colors = {QColor("#3b82f6"), QColor("#6366f1"), QColor("#ec4899"),
                                QColor("#10b981"), QColor("#64748b")};

        int barWidth = width() / (categories.size() * 2);
        int maxH = height() - 60;  // Leave space for labels and title

        painter.setPen(QColor("#0F172A"));
        painter.setFont(QFont("Arial", 11, QFont::Bold));
        painter.drawText(QRect(0, 8, width(), 22), Qt::AlignCenter,
                         "Tỉ lệ hoàn thành công việc tuần này (Completion Rates)");

        painter.setFont(QFont("Arial", 9));

        int xOffset = barWidth / 2;
        for (int i = 0; i < categories.size(); ++i) {
            Category cat = categories[i];
            int total = totalTasks[cat];
            int comp = completedTasks[cat];

            float rate = total > 0 ? (float)comp / total : 0;
            int h = rate * maxH;

            // Draw background bar (total)
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#F1F5F9"));
            QRect bgRect(xOffset, 36 + (maxH - maxH), barWidth, maxH);
            painter.drawRoundedRect(bgRect, 4, 4);

            // Draw filled bar (completed)
            painter.setBrush(colors[i]);
            QRect fgRect(xOffset, 36 + (maxH - h), barWidth, h);
            painter.drawRoundedRect(fgRect, 4, 4);

            // Draw label
            painter.setPen(QColor("#475569"));
            painter.drawText(QRect(xOffset - 10, height() - 25, barWidth + 20, 20), Qt::AlignCenter,
                             CalendarManager::categoryToString(cat));

            // Draw percentage
            if (total > 0) {
                QString pct = QString::number((int)(rate * 100)) + "%";
                painter.drawText(QRect(xOffset, 36 + (maxH - h) - 20, barWidth, 20),
                                 Qt::AlignCenter, pct);
            }

            xOffset += barWidth * 2;
        }
    }

private:
    CalendarManager *m_manager;
};

class DragDropCalendarWidget : public QCalendarWidget {
public:
    explicit DragDropCalendarWidget(CalendarManager *manager, QWidget *parent = nullptr)
        : QCalendarWidget(parent),
          m_manager(manager),
          m_eventList(nullptr),
          m_searchInput(nullptr) {
        setAcceptDrops(true);
        QTableView *view = findChild<QTableView *>("qt_calendar_calendarview");
        if (view) {
            view->setAcceptDrops(true);
            view->viewport()->setAcceptDrops(true);
            view->installEventFilter(this);
            view->viewport()->installEventFilter(this);
        }
    }

    void setEventList(QListWidget *list) {
        m_eventList = list;
    }
    void setSearchInput(QLineEdit *searchInput) {
        m_searchInput = searchInput;
    }

    std::function<void(const QString &, const QDate &)> onEventDropped;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        QTableView *view = findChild<QTableView *>("qt_calendar_calendarview");
        if (view && (watched == view || watched == view->viewport())) {
            if (event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove) {
                QDropEvent *de = static_cast<QDropEvent *>(event);
                if (de->source() == m_eventList) {
                    de->acceptProposedAction();
                    return true;
                }
            } else if (event->type() == QEvent::Drop) {
                QDropEvent *de = static_cast<QDropEvent *>(event);
                if (de->source() == m_eventList && m_eventList) {
                    QListWidgetItem *item = m_eventList->currentItem();
                    if (item) {
                        QString eventId = item->data(Qt::UserRole).toString();

                        QPoint viewPos = de->position().toPoint();
                        if (watched == view) {
                            viewPos = view->viewport()->mapFrom(view, de->position().toPoint());
                        }

                        QModelIndex index = view->indexAt(viewPos);
                        if (index.isValid()) {
                            view->selectionModel()->setCurrentIndex(
                                index, QItemSelectionModel::ClearAndSelect);
                            QDate droppedDate = selectedDate();
                            if (onEventDropped) {
                                onEventDropped(eventId, droppedDate);
                            }
                        }
                    }
                    de->acceptProposedAction();
                    return true;
                }
            }
        }
        return QCalendarWidget::eventFilter(watched, event);
    }

    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override {
        QCalendarWidget::paintCell(painter, rect, date);

        if (!m_manager)
            return;

        auto events = m_manager->getEventsForDate(date);
        if (events.isEmpty())
            return;

        // Apply search filter if search input is set
        if (m_searchInput && !m_searchInput->text().isEmpty()) {
            QString search = m_searchInput->text().toLower();
            QList<ScheduleEvent> filteredEvents;
            for (const auto &ev : events) {
                QString catStr = CalendarManager::categoryToString(ev.category).toLower();
                if (ev.title.toLower().contains(search) ||
                    ev.description.toLower().contains(search) ||
                    ev.location.toLower().contains(search) || catStr.contains(search)) {
                    filteredEvents.append(ev);
                }
            }
            events = filteredEvents;
        }

        if (events.isEmpty())
            return;

        int yOffset = rect.top() + 20;

        painter->save();
        for (const auto &ev : events) {
            if (yOffset > rect.bottom() - 14) {
                painter->setPen(QColor("#64748b"));
                painter->setFont(QFont("Arial", 7, QFont::Bold));
                painter->drawText(QRect(rect.left() + 2, yOffset, rect.width() - 4, 12),
                                  Qt::AlignCenter, "...");
                break;
            }

            QColor bgColor = "#10B981";  // Default/Low: Green
            if (ev.priority == Priority::High)
                bgColor = "#EF4444";  // Red
            else if (ev.priority == Priority::Medium)
                bgColor = "#F59E0B";  // Amber

            QRect bgRect(rect.left() + 2, yOffset, rect.width() - 4, 13);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);
            painter->setBrush(bgColor);
            painter->drawRoundedRect(bgRect, 3, 3);

            painter->setPen(Qt::white);
            QFont f = painter->font();
            f.setPointSize(7);
            f.setBold(true);
            painter->setFont(f);

            QString displayTitle =
                painter->fontMetrics().elidedText(ev.title, Qt::ElideRight, rect.width() - 8);
            painter->drawText(QRect(rect.left() + 5, yOffset, rect.width() - 10, 13),
                              Qt::AlignLeft | Qt::AlignVCenter, displayTitle);

            yOffset += 15;
        }
        painter->restore();
    }

private:
    CalendarManager *m_manager;
    QListWidget *m_eventList;
    QLineEdit *m_searchInput;
};

static QTextEdit *g_globalLogDisplay = nullptr;

void customLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString typeStr;
    QString color;
    switch (type) {
        case QtDebugMsg:
            typeStr = "[DEBUG]";
            color = "#6B7280";
            break;  // Gray
        case QtInfoMsg:
            typeStr = "[INFO]";
            color = "#3B82F6";
            break;  // Blue
        case QtWarningMsg:
            typeStr = "[WARN]";
            color = "#F59E0B";
            break;  // Yellow
        case QtCriticalMsg:
            typeStr = "[ERROR]";
            color = "#EF4444";
            break;  // Red
        case QtFatalMsg:
            typeStr = "[FATAL]";
            color = "#B91C1C";
            break;  // Dark Red
    }

    QString formattedMsg = QString("<span style='color: %1;'><b>%2 %3:</b> %4</span>")
                               .arg(color, timeStr, typeStr, msg.toHtmlEscaped());

    if (g_globalLogDisplay) {
        QMetaObject::invokeMethod(g_globalLogDisplay, "append", Qt::QueuedConnection,
                                  Q_ARG(QString, formattedMsg));
    }

    fprintf(stderr, "%s %s: %s\n", qPrintable(timeStr), qPrintable(typeStr), qPrintable(msg));
    fflush(stderr);
}

ScheduleWidget::ScheduleWidget(CalendarManager *manager, GeminiService *gemini, QWidget *parent)
    : QWidget(parent),
      m_calendarManager(manager),
      m_geminiService(gemini),
      m_completionChart(nullptr),
      m_sidebarList(nullptr),
      m_headerTitleLabel(nullptr),
      m_headerSubtitleLabel(nullptr),
      m_headerSyncStatus(nullptr),
      m_dashGreeting(nullptr),
      m_dashNextEvent(nullptr),
      m_dashTotalEvents(nullptr),
      m_dashCompletedEvents(nullptr),
      m_dashPomoCount(nullptr),
      m_dashTotalCost(nullptr),
      m_dashTodayList(nullptr) {
    qInstallMessageHandler(customLogHandler);

    // Load Settings
    QSettings settings;
    QString savedGeminiKey = settings.value("gemini_api_key", "").toString();
    if (!savedGeminiKey.isEmpty()) {
        m_geminiService->setApiKey(savedGeminiKey);
    }

    setupUI();
    applyModernStyle();

    // Auto load Google Auth if available
    QString savedClientId = settings.value("google_client_id", "").toString();
    QString savedClientSecret = settings.value("google_client_secret", "").toString();
    if (!savedClientId.isEmpty() && !savedClientSecret.isEmpty()) {
        m_googleClientIdInput->setText(savedClientId);
        m_googleClientSecretInput->setText(savedClientSecret);
        // Automatically setup but don't force popup immediately
        m_calendarManager->setupGoogleOAuth(savedClientId, savedClientSecret);
    }

    // Kết nối tín hiệu từ CalendarManager
    connect(m_calendarManager, &CalendarManager::eventAdded, this,
            [this](const ScheduleEvent &) { refreshCalendarGrid(); });
    connect(m_calendarManager, &CalendarManager::eventUpdated, this,
            [this](const ScheduleEvent &) { refreshCalendarGrid(); });
    connect(m_calendarManager, &CalendarManager::eventDeleted, this,
            [this](const QString &) { refreshCalendarGrid(); });
    connect(m_calendarManager, &CalendarManager::eventsReloaded, this,
            &ScheduleWidget::refreshCalendarGrid);
    connect(m_calendarManager, &CalendarManager::undoAvailabilityChanged, this,
            &ScheduleWidget::updateUndoButton);

    connect(m_calendarManager, &CalendarManager::googleAuthStatusChanged, this,
            [this](bool isAuth, const QString &msg) {
                m_googleStatusLabel->setText(isAuth ? "🟢 Google Calendar: Đã kết nối"
                                                    : "⚪ Google Calendar: Chưa kết nối");
                if (m_headerSyncStatus) {
                    m_headerSyncStatus->setText(isAuth ? "🟢 Google: Connected" : "⚪ Google: Offline");
                    m_headerSyncStatus->setStyleSheet(isAuth ? "font-size: 11px; font-weight: 600; color: #10B981;" : "font-size: 11px; font-weight: 600; color: #94A3B8;");
                }
                m_googleSyncBtn->setEnabled(isAuth);

                if (isAuth) {
                    customLogHandler(QtInfoMsg, QMessageLogContext(), "Google Auth: " + msg);
                } else {
                    customLogHandler(QtWarningMsg, QMessageLogContext(), "Google Auth: " + msg);
                }
            });

    connect(m_calendarManager, &CalendarManager::googleSyncFinished, this,
            [this](bool success, const QString &msg) {
                if (success) {
                    customLogHandler(QtInfoMsg, QMessageLogContext(), msg);
                } else {
                    customLogHandler(QtWarningMsg, QMessageLogContext(), msg);
                    QMessageBox::warning(this, "Lỗi Đồng bộ Google", msg);
                }
            });

    connect(m_calendarManager, &CalendarManager::notificationTriggered, this,
            [this](const QString &title, const QString &msg) {
                QMessageBox::information(this, title, msg);
            });

    connect(m_calendarManager, &CalendarManager::pomodoroTick, this,
            [this](int secs, PomodoroState state) {
                int m = secs / 60;
                int s = secs % 60;
                m_pomoTimerLabel->setText(
                    QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));

                int totalSecs = (state == PomodoroState::Working) ? 25 * 60 : 5 * 60;
                int progress = 100 - (secs * 100 / totalSecs);
                m_pomoProgressBar->setValue(progress);

                if (state == PomodoroState::Working) {
                    m_pomoStateLabel->setText("🎯 Đang tập trung (Deep Work)");
                    m_pomoStateLabel->setStyleSheet(
                        "color: #4F46E5; font-weight: bold; font-size: 14px;");
                } else if (state == PomodoroState::ShortBreak) {
                    m_pomoStateLabel->setText("☕ Nghỉ ngắn (5 phút)");
                    m_pomoStateLabel->setStyleSheet(
                        "color: #10B981; font-weight: bold; font-size: 14px;");
                } else if (state == PomodoroState::LongBreak) {
                    m_pomoStateLabel->setText("🌴 Nghỉ dài (15 phút)");
                    m_pomoStateLabel->setStyleSheet(
                        "color: #8B5CF6; font-weight: bold; font-size: 14px;");
                } else {
                    m_pomoStateLabel->setText("⚪ Sẵn sàng");
                    m_pomoStateLabel->setStyleSheet("color: #64748B; font-size: 14px;");
                }
            });

    connect(
        m_calendarManager, &CalendarManager::pomodoroFinished, this, [this](PomodoroState state) {
            m_pomoCountLabel->setText(
                QString("Số hiệp hoàn thành: %1").arg(m_calendarManager->getCompletedPomodoros()));
            refreshDashboard();
            if (state == PomodoroState::Working) {
                QMessageBox::information(
                    this, "Pomodoro",
                    "🎉 Tuyệt vời! Bạn đã hoàn thành 1 hiệp làm việc. Hãy nghỉ ngơi 5 phút.");
            } else {
                QMessageBox::information(this, "Pomodoro",
                                         "⏰ Hết giờ nghỉ! Sẵn sàng cho hiệp làm việc tiếp theo.");
            }
        });

    refreshCalendarGrid();
}

QWidget *ScheduleWidget::createSidebar() {
    auto *sidebar = new QWidget(this);
    sidebar->setFixedWidth(230);
    sidebar->setStyleSheet("background-color: #FFFFFF; border-right: 1px solid #E2E8F0;");

    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(16, 20, 16, 20);
    sidebarLayout->setSpacing(12);

    // 1. App Brand Header
    auto *brandRow = new QHBoxLayout();
    brandRow->setSpacing(10);

    auto *logoIcon = new QLabel("🗓️", sidebar);
    logoIcon->setStyleSheet("font-size: 24px;");

    auto *brandCol = new QVBoxLayout();
    brandCol->setSpacing(0);

    auto *brandTitle = new QLabel("SmartSchedule", sidebar);
    brandTitle->setStyleSheet("font-size: 15px; font-weight: 700; color: #0F172A;");

    auto *brandBadge = new QLabel("PRO AI", sidebar);
    brandBadge->setStyleSheet("font-size: 10px; font-weight: 700; color: #4F46E5; background: #EEF2FF; border-radius: 4px; padding: 2px 6px;");

    auto *titleRow = new QHBoxLayout();
    titleRow->addWidget(brandTitle);
    titleRow->addWidget(brandBadge);
    titleRow->addStretch();

    auto *brandSubtitle = new QLabel("Productivity Suite", sidebar);
    brandSubtitle->setStyleSheet("font-size: 11px; color: #94A3B8;");

    brandCol->addLayout(titleRow);
    brandCol->addWidget(brandSubtitle);

    brandRow->addWidget(logoIcon);
    brandRow->addLayout(brandCol);
    sidebarLayout->addLayout(brandRow);

    sidebarLayout->addSpacing(8);

    // 2. Quick Action: + New Event
    auto *quickAddBtn = new QPushButton("➕  New Event", sidebar);
    quickAddBtn->setObjectName("primaryBtn");
    quickAddBtn->setCursor(Qt::PointingHandCursor);
    quickAddBtn->setStyleSheet(
        "QPushButton { background-color: #4F46E5; color: #FFFFFF; border-radius: 8px; font-weight: 600; padding: 10px; font-size: 13px; border: none; }"
        "QPushButton:hover { background-color: #4338CA; }"
        "QPushButton:pressed { background-color: #3730A3; }"
    );
    connect(quickAddBtn, &QPushButton::clicked, [this]() {
        if (m_sidebarList) {
            m_sidebarList->setCurrentRow(1); // Switch to Schedule tab
        }
        if (m_titleInput) {
            m_titleInput->setFocus();
            m_titleInput->selectAll();
        }
    });
    sidebarLayout->addWidget(quickAddBtn);

    sidebarLayout->addSpacing(8);

    // 3. Navigation List
    m_sidebarList = new QListWidget(sidebar);
    m_sidebarList->setObjectName("sidebarList");
    m_sidebarList->setFocusPolicy(Qt::NoFocus);

    struct NavItem {
        QString text;
        QString tooltip;
    };
    QList<NavItem> items = {
        {"📊  Dashboard", "Overview and today's schedule summary"},
        {"📅  Schedule", "Calendar, events, drag & drop agenda"},
        {"⏱️  Focus", "Pomodoro deep work timer"},
        {"🤖  AI Assistant", "Gemini AI briefing and schedule optimizer"},
        {"📈  Analytics", "Productivity stats and category breakdown"},
        {"⚙️  Settings", "Google OAuth, Gemini API, system monitoring"},
        {"📝  System Logs", "Activity and sync logs"}
    };

    for (const auto &item : items) {
        auto *listItem = new QListWidgetItem(item.text, m_sidebarList);
        listItem->setToolTip(item.tooltip);
        listItem->setSizeHint(QSize(190, 40));
    }

    connect(m_sidebarList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < m_mainTabs->count()) {
            m_mainTabs->setCurrentIndex(row);

            QString titles[] = {
                "Dashboard Overview",
                "Schedule & Calendar",
                "Focus Session (Pomodoro)",
                "Gemini AI Assistant",
                "Analytics & Reports",
                "Settings & Integrations",
                "System Activity Logs"
            };
            QString subtitles[] = {
                "Real-time productivity summary and today's agenda",
                "Interactive visual calendar and day agenda planning",
                "Deep work sessions and intervals tracker",
                "Intelligent event extraction, briefing, and time optimization",
                "Completion rates, cost tracking, and productivity trends",
                "Cloud sync, Google Calendar OAuth, and Gemini API keys",
                "Real-time diagnostic events and sync audit trail"
            };
            if (row < 7 && m_headerTitleLabel && m_headerSubtitleLabel) {
                m_headerTitleLabel->setText(titles[row]);
                m_headerSubtitleLabel->setText(subtitles[row]);
            }
        }
    });

    sidebarLayout->addWidget(m_sidebarList);
    sidebarLayout->addStretch();

    // 4. Bottom Section: Google Sync Chip & Undo Helper
    auto *bottomFrame = new QFrame(sidebar);
    bottomFrame->setStyleSheet("background-color: #F8FAFC; border-radius: 8px; border: 1px solid #E2E8F0; padding: 10px;");
    auto *bottomLayout = new QVBoxLayout(bottomFrame);
    bottomLayout->setContentsMargins(6, 6, 6, 6);
    bottomLayout->setSpacing(6);

    m_headerSyncStatus = new QLabel("⚪ Google: Disconnected", bottomFrame);
    m_headerSyncStatus->setStyleSheet("font-size: 11px; font-weight: 600; color: #64748B;");
    bottomLayout->addWidget(m_headerSyncStatus);

    auto *quickUndo = new QPushButton("↩️ Undo Action", bottomFrame);
    quickUndo->setToolTip("Undo last calendar change (Ctrl+Z)");
    quickUndo->setStyleSheet("QPushButton { background-color: transparent; border: 1px solid #CBD5E1; border-radius: 6px; padding: 4px; font-size: 11px; color: #475569; }"
                             "QPushButton:hover { background-color: #E2E8F0; color: #0F172A; }");
    connect(quickUndo, &QPushButton::clicked, this, &ScheduleWidget::handleUndo);
    bottomLayout->addWidget(quickUndo);

    sidebarLayout->addWidget(bottomFrame);
    return sidebar;
}

QWidget *ScheduleWidget::createHeaderToolbar() {
    auto *header = new QWidget(this);
    header->setFixedHeight(68);
    header->setStyleSheet("background-color: #FFFFFF; border-bottom: 1px solid #E2E8F0; padding: 0 24px;");

    auto *layout = new QHBoxLayout(header);
    layout->setContentsMargins(24, 0, 24, 0);
    layout->setSpacing(16);

    // Left: Dynamic Page Title & Subtitle
    auto *titleCol = new QVBoxLayout();
    titleCol->setAlignment(Qt::AlignVCenter);
    titleCol->setSpacing(2);

    m_headerTitleLabel = new QLabel("Dashboard Overview", header);
    m_headerTitleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #0F172A;");

    m_headerSubtitleLabel = new QLabel(QString("%1 • Welcome back").arg(QDate::currentDate().toString("dddd, MMMM d, yyyy")), header);
    m_headerSubtitleLabel->setStyleSheet("font-size: 12px; color: #64748B;");

    titleCol->addWidget(m_headerTitleLabel);
    titleCol->addWidget(m_headerSubtitleLabel);
    layout->addLayout(titleCol);

    layout->addStretch();

    // Right: Search Input
    m_searchEventInput = new QLineEdit(header);
    m_searchEventInput->setPlaceholderText("🔍  Search events, tags, locations...");
    m_searchEventInput->setFixedWidth(260);
    m_searchEventInput->setFixedHeight(36);
    m_searchEventInput->setStyleSheet(
        "QLineEdit { background: #F8FAFC; border: 1px solid #E2E8F0; border-radius: 8px; padding: 6px 12px; font-size: 12px; color: #0F172A; }"
        "QLineEdit:focus { background: #FFFFFF; border: 1px solid #4F46E5; }"
    );
    connect(m_searchEventInput, &QLineEdit::textChanged, this, [this]() {
        updateEventListView();
        if (m_calendarWidget) {
            m_calendarWidget->update();
        }
    });
    layout->addWidget(m_searchEventInput);

    // Today Button
    auto *todayBtn = new QPushButton("Today", header);
    todayBtn->setFixedHeight(36);
    todayBtn->setStyleSheet(
        "QPushButton { background: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; padding: 6px 14px; font-size: 12px; font-weight: 600; color: #334155; }"
        "QPushButton:hover { background: #F8FAFC; border-color: #CBD5E1; }"
    );
    connect(todayBtn, &QPushButton::clicked, [this]() {
        if (m_calendarWidget) {
            m_calendarWidget->setSelectedDate(QDate::currentDate());
            handleDateSelected(QDate::currentDate());
        }
    });
    layout->addWidget(todayBtn);

    // Sync Button
    auto *syncBtn = new QPushButton("🔄 Sync", header);
    syncBtn->setFixedHeight(36);
    syncBtn->setToolTip("Fetch and synchronize Google Calendar events");
    syncBtn->setStyleSheet(
        "QPushButton { background: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; padding: 6px 14px; font-size: 12px; font-weight: 600; color: #334155; }"
        "QPushButton:hover { background: #F8FAFC; border-color: #CBD5E1; }"
    );
    connect(syncBtn, &QPushButton::clicked, this, &ScheduleWidget::handleGoogleSync);
    layout->addWidget(syncBtn);

    return header;
}

QWidget *ScheduleWidget::createDashboardTab() {
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: #F8FAFC; border: none; }");

    auto *container = new QWidget(scrollArea);
    container->setStyleSheet("background-color: #F8FAFC;");
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(20);

    // 1. Hero / Welcome Card
    auto *heroCard = new QFrame(container);
    heroCard->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 12px; padding: 20px; }");
    auto *heroLayout = new QHBoxLayout(heroCard);
    heroLayout->setContentsMargins(20, 16, 20, 16);

    auto *heroLeft = new QVBoxLayout();
    heroLeft->setSpacing(4);

    auto *heroTitle = new QLabel("Welcome to SmartSchedule", heroCard);
    heroTitle->setStyleSheet("font-size: 20px; font-weight: 700; color: #0F172A;");
    m_dashGreeting = new QLabel("Loading your schedule...", heroCard);
    m_dashGreeting->setStyleSheet("font-size: 13px; color: #64748B;");

    heroLeft->addWidget(heroTitle);
    heroLeft->addWidget(m_dashGreeting);
    heroLayout->addLayout(heroLeft, 1);

    auto *quickPlanBtn = new QPushButton("✨ AI Daily Briefing", heroCard);
    quickPlanBtn->setStyleSheet(
        "QPushButton { background-color: #EEF2FF; color: #4F46E5; border: 1px solid #C7D2FE; border-radius: 8px; font-weight: 600; padding: 10px 18px; font-size: 13px; }"
        "QPushButton:hover { background-color: #E0E7FF; }"
    );
    connect(quickPlanBtn, &QPushButton::clicked, [this]() {
        if (m_sidebarList) {
            m_sidebarList->setCurrentRow(3); // Switch to AI tab
        }
        handleGenerateBriefing();
    });
    heroLayout->addWidget(quickPlanBtn);
    layout->addWidget(heroCard);

    // 2. Metric KPI Cards Grid (4 Cards)
    auto *kpiRow = new QHBoxLayout();
    kpiRow->setSpacing(16);

    auto createKpiCard = [](const QString &icon, const QString &label, QLabel *&valLabel, const QString &valColor) -> QFrame* {
        auto *card = new QFrame();
        card->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 12px; padding: 16px; }"
                            "QFrame:hover { border-color: #CBD5E1; }");
        auto *l = new QVBoxLayout(card);
        l->setContentsMargins(16, 14, 16, 14);
        l->setSpacing(6);

        auto *top = new QHBoxLayout();
        auto *iconLbl = new QLabel(icon);
        iconLbl->setStyleSheet("font-size: 18px;");
        auto *titleLbl = new QLabel(label);
        titleLbl->setStyleSheet("font-size: 12px; font-weight: 600; color: #64748B;");
        top->addWidget(iconLbl);
        top->addWidget(titleLbl);
        top->addStretch();
        l->addLayout(top);

        valLabel = new QLabel("0");
        valLabel->setStyleSheet(QString("font-size: 22px; font-weight: 700; color: %1;").arg(valColor));
        l->addWidget(valLabel);

        return card;
    };

    kpiRow->addWidget(createKpiCard("📅", "TOTAL EVENTS", m_dashTotalEvents, "#0F172A"));
    kpiRow->addWidget(createKpiCard("✅", "COMPLETED", m_dashCompletedEvents, "#10B981"));
    kpiRow->addWidget(createKpiCard("⏱️", "FOCUS SESSIONS", m_dashPomoCount, "#4F46E5"));
    kpiRow->addWidget(createKpiCard("💰", "ESTIMATED COST", m_dashTotalCost, "#059669"));

    layout->addLayout(kpiRow);

    // 3. Two-Column Layout: Left (Today's Agenda) & Right (Next Event & Quick Tools)
    auto *contentRow = new QHBoxLayout();
    contentRow->setSpacing(16);

    // Left Column: Today's Agenda (60%)
    auto *agendaCard = new QFrame(container);
    agendaCard->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 12px; padding: 18px; }");
    auto *agendaLayout = new QVBoxLayout(agendaCard);
    agendaLayout->setContentsMargins(18, 16, 18, 16);
    agendaLayout->setSpacing(12);

    auto *agendaHeader = new QHBoxLayout();
    auto *agendaTitle = new QLabel("Today's Agenda", agendaCard);
    agendaTitle->setStyleSheet("font-size: 16px; font-weight: 700; color: #0F172A;");
    agendaHeader->addWidget(agendaTitle);
    agendaHeader->addStretch();

    auto *viewFullCalBtn = new QPushButton("Open Calendar ➔", agendaCard);
    viewFullCalBtn->setStyleSheet("QPushButton { background: transparent; border: none; color: #4F46E5; font-weight: 600; font-size: 12px; }"
                                  "QPushButton:hover { text-decoration: underline; }");
    connect(viewFullCalBtn, &QPushButton::clicked, [this]() {
        if (m_sidebarList) {
            m_sidebarList->setCurrentRow(1);
        }
    });
    agendaHeader->addWidget(viewFullCalBtn);
    agendaLayout->addLayout(agendaHeader);

    m_dashTodayList = new QListWidget(agendaCard);
    m_dashTodayList->setStyleSheet(
        "QListWidget { border: none; background: transparent; }"
        "QListWidget::item { background-color: #F8FAFC; border: 1px solid #E2E8F0; border-radius: 8px; padding: 10px 14px; margin-bottom: 8px; color: #0F172A; font-weight: 500; font-size: 13px; }"
        "QListWidget::item:hover { background-color: #F1F5F9; border-color: #CBD5E1; }"
        "QListWidget::item:selected { background-color: #EEF2FF; border-color: #C7D2FE; color: #4F46E5; }"
    );
    agendaLayout->addWidget(m_dashTodayList);
    contentRow->addWidget(agendaCard, 6);

    // Right Column: Side Quick Widgets (40%)
    auto *rightCol = new QVBoxLayout();
    rightCol->setSpacing(16);

    // Widget 1: Next Event
    auto *nextEventCard = new QFrame(container);
    nextEventCard->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 12px; padding: 16px; }");
    auto *nextLayout = new QVBoxLayout(nextEventCard);
    nextLayout->setContentsMargins(16, 14, 16, 14);
    nextLayout->setSpacing(8);

    auto *nextHeader = new QLabel("⏰ Next Upcoming Event", nextEventCard);
    nextHeader->setStyleSheet("font-size: 12px; font-weight: 700; color: #64748B; text-transform: uppercase;");
    nextLayout->addWidget(nextHeader);

    m_dashNextEvent = new QLabel("Checking upcoming events...", nextEventCard);
    m_dashNextEvent->setWordWrap(true);
    m_dashNextEvent->setStyleSheet("font-size: 14px; color: #0F172A;");
    nextLayout->addWidget(m_dashNextEvent);
    rightCol->addWidget(nextEventCard);

    // Widget 2: Quick Pomodoro Focus Card
    auto *focusCard = new QFrame(container);
    focusCard->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 12px; padding: 16px; }");
    auto *focusLayout = new QVBoxLayout(focusCard);
    focusLayout->setContentsMargins(16, 14, 16, 14);
    focusLayout->setSpacing(10);

    auto *focusHeader = new QLabel("⏱️ Quick Focus Session", focusCard);
    focusHeader->setStyleSheet("font-size: 14px; font-weight: 700; color: #0F172A;");
    auto *focusDesc = new QLabel("Boost productivity with a 25-minute Pomodoro interval.", focusCard);
    focusDesc->setStyleSheet("font-size: 12px; color: #64748B;");
    focusDesc->setWordWrap(true);

    auto *startPomoQuickBtn = new QPushButton("▶️  Start 25m Focus", focusCard);
    startPomoQuickBtn->setStyleSheet(
        "QPushButton { background-color: #4F46E5; color: #FFFFFF; border-radius: 8px; font-weight: 600; padding: 10px; font-size: 13px; border: none; }"
        "QPushButton:hover { background-color: #4338CA; }"
    );
    connect(startPomoQuickBtn, &QPushButton::clicked, [this]() {
        if (m_sidebarList) {
            m_sidebarList->setCurrentRow(2); // Focus tab
        }
        m_calendarManager->startPomodoro(25, 5);
    });

    focusLayout->addWidget(focusHeader);
    focusLayout->addWidget(focusDesc);
    focusLayout->addWidget(startPomoQuickBtn);
    rightCol->addWidget(focusCard);

    contentRow->addLayout(rightCol, 4);
    layout->addLayout(contentRow);

    layout->addStretch();

    scrollArea->setWidget(container);
    return scrollArea;
}

void ScheduleWidget::refreshDashboard() {
    if (!m_dashTotalEvents) return;

    int total = static_cast<int>(m_calendarManager->m_localEvents.size());
    int completed = 0;
    double cost = 0.0;

    for (const auto &e : m_calendarManager->m_localEvents) {
        cost += e.cost;
        if (e.isCompleted) completed++;
    }

    QDate today = QDate::currentDate();
    auto todayEvents = m_calendarManager->getEventsForDate(today);

    m_dashTotalEvents->setText(QString::number(total));
    m_dashCompletedEvents->setText(QString::number(completed));
    m_dashPomoCount->setText(QString::number(m_calendarManager->getCompletedPomodoros()));
    m_dashTotalCost->setText(QString("%1 VNĐ").arg(QLocale(QLocale::Vietnamese).toString(static_cast<qint64>(cost))));

    if (m_dashGreeting) {
        m_dashGreeting->setText(QString("You have %1 events scheduled for today (%2).")
            .arg(todayEvents.size())
            .arg(today.toString("dddd, MMMM d")));
    }

    if (m_dashTodayList) {
        m_dashTodayList->clear();
        if (todayEvents.isEmpty()) {
            auto *emptyItem = new QListWidgetItem("🎉 No events scheduled for today! Enjoy your free time.", m_dashTodayList);
            emptyItem->setForeground(QColor("#94A3B8"));
        } else {
            std::sort(todayEvents.begin(), todayEvents.end(), [](const ScheduleEvent &a, const ScheduleEvent &b) {
                return a.startTime < b.startTime;
            });
            for (const auto &e : todayEvents) {
                QString itemText = QString("%1  %2 - %3 | %4 %5")
                    .arg(e.isCompleted ? "✅" : "⏳")
                    .arg(e.startTime.toString("HH:mm"))
                    .arg(e.endTime.toString("HH:mm"))
                    .arg(e.title)
                    .arg(e.location.isEmpty() ? "" : "📍 " + e.location);
                auto *item = new QListWidgetItem(itemText, m_dashTodayList);
                item->setData(Qt::UserRole, e.id);
                item->setSizeHint(QSize(200, 36));
                m_dashTodayList->addItem(item);
            }
        }
    }

    if (m_dashNextEvent) {
        QDateTime now = QDateTime::currentDateTime();
        ScheduleEvent nextEv;
        bool found = false;
        for (const auto &e : todayEvents) {
            if (e.endTime > now && !e.isCompleted) {
                nextEv = e;
                found = true;
                break;
            }
        }
        if (found) {
            m_dashNextEvent->setText(QString("<b>%1</b><br/><span style='color: #4F46E5;'>%2 - %3</span> %4")
                .arg(nextEv.title.toHtmlEscaped())
                .arg(nextEv.startTime.toString("HH:mm"))
                .arg(nextEv.endTime.toString("HH:mm"))
                .arg(nextEv.location.isEmpty() ? "" : "| 📍 " + nextEv.location.toHtmlEscaped()));
        } else {
            m_dashNextEvent->setText("<span style='color: #94A3B8;'>No more pending events today</span>");
        }
    }
}

void ScheduleWidget::setupUI() {
    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // 1. Sidebar on Left
    rootLayout->addWidget(createSidebar());

    // 2. Right Content Area (Header + Main Pages Stack)
    auto *mainContainer = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createHeaderToolbar());

    m_mainTabs = new QTabWidget(this);
    m_mainTabs->tabBar()->hide(); // Controlled via Modern Sidebar
    m_mainTabs->setStyleSheet("QTabWidget::pane { border: none; background: #F8FAFC; margin: 0; padding: 0; }");

    m_mainTabs->addTab(createDashboardTab(), "Dashboard");
    m_mainTabs->addTab(createScheduleTab(), "Schedule");
    m_mainTabs->addTab(createPomodoroTab(), "Focus");
    m_mainTabs->addTab(createAiTab(), "AI Assistant");
    m_mainTabs->addTab(createStatsTab(), "Analytics");
    m_mainTabs->addTab(createSettingsTab(), "Settings");
    m_mainTabs->addTab(createLogsTab(), "Logs");

    mainLayout->addWidget(m_mainTabs);
    rootLayout->addWidget(mainContainer, 1);

    if (m_sidebarList) {
        m_sidebarList->setCurrentRow(0);
    }
}

QWidget *ScheduleWidget::createScheduleTab() {
    auto *container = new QWidget(this);
    auto *mainSplitter = new QSplitter(Qt::Horizontal, container);

    // --- BÊN TRÁI: Calendar Widget & Form thêm sự kiện ---
    auto *leftWidget = new QWidget(mainSplitter);
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 10, 0);

    m_calendarWidget = new DragDropCalendarWidget(m_calendarManager, this);
    m_calendarWidget->setGridVisible(true);
    connect(m_calendarWidget, &QCalendarWidget::clicked, this, &ScheduleWidget::handleDateSelected);
    leftLayout->addWidget(m_calendarWidget);

    auto *formGroup = new QGroupBox("➕ Thêm sự kiện mới", this);
    auto *formLayout = new QVBoxLayout(formGroup);

    m_titleInput = new QLineEdit(this);
    m_titleInput->setPlaceholderText("Tiêu đề sự kiện (VD: Họp dự án)...");
    m_descInput = new QLineEdit(this);
    m_descInput->setPlaceholderText("Ghi chú chi tiết...");
    m_locInput = new QLineEdit(this);
    m_locInput->setPlaceholderText("Địa điểm (VD: Phòng 402 hoặc Google Meet)...");

    auto *timeRow = new QHBoxLayout();
    m_startDateTime = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    m_startDateTime->setDisplayFormat("dd/MM/yyyy HH:mm");
    m_endDateTime = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(3600), this);
    m_endDateTime->setDisplayFormat("dd/MM/yyyy HH:mm");
    timeRow->addWidget(new QLabel("Bắt đầu:", this));
    timeRow->addWidget(m_startDateTime);
    timeRow->addWidget(new QLabel("Kết thúc:", this));
    timeRow->addWidget(m_endDateTime);

    auto *metaRow = new QHBoxLayout();
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItems({"Work", "Study", "Personal", "Health", "Other"});
    m_priorityCombo = new QComboBox(this);
    m_priorityCombo->addItems({"Low", "Medium", "High"});
    m_costSpinBox = new QDoubleSpinBox(this);
    m_costSpinBox->setMaximum(1000000000);
    m_costSpinBox->setSuffix(" VNĐ");

    metaRow->addWidget(new QLabel("Loại:", this));
    metaRow->addWidget(m_categoryCombo);
    metaRow->addWidget(new QLabel("Ưu tiên:", this));
    metaRow->addWidget(m_priorityCombo);
    metaRow->addWidget(new QLabel("Chi phí:", this));
    metaRow->addWidget(m_costSpinBox);

    m_conflictWarningLabel = new QLabel("", this);
    m_conflictWarningLabel->setStyleSheet("color: #dc2626; font-size: 11px;");

    auto *addBtn = new QPushButton("✅ Thêm sự kiện", this);
    addBtn->setStyleSheet(
        "background-color: #2563eb; color: white; padding: 8px; font-weight: bold; border-radius: "
        "6px;");
    connect(addBtn, &QPushButton::clicked, this, &ScheduleWidget::handleAddEvent);

    formLayout->addWidget(m_titleInput);
    formLayout->addWidget(m_descInput);
    formLayout->addWidget(m_locInput);
    formLayout->addLayout(timeRow);
    formLayout->addLayout(metaRow);
    formLayout->addWidget(m_conflictWarningLabel);
    formLayout->addWidget(addBtn);

    leftLayout->addWidget(formGroup);

    // --- BÊN PHẢI: Danh sách sự kiện & Tìm kiếm ---
    auto *rightWidget = new QWidget(mainSplitter);
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(10, 0, 0, 0);

    auto *filterRow = new QHBoxLayout();

    m_filterCategoryCombo = new QComboBox(this);
    m_filterCategoryCombo->addItems(
        {"Tất cả thể loại", "Work", "Study", "Personal", "Health", "Other"});
    connect(m_filterCategoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &ScheduleWidget::updateEventListView);

    m_sortCombo = new QComboBox(this);
    m_sortCombo->addItems({"⏰ Sắp xếp: Thời gian", "🔥 Sắp xếp: Độ ưu tiên"});
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &ScheduleWidget::updateEventListView);

    filterRow->addWidget(m_filterCategoryCombo);
    filterRow->addWidget(m_sortCombo);
    rightLayout->addLayout(filterRow);

    auto *aiActionRow = new QHBoxLayout();
    m_aiSummaryBtn = new QPushButton("✨ Tóm tắt lịch hôm nay bằng AI", this);
    m_aiSummaryBtn->setStyleSheet(
        "background-color: #8b5cf6; color: white; padding: 6px; font-weight: bold; border-radius: "
        "4px;");
    connect(m_aiSummaryBtn, &QPushButton::clicked, this,
            &ScheduleWidget::handleGenerateDailySummary);

    m_exportWeeklyPdfBtn = new QPushButton("🖨️ Xuất PDF Lịch tuần", this);
    m_exportWeeklyPdfBtn->setStyleSheet(
        "background-color: #ef4444; color: white; padding: 6px; font-weight: bold; border-radius: "
        "4px;");
    connect(m_exportWeeklyPdfBtn, &QPushButton::clicked, this,
            &ScheduleWidget::handleExportWeeklyPdf);

    aiActionRow->addWidget(m_aiSummaryBtn);
    aiActionRow->addWidget(m_exportWeeklyPdfBtn);

    m_aiSummaryDisplay = new QTextEdit(this);
    m_aiSummaryDisplay->setReadOnly(true);
    m_aiSummaryDisplay->setMaximumHeight(120);
    m_aiSummaryDisplay->setStyleSheet(
        "background-color: #f8fafc; border: 1px solid #cbd5e1; border-radius: 4px; padding: 4px;");
    m_aiSummaryDisplay->setPlaceholderText(
        "Bấm 'Tóm tắt lịch hôm nay bằng AI' để Gemini tổng hợp lịch trình của bạn...");

    rightLayout->addLayout(aiActionRow);
    rightLayout->addWidget(m_aiSummaryDisplay);

    m_eventListView = new QListWidget(this);
    m_eventListView->setDragEnabled(true);
    m_eventListView->setDragDropMode(QAbstractItemView::DragOnly);

    auto *dragCalendar = static_cast<DragDropCalendarWidget *>(m_calendarWidget);
    dragCalendar->setEventList(m_eventListView);
    dragCalendar->setSearchInput(m_searchEventInput);
    dragCalendar->onEventDropped = [this](const QString &eventId, const QDate &newDate) {
        for (const auto &ev : m_calendarManager->m_localEvents) {
            if (ev.id == eventId) {
                ScheduleEvent updatedEv = ev;
                qint64 duration = updatedEv.startTime.secsTo(updatedEv.endTime);
                QTime startTime = updatedEv.startTime.time();
                updatedEv.startTime = QDateTime(newDate, startTime);
                updatedEv.endTime = updatedEv.startTime.addSecs(duration);
                m_calendarManager->updateEvent(updatedEv.id, updatedEv);

                customLogHandler(QtInfoMsg, QMessageLogContext(),
                                 QString("Đã di chuyển '%1' sang ngày %2")
                                     .arg(updatedEv.title, newDate.toString("dd/MM/yyyy")));
                break;
            }
        }
        refreshCalendarGrid();
    };

    rightLayout->addWidget(m_eventListView);

    auto *actionRow = new QHBoxLayout();
    auto *deleteBtn = new QPushButton("🗑️ Xóa sự kiện chọn", this);
    deleteBtn->setStyleSheet(
        "background-color: #ef4444; color: white; padding: 6px; border-radius: 4px;");
    connect(deleteBtn, &QPushButton::clicked, [this]() {
        QListWidgetItem *item = m_eventListView->currentItem();
        if (item) {
            QString id = item->data(Qt::UserRole).toString();
            m_calendarManager->deleteEvent(id);
        } else {
            QMessageBox::information(this, "Thông báo", "Vui lòng chọn 1 sự kiện để xóa.");
        }
    });

    auto *pushGoogleBtn = new QPushButton("☁️ Đẩy lên Google Calendar", this);
    connect(pushGoogleBtn, &QPushButton::clicked, [this]() {
        QListWidgetItem *item = m_eventListView->currentItem();
        if (item) {
            QString id = item->data(Qt::UserRole).toString();
            for (const auto &e : m_calendarManager->m_localEvents) {
                if (e.id == id) {
                    m_calendarManager->syncEventToGoogle(e);
                    return;
                }
            }
        } else {
            QMessageBox::information(this, "Thông báo", "Vui lòng chọn 1 sự kiện để đồng bộ.");
        }
    });

    m_undoBtn = new QPushButton("↩️ Hoàn tác", this);
    m_undoBtn->setStyleSheet(
        "background-color: #f59e0b; color: white; padding: 6px; border-radius: 4px;");
    m_undoBtn->setEnabled(false);  // Sẽ bật khi có sự kiện có thể hoàn tác
    m_undoBtn->setToolTip("Hoàn tác thao tác xóa hoặc kéo thả vừa rồi (Ctrl+Z)");
    connect(m_undoBtn, &QPushButton::clicked, this, &ScheduleWidget::handleUndo);

    m_undoShortcut = new QShortcut(QKeySequence("Ctrl+Z"), this);
    connect(m_undoShortcut, &QShortcut::activated, this, &ScheduleWidget::handleUndo);

    auto *completeBtn = new QPushButton("✅ Hoàn thành", this);
    completeBtn->setStyleSheet(
        "background-color: #10b981; color: white; padding: 6px; border-radius: 4px;");
    connect(completeBtn, &QPushButton::clicked, [this]() {
        QListWidgetItem *item = m_eventListView->currentItem();
        if (item) {
            QString id = item->data(Qt::UserRole).toString();
            for (const auto &e : m_calendarManager->m_localEvents) {
                if (e.id == id) {
                    ScheduleEvent updated = e;
                    updated.isCompleted = !updated.isCompleted;
                    m_calendarManager->updateEvent(id, updated);
                    return;
                }
            }
        } else {
            QMessageBox::information(this, "Thông báo", "Vui lòng chọn 1 sự kiện để đánh dấu.");
        }
    });

    actionRow->addWidget(completeBtn);
    actionRow->addWidget(deleteBtn);
    actionRow->addWidget(m_undoBtn);
    actionRow->addWidget(pushGoogleBtn);
    rightLayout->addLayout(actionRow);

    mainSplitter->addWidget(leftWidget);
    mainSplitter->addWidget(rightWidget);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 1);

    auto *tabLayout = new QVBoxLayout(container);
    tabLayout->addWidget(mainSplitter);
    return container;
}

QWidget *ScheduleWidget::createAiTab() {
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);

    // 1. Phân tích ngôn ngữ tự nhiên thông minh
    auto *smartBox =
        new QGroupBox("✨ Tạo lịch trình tự động bằng AI (Nhập câu nói tự nhiên)", this);
    auto *smartLayout = new QVBoxLayout(smartBox);

    auto *inputRow = new QHBoxLayout();
    m_aiSmartInput = new QLineEdit(this);
    m_aiSmartInput->setPlaceholderText(
        "VD: Sáng mai từ 9h đến 11h họp team công ty tại phòng 301, sau đó 12h ăn trưa với Lan ở "
        "Bitexco chi phí 120k");
    m_aiSmartAddBtn = new QPushButton("🪄 Phân tích & Trích xuất", this);
    m_aiSmartAddBtn->setStyleSheet(
        "background-color: #8b5cf6; color: white; font-weight: bold; padding: 6px 12px; "
        "border-radius: 6px;");
    connect(m_aiSmartAddBtn, &QPushButton::clicked, this, &ScheduleWidget::handleSmartAiAdd);

    inputRow->addWidget(m_aiSmartInput);
    inputRow->addWidget(m_aiSmartAddBtn);
    smartLayout->addLayout(inputRow);

    m_aiParsedEventsList = new QListWidget(this);
    m_aiParsedEventsList->setMaximumHeight(130);
    smartLayout->addWidget(new QLabel("Các sự kiện AI nhận diện được:", this));
    smartLayout->addWidget(m_aiParsedEventsList);

    m_aiConfirmAddBtn = new QPushButton("📥 Thêm tất cả sự kiện này vào Lịch", this);
    m_aiConfirmAddBtn->setEnabled(false);
    m_aiConfirmAddBtn->setStyleSheet(
        "background-color: #10b981; color: white; font-weight: bold; padding: 8px; border-radius: "
        "6px;");
    connect(m_aiConfirmAddBtn, &QPushButton::clicked, [this]() {
        for (const auto &ev : m_pendingAiEvents) {
            m_calendarManager->addLocalEvent(ev);
        }
        m_pendingAiEvents.clear();
        m_aiParsedEventsList->clear();
        m_aiConfirmAddBtn->setEnabled(false);
        QMessageBox::information(this, "Thành công", "Đã thêm toàn bộ sự kiện từ AI vào Lịch!");
    });
    smartLayout->addWidget(m_aiConfirmAddBtn);
    layout->addWidget(smartBox);

    // 2. Executive Daily Briefing & Optimizer
    auto *briefingBox = new QGroupBox("📋 Báo cáo tổng kết ngày & Tối ưu năng suất", this);
    auto *briefingLayout = new QVBoxLayout(briefingBox);

    auto *btnRow = new QHBoxLayout();
    auto *genBriefingBtn = new QPushButton("🎯 Tạo Tóm Tắt Ngày (Daily Briefing)", this);
    genBriefingBtn->setStyleSheet(
        "background-color: #0284c7; color: white; padding: 8px; border-radius: 6px; font-weight: "
        "bold;");
    connect(genBriefingBtn, &QPushButton::clicked, this, &ScheduleWidget::handleGenerateBriefing);

    auto *optBtn = new QPushButton("⚡ Gợi ý tối ưu hóa thời gian trống", this);
    optBtn->setStyleSheet(
        "background-color: #d97706; color: white; padding: 8px; border-radius: 6px; font-weight: "
        "bold;");
    connect(optBtn, &QPushButton::clicked, this, &ScheduleWidget::handleOptimizeSchedule);

    btnRow->addWidget(genBriefingBtn);
    btnRow->addWidget(optBtn);
    briefingLayout->addLayout(btnRow);

    m_aiBriefingDisplay = new QTextEdit(this);
    m_aiBriefingDisplay->setReadOnly(true);
    m_aiBriefingDisplay->setPlaceholderText(
        "Bản báo cáo tóm tắt lịch trình từ Gemini AI sẽ hiển thị tại đây...");
    briefingLayout->addWidget(m_aiBriefingDisplay);
    layout->addWidget(briefingBox);

    // 3. Trò chuyện tự do với AI
    auto *chatBox = new QGroupBox("💬 Chat tư vấn lịch trình cùng Gemini", this);
    auto *chatLayout = new QVBoxLayout(chatBox);

    m_aiChatHistory = new QTextEdit(this);
    m_aiChatHistory->setReadOnly(true);
    m_aiChatHistory->setMaximumHeight(120);
    chatLayout->addWidget(m_aiChatHistory);

    auto *chatInputRow = new QHBoxLayout();
    m_aiChatInput = new QLineEdit(this);
    m_aiChatInput->setPlaceholderText("Hỏi AI bất kỳ điều gì về sắp xếp thời gian...");
    auto *sendChatBtn = new QPushButton("Gửi", this);
    connect(sendChatBtn, &QPushButton::clicked, this, &ScheduleWidget::handleSendAiChat);
    connect(m_aiChatInput, &QLineEdit::returnPressed, this, &ScheduleWidget::handleSendAiChat);

    chatInputRow->addWidget(m_aiChatInput);
    chatInputRow->addWidget(sendChatBtn);
    chatLayout->addLayout(chatInputRow);
    layout->addWidget(chatBox);

    return container;
}

QWidget *ScheduleWidget::createPomodoroTab() {
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    auto *card = new QFrame(container);
    card->setStyleSheet(
        "background-color: white; border-radius: 12px; border: 1px solid #e2e8f0; padding: 30px;");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setAlignment(Qt::AlignCenter);

    m_pomoStateLabel = new QLabel("🎯 Sẵn sàng tập trung (25 phút)", card);
    m_pomoStateLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #475569;");
    cardLayout->addWidget(m_pomoStateLabel, 0, Qt::AlignCenter);

    m_pomoTimerLabel = new QLabel("25:00", card);
    m_pomoTimerLabel->setStyleSheet(
        "font-size: 64px; font-weight: 900; color: #1e293b; margin: 20px 0;");
    cardLayout->addWidget(m_pomoTimerLabel, 0, Qt::AlignCenter);

    m_pomoProgressBar = new QProgressBar(card);
    m_pomoProgressBar->setRange(0, 100);
    m_pomoProgressBar->setValue(0);
    m_pomoProgressBar->setFixedHeight(8);
    m_pomoProgressBar->setTextVisible(false);
    m_pomoProgressBar->setStyleSheet(
        "QProgressBar { background: #f1f5f9; border-radius: 4px; } QProgressBar::chunk { "
        "background: #2563eb; border-radius: 4px; }");
    cardLayout->addWidget(m_pomoProgressBar);

    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(12);

    m_startPomoBtn = new QPushButton("▶️ Bắt đầu", card);
    m_startPomoBtn->setStyleSheet(
        "background-color: #2563eb; color: white; font-weight: bold; padding: 10px 24px; "
        "border-radius: 8px; font-size: 14px;");
    connect(m_startPomoBtn, &QPushButton::clicked,
            [this]() { m_calendarManager->startPomodoro(25, 5); });

    m_pausePomoBtn = new QPushButton("⏸️ Tạm dừng", card);
    m_pausePomoBtn->setStyleSheet(
        "background-color: #f59e0b; color: white; font-weight: bold; padding: 10px 24px; "
        "border-radius: 8px; font-size: 14px;");
    connect(m_pausePomoBtn, &QPushButton::clicked, m_calendarManager,
            &CalendarManager::pausePomodoro);

    m_resetPomoBtn = new QPushButton("🔄 Đặt lại", card);
    m_resetPomoBtn->setStyleSheet(
        "background-color: #64748b; color: white; font-weight: bold; padding: 10px 24px; "
        "border-radius: 8px; font-size: 14px;");
    connect(m_resetPomoBtn, &QPushButton::clicked, m_calendarManager,
            &CalendarManager::resetPomodoro);

    btnRow->addWidget(m_startPomoBtn);
    btnRow->addWidget(m_pausePomoBtn);
    btnRow->addWidget(m_resetPomoBtn);
    cardLayout->addLayout(btnRow);

    m_pomoCountLabel = new QLabel("Số hiệp hoàn thành: 0", card);
    m_pomoCountLabel->setStyleSheet("color: #64748b; font-size: 13px; margin-top: 15px;");
    cardLayout->addWidget(m_pomoCountLabel, 0, Qt::AlignCenter);

    layout->addWidget(card);
    return container;
}

QWidget *ScheduleWidget::createStatsTab() {
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);

    auto *summaryGroup = new QGroupBox("📈 Tổng quan chi phí & Sự kiện", this);
    auto *sumLayout = new QVBoxLayout(summaryGroup);

    m_statTotalEvents = new QLabel("Tổng số sự kiện: 0", this);
    m_statTotalEvents->setStyleSheet("font-size: 15px; font-weight: bold; color: #1e293b;");
    m_statTotalCost = new QLabel("Tổng chi phí ước tính: 0 VNĐ", this);
    m_statTotalCost->setStyleSheet("font-size: 15px; font-weight: bold; color: #059669;");

    sumLayout->addWidget(m_statTotalEvents);
    sumLayout->addWidget(m_statTotalCost);

    auto *exportCsvBtn = new QPushButton("📥 Xuất dữ liệu ra file CSV", this);
    exportCsvBtn->setStyleSheet(
        "background-color: #059669; color: white; padding: 8px; border-radius: 6px; font-weight: "
        "bold; margin-top: 8px;");
    connect(exportCsvBtn, &QPushButton::clicked, this, &ScheduleWidget::handleExportCsv);
    sumLayout->addWidget(exportCsvBtn);

    layout->addWidget(summaryGroup);

    auto *categoryGroup = new QGroupBox("🏷️ Phân bổ theo danh mục", this);
    auto *catLayout = new QVBoxLayout(categoryGroup);

    m_statWorkCount = new QLabel("💼 Công việc (Work): 0 sự kiện", this);
    m_statWorkProgress = new QProgressBar(this);
    catLayout->addWidget(m_statWorkCount);
    catLayout->addWidget(m_statWorkProgress);

    m_statStudyCount = new QLabel("📚 Học tập (Study): 0 sự kiện", this);
    m_statStudyProgress = new QProgressBar(this);
    catLayout->addWidget(m_statStudyCount);
    catLayout->addWidget(m_statStudyProgress);

    m_statPersonalCount = new QLabel("👤 Cá nhân (Personal): 0 sự kiện", this);
    m_statPersonalProgress = new QProgressBar(this);
    catLayout->addWidget(m_statPersonalCount);
    catLayout->addWidget(m_statPersonalProgress);

    m_statHealthCount = new QLabel("🏃 Sức khỏe (Health): 0 sự kiện", this);
    m_statHealthProgress = new QProgressBar(this);
    catLayout->addWidget(m_statHealthCount);
    catLayout->addWidget(m_statHealthProgress);

    layout->addWidget(categoryGroup);

    auto *chartGroup = new QGroupBox("📊 Biểu đồ hoàn thành (Tuần này)", this);
    auto *chartLayout = new QVBoxLayout(chartGroup);
    m_completionChart = new CompletionChartWidget(m_calendarManager, this);
    chartLayout->addWidget(m_completionChart);
    layout->addWidget(chartGroup);

    layout->addStretch();
    return container;
}

QWidget *ScheduleWidget::createSettingsTab() {
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);

    // Google Calendar
    auto *googleBox = new QGroupBox("🌐 Kết nối Google Calendar (OAuth 2.0)", this);
    auto *googleLayout = new QVBoxLayout(googleBox);

    m_googleClientIdInput = new QLineEdit(this);
    m_googleClientIdInput->setPlaceholderText(
        "Google Client ID (VD: xxxxx.apps.googleusercontent.com)...");
    m_googleClientSecretInput = new QLineEdit(this);
    m_googleClientSecretInput->setPlaceholderText("Google Client Secret...");
    m_googleClientSecretInput->setEchoMode(QLineEdit::Password);

    m_googleStatusLabel = new QLabel("⚪ Google Calendar: Chưa kết nối", this);

    auto *btnRow = new QHBoxLayout();
    m_googleLoginBtn = new QPushButton("🔑 Đăng nhập tài khoản Google", this);
    m_googleLoginBtn->setStyleSheet(
        "background-color: #ea4335; color: white; font-weight: bold; padding: 8px; border-radius: "
        "6px;");
    connect(m_googleLoginBtn, &QPushButton::clicked, [this]() {
        QString cId = m_googleClientIdInput->text().trimmed();
        QString cSecret = m_googleClientSecretInput->text().trimmed();
        if (cId.isEmpty() || cSecret.isEmpty()) {
            QMessageBox::warning(
                this, "Thiếu thông tin",
                "Vui lòng nhập Client ID và Client Secret từ Google Cloud Console.");
            return;
        }

        QSettings settings;
        settings.setValue("google_client_id", cId);
        settings.setValue("google_client_secret", cSecret);

        m_googleStatusLabel->setText("⏳ Đang kết nối Google...");
        m_calendarManager->setupGoogleOAuth(cId, cSecret);
        m_calendarManager->authenticateGoogle();
    });

    m_googleSyncBtn = new QPushButton("🔄 Đồng bộ 2 chiều ngay", this);
    m_googleSyncBtn->setEnabled(false);
    connect(m_googleSyncBtn, &QPushButton::clicked, this, &ScheduleWidget::handleGoogleSync);

    btnRow->addWidget(m_googleLoginBtn);
    btnRow->addWidget(m_googleSyncBtn);

    googleLayout->addWidget(new QLabel("Client ID:", this));
    googleLayout->addWidget(m_googleClientIdInput);
    googleLayout->addWidget(new QLabel("Client Secret:", this));
    googleLayout->addWidget(m_googleClientSecretInput);
    googleLayout->addWidget(m_googleStatusLabel);
    googleLayout->addLayout(btnRow);
    layout->addWidget(googleBox);

    // Gemini API
    auto *geminiBox = new QGroupBox("🧠 Cài đặt Gemini AI API Key", this);
    auto *geminiLayout = new QVBoxLayout(geminiBox);

    m_geminiApiKeyInput = new QLineEdit(this);
    m_geminiApiKeyInput->setPlaceholderText("Nhập Gemini API Key...");
    m_geminiApiKeyInput->setText(m_geminiService->apiKey());

    auto *saveKeyBtn = new QPushButton("💾 Lưu API Key", this);
    connect(saveKeyBtn, &QPushButton::clicked, this, &ScheduleWidget::handleSaveSettings);

    geminiLayout->addWidget(m_geminiApiKeyInput);
    geminiLayout->addWidget(saveKeyBtn);
    layout->addWidget(geminiBox);

    // Thêm System Settings (Resource & Storage Monitor)
    auto *systemSettings = new SettingsWidget(m_calendarManager, this);
    layout->addWidget(systemSettings);

    layout->addStretch();
    return container;
}

void ScheduleWidget::handleAddEvent() {
    QString title = m_titleInput->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "Thiếu dữ liệu", "Vui lòng nhập tiêu đề sự kiện.");
        return;
    }

    ScheduleEvent event;
    event.title = title;
    event.description = m_descInput->text().trimmed();
    event.location = m_locInput->text().trimmed();
    event.startTime = m_startDateTime->dateTime();
    event.endTime = m_endDateTime->dateTime();
    event.category = static_cast<Category>(m_categoryCombo->currentIndex());
    event.priority = static_cast<Priority>(m_priorityCombo->currentIndex());
    event.cost = m_costSpinBox->value();

    if (event.endTime <= event.startTime) {
        QMessageBox::warning(this, "Thời gian không hợp lệ",
                             "Thời gian kết thúc phải sau thời gian bắt đầu.");
        return;
    }

    auto conflicts = m_calendarManager->findConflicts(event);
    if (!conflicts.isEmpty()) {
        QString conflictNames;
        for (const auto &c : conflicts) conflictNames += " - " + c.title + "\n";
        auto res = QMessageBox::question(
            this, "Trùng lịch!",
            QString("Sự kiện này trùng giờ với:\n%1\nBạn có vẫn muốn thêm không?")
                .arg(conflictNames),
            QMessageBox::Yes | QMessageBox::No);
        if (res != QMessageBox::Yes)
            return;
    }

    m_calendarManager->addLocalEvent(event);

    m_titleInput->clear();
    m_descInput->clear();
    m_locInput->clear();
    m_costSpinBox->setValue(0);
}

void ScheduleWidget::handleSmartAiAdd() {
    QString text = m_aiSmartInput->text().trimmed();
    if (text.isEmpty())
        return;

    m_aiSmartAddBtn->setEnabled(false);
    m_aiSmartAddBtn->setText("⏳ Đang phân tích...");

    m_geminiService->parseNaturalLanguageSchedule(
        text, QDateTime::currentDateTime(),
        [this](bool success, QList<ScheduleEvent> events, QString explanation) {
            m_aiSmartAddBtn->setEnabled(true);
            m_aiSmartAddBtn->setText("🪄 Phân tích & Trích xuất");

            if (success) {
                m_pendingAiEvents = events;
                m_aiParsedEventsList->clear();
                for (const auto &ev : events) {
                    QString itemText = QString("🔹 %1 (%2 -> %3) | %4 | Chi phí: %5 VNĐ")
                                           .arg(ev.title)
                                           .arg(ev.startTime.toString("dd/MM HH:mm"))
                                           .arg(ev.endTime.toString("HH:mm"))
                                           .arg(CalendarManager::categoryToString(ev.category))
                                           .arg(ev.cost);
                    m_aiParsedEventsList->addItem(itemText);
                }
                m_aiConfirmAddBtn->setEnabled(!events.isEmpty());
            } else {
                QMessageBox::warning(this, "Lỗi phân tích AI", explanation);
            }
        });
}

void ScheduleWidget::handleGenerateBriefing() {
    QDate target = m_calendarWidget->selectedDate();
    auto events = m_calendarManager->getEventsForDate(target);

    m_aiBriefingDisplay->setHtml("<i>Gemini đang tổng hợp báo cáo điều hành...</i>");

    m_geminiService->generateDailyBriefing(events, target, [this](bool success, QString result) {
        if (success) {
            m_aiBriefingDisplay->setPlainText(result);
        } else {
            m_aiBriefingDisplay->setPlainText("Không thể tạo báo cáo: " + result);
        }
    });
}

void ScheduleWidget::handleOptimizeSchedule() {
    QDate target = m_calendarWidget->selectedDate();
    auto events = m_calendarManager->getEventsForDate(target);

    m_aiBriefingDisplay->setHtml("<i>Gemini đang tính toán các khung giờ tối ưu...</i>");

    m_geminiService->optimizeSchedule(events, target, [this](bool success, QString result) {
        if (success) {
            m_aiBriefingDisplay->setPlainText(result);
        } else {
            m_aiBriefingDisplay->setPlainText("Không thể tối ưu: " + result);
        }
    });
}

void ScheduleWidget::handleSendAiChat() {
    QString msg = m_aiChatInput->text().trimmed();
    if (msg.isEmpty())
        return;

    m_aiChatHistory->append("<b>Bạn:</b> " + msg);
    m_aiChatInput->clear();

    m_geminiService->askGemini(msg, [this](bool success, QString resp) {
        if (success) {
            m_aiChatHistory->append("<b>Gemini:</b> " + resp + "\n");
        } else {
            m_aiChatHistory->append("<b>Gemini:</b> <i>Lỗi: " + resp + "</i>\n");
        }
    });
}

void ScheduleWidget::updateEventListView() {
    m_eventListView->clear();
    QString search = m_searchEventInput ? m_searchEventInput->text().toLower() : "";
    int filterCat = m_filterCategoryCombo ? m_filterCategoryCombo->currentIndex() : 0;
    int sortMode = m_sortCombo ? m_sortCombo->currentIndex() : 0;
    
    QDate selectedDate = m_calendarWidget ? m_calendarWidget->selectedDate() : QDate::currentDate();
    QDateTime dayStart(selectedDate, QTime(0, 0));
    QDateTime dayEnd = dayStart.addDays(1);

    QList<ScheduleEvent> displayEvents;
    for (const auto &e : m_calendarManager->m_localEvents) {
        // Lọc theo ngày được chọn trên lịch (overlap 24h)
        if (!(e.startTime < dayEnd && e.endTime > dayStart)) {
            continue;
        }

        if (!search.isEmpty()) {
            QString catStr = CalendarManager::categoryToString(e.category).toLower();
            if (!e.title.toLower().contains(search) && !e.description.toLower().contains(search) &&
                !e.location.toLower().contains(search) && !catStr.contains(search)) {
                continue;
            }
        }
        if (filterCat > 0) {
            Category expected = static_cast<Category>(filterCat - 1);
            if (e.category != expected)
                continue;
        }
        displayEvents.append(e);
    }

    std::sort(displayEvents.begin(), displayEvents.end(),
              [sortMode](const ScheduleEvent &a, const ScheduleEvent &b) {
                  if (sortMode == 1) {
                      // Sắp xếp theo Độ ưu tiên (High > Medium > Low)
                      if (a.priority != b.priority) {
                          return static_cast<int>(a.priority) > static_cast<int>(b.priority);
                      }
                  }
                  // Mặc định hoặc trùng ưu tiên: Sắp xếp theo Thời gian
                  return a.startTime < b.startTime;
              });

    for (const auto &e : displayEvents) {
        QString priorityStr = "Trung bình";
        if (e.priority == Priority::High)
            priorityStr = "🔥 Cao";
        else if (e.priority == Priority::Low)
            priorityStr = "🔽 Thấp";

        QString display = QString("%1📌 [%2] %3 (%4)\n🕒 %5 ➔ %6 | 💰 %7 VNĐ %8\n📍 %9")
                              .arg(e.isCompleted ? "✅ " : "")
                              .arg(CalendarManager::categoryToString(e.category))
                              .arg(e.title)
                              .arg(priorityStr)
                              .arg(e.startTime.toString("dd/MM HH:mm"))
                              .arg(e.endTime.toString("HH:mm"))
                              .arg(e.cost)
                              .arg(e.isSyncedGoogle ? " | 🟢 Google Synced" : "")
                              .arg(e.location.isEmpty() ? "Không có địa điểm" : e.location);

        auto *item = new QListWidgetItem(display);
        item->setData(Qt::UserRole, e.id);  // Lưu ID để các thao tác xóa, đồng bộ lấy chính xác ID

        // Color-coded categories
        QColor bgColor = QColor("#f3f4f6");  // Default/Other (Gray)
        if (e.isCompleted)
            bgColor = QColor("#dcfce7");  // Green if completed
        else if (e.category == Category::Study)
            bgColor = QColor("#e0e7ff");  // Indigo
        else if (e.category == Category::Work)
            bgColor = QColor("#dbeafe");  // Blue
        else if (e.category == Category::Personal)
            bgColor = QColor("#fce7f3");  // Pink
        else if (e.category == Category::Health)
            bgColor = QColor("#d1fae5");  // Emerald

        item->setBackground(bgColor);
        m_eventListView->addItem(item);
    }
}

void ScheduleWidget::updateStatistics() {
    double totalCost = 0;
    int work = 0, study = 0, personal = 0, health = 0;
    int total = m_calendarManager->m_localEvents.size();

    for (const auto &e : m_calendarManager->m_localEvents) {
        totalCost += e.cost;
        if (e.category == Category::Work)
            work++;
        else if (e.category == Category::Study)
            study++;
        else if (e.category == Category::Personal)
            personal++;
        else if (e.category == Category::Health)
            health++;
    }

    m_statTotalEvents->setText(QString("Tổng số sự kiện: %1").arg(total));
    m_statTotalCost->setText(QString("Tổng chi phí ước tính: %1 VNĐ").arg(totalCost));

    m_statWorkCount->setText(QString("💼 Công việc (Work): %1").arg(work));
    m_statStudyCount->setText(QString("📚 Học tập (Study): %1").arg(study));
    m_statPersonalCount->setText(QString("👤 Cá nhân (Personal): %1").arg(personal));
    m_statHealthCount->setText(QString("🏃 Sức khỏe (Health): %1").arg(health));

    if (total > 0) {
        m_statWorkProgress->setValue(work * 100 / total);
        m_statStudyProgress->setValue(study * 100 / total);
        m_statPersonalProgress->setValue(personal * 100 / total);
        m_statHealthProgress->setValue(health * 100 / total);
    }

    if (m_completionChart) {
        static_cast<CompletionChartWidget *>(m_completionChart)->updateChart();
    }
}

void ScheduleWidget::refreshCalendarGrid() {
    updateEventListView();
    updateStatistics();
    refreshDashboard();
}

void ScheduleWidget::handleDateSelected(const QDate &date) {
    m_startDateTime->setDate(date);
    m_endDateTime->setDate(date);
    updateEventListView();
}

void ScheduleWidget::handleGoogleSync() {
    QDate start = QDate::currentDate().addDays(-15);
    QDate end = QDate::currentDate().addDays(15);
    
    m_calendarManager->fetchGoogleCalendarEvents(start, end);
}

void ScheduleWidget::handleSaveSettings() {
    QString key = m_geminiApiKeyInput->text().trimmed();
    m_geminiService->setApiKey(key);

    QSettings settings;
    settings.setValue("gemini_api_key", key);

    QMessageBox::information(this, "Đã lưu", "Đã cập nhật Gemini API Key thành công!");
}

QWidget *ScheduleWidget::createLogsTab() {
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);

    m_logsDisplay = new QTextEdit(this);
    m_logsDisplay->setReadOnly(true);
    m_logsDisplay->setStyleSheet(
        "background-color: #111827; color: #E5E7EB; font-family: monospace; font-size: 13px; "
        "border-radius: 8px; padding: 10px;");

    g_globalLogDisplay = m_logsDisplay;

    layout->addWidget(new QLabel("📝 Lịch sử Hoạt động & Lỗi (Logs):", this));
    layout->addWidget(m_logsDisplay);

    qInfo() << "Đã khởi tạo hệ thống Logs.";

    return container;
}

void ScheduleWidget::handleExportCsv() {
    QString fileName = QFileDialog::getSaveFileName(this, "Xuất CSV", "", "CSV Files (*.csv)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Không thể mở file CSV để ghi:" << fileName;
        QMessageBox::critical(this, "Lỗi", "Không thể lưu file CSV.");
        return;
    }

    QTextStream out(&file);
    // Tiêu đề
    out << QString::fromUtf8(
        "Tiêu đề,Thời gian bắt đầu,Thời gian kết thúc,Thể loại,Độ ưu tiên,Chi phí (VNĐ),Địa "
        "điểm\n");

    for (const auto &e : m_calendarManager->m_localEvents) {
        QString priorityStr = e.priority == Priority::High
                                  ? "Cao"
                                  : (e.priority == Priority::Low ? "Thấp" : "Trung bình");

        // Thoát chuỗi nếu có dấu phẩy
        auto escapeCsv = [](QString s) {
            s.replace("\"", "\"\"");
            if (s.contains(",") || s.contains("\n") || s.contains("\"")) {
                return "\"" + s + "\"";
            }
            return s;
        };

        out << escapeCsv(e.title) << "," << e.startTime.toString("yyyy-MM-dd HH:mm:ss") << ","
            << e.endTime.toString("yyyy-MM-dd HH:mm:ss") << ","
            << CalendarManager::categoryToString(e.category) << "," << priorityStr << ","
            << QString::number(e.cost, 'f', 0) << "," << escapeCsv(e.location) << "\n";
    }

    file.close();
    qInfo() << "Đã xuất dữ liệu thành công ra CSV:" << fileName;
    QMessageBox::information(this, "Thành công", "Đã xuất file CSV thành công!");
}

void ScheduleWidget::applyModernStyle() {
    QFile file(":/styles/modern_light.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(QString::fromUtf8(file.readAll()));
        file.close();
        return;
    }

    QFile diskFile("styles/modern_light.qss");
    if (diskFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(QString::fromUtf8(diskFile.readAll()));
        diskFile.close();
    }
}

void ScheduleWidget::handleUndo() {
    m_calendarManager->undoLastAction();
}

void ScheduleWidget::updateUndoButton(bool canUndo) {
    if (m_undoBtn) {
        m_undoBtn->setEnabled(canUndo);
    }
}

void ScheduleWidget::handleGenerateDailySummary() {
    QDate today = QDate::currentDate();
    if (m_calendarWidget) {
        today = m_calendarWidget->selectedDate();
    }

    auto events = m_calendarManager->getEventsForDate(today);

    if (events.isEmpty()) {
        m_aiSummaryDisplay->setText(
            QString("Ngày %1 không có sự kiện nào để tóm tắt.").arg(today.toString("dd/MM/yyyy")));
        return;
    }

    QString prompt = QString(
                         "Hãy đóng vai là một trợ lý ảo cá nhân thông minh. Tóm tắt ngắn gọn và "
                         "động viên tôi cho lịch trình ngày %1. Danh sách sự kiện:\n")
                         .arg(today.toString("dd/MM/yyyy"));
    for (const auto &ev : events) {
        prompt += QString("- %1 (%2 - %3) tại %4. Ưu tiên: %5, Thể loại: %6\n")
                      .arg(ev.title)
                      .arg(ev.startTime.toString("HH:mm"))
                      .arg(ev.endTime.toString("HH:mm"))
                      .arg(ev.location.isEmpty() ? "Không xác định" : ev.location)
                      .arg(CalendarManager::priorityToString(ev.priority))
                      .arg(CalendarManager::categoryToString(ev.category));
    }

    m_aiSummaryBtn->setEnabled(false);
    m_aiSummaryBtn->setText("⏳ Đang tạo tóm tắt...");
    m_aiSummaryDisplay->setText("Đang gọi AI Gemini để phân tích...");

    m_geminiService->askGemini(prompt, [this](bool success, QString resp) {
        m_aiSummaryBtn->setEnabled(true);
        m_aiSummaryBtn->setText("✨ Tóm tắt lịch hôm nay bằng AI");
        if (success) {
            m_aiSummaryDisplay->setMarkdown(resp);
        } else {
            m_aiSummaryDisplay->setText("Lỗi: " + resp);
        }
    });
}

void ScheduleWidget::handleExportWeeklyPdf() {
    QDate selected = QDate::currentDate();
    if (m_calendarWidget) {
        selected = m_calendarWidget->selectedDate();
    }

    // Find Monday of this week
    int dayOfWeek = selected.dayOfWeek();  // 1 = Monday, 7 = Sunday
    QDate startOfWeek = selected.addDays(1 - dayOfWeek);
    QDate endOfWeek = startOfWeek.addDays(6);

    QString fileName = QFileDialog::getSaveFileName(
        this, "Xuất PDF Lịch tuần",
        QString("LichTuan_%1_den_%2.pdf")
            .arg(startOfWeek.toString("dd-MM-yyyy"), endOfWeek.toString("dd-MM-yyyy")),
        "PDF Files (*.pdf)");

    if (fileName.isEmpty())
        return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    // Use QPageLayout if available, or older setOrientation
    printer.setPageOrientation(QPageLayout::Landscape);

    QString html = QString(
        "<html><head><style>"
        "body { font-family: Arial, sans-serif; }"
        "h1 { text-align: center; color: #1e293b; }"
        "table { width: 100%; border-collapse: collapse; margin-top: 20px; }"
        "th, td { border: 1px solid #cbd5e1; padding: 8px; text-align: left; vertical-align: top; }"
        "th { background-color: #f1f5f9; font-weight: bold; width: 14%; }"
        ".event { margin-bottom: 8px; padding: 6px; border-radius: 4px; font-size: 12px; "
        "border-left: 4px solid #cbd5e1; background-color: #ffffff; }"
        ".high { border-left-color: #ef4444; }"
        ".medium { border-left-color: #f59e0b; }"
        ".low { border-left-color: #10b981; }"
        ".tag { display: inline-block; padding: 2px 4px; border-radius: 4px; font-size: 10px; "
        "font-weight: bold; margin-bottom: 4px; }"
        ".cat-study { background-color: #e0e7ff; color: #3730a3; }"
        ".cat-work { background-color: #dbeafe; color: #1e40af; }"
        ".cat-personal { background-color: #fce7f3; color: #be185d; }"
        ".cat-health { background-color: #d1fae5; color: #065f46; }"
        ".cat-other { background-color: #f3f4f6; color: #374151; }"
        "</style></head><body>");

    html += QString("<h1>Lịch trình Tuần (%1 - %2)</h1>")
                .arg(startOfWeek.toString("dd/MM/yyyy"), endOfWeek.toString("dd/MM/yyyy"));
    html += "<table><tr>";

    // Headers
    QStringList days = {"Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7", "Chủ nhật"};
    for (int i = 0; i < 7; ++i) {
        html +=
            QString("<th>%1<br/>%2</th>").arg(days[i], startOfWeek.addDays(i).toString("dd/MM"));
    }
    html += "</tr><tr>";

    // Content
    for (int i = 0; i < 7; ++i) {
        QDate d = startOfWeek.addDays(i);
        html += "<td>";
        auto events = m_calendarManager->getEventsForDate(d);
        std::sort(events.begin(), events.end(), [](const ScheduleEvent &a, const ScheduleEvent &b) {
            return a.startTime < b.startTime;
        });

        for (const auto &ev : events) {
            QString prioClass = "low";
            if (ev.priority == Priority::High)
                prioClass = "high";
            else if (ev.priority == Priority::Medium)
                prioClass = "medium";

            QString catClass = "cat-other";
            if (ev.category == Category::Study)
                catClass = "cat-study";
            else if (ev.category == Category::Work)
                catClass = "cat-work";
            else if (ev.category == Category::Personal)
                catClass = "cat-personal";
            else if (ev.category == Category::Health)
                catClass = "cat-health";

            html += QString("<div class='event %1'>").arg(prioClass);
            html += QString("<span class='tag %1'>%2</span><br/>")
                        .arg(catClass, CalendarManager::categoryToString(ev.category));
            html += QString("<b>%1</b><br/>").arg(ev.title);
            html += QString("<i>%1 - %2</i><br/>")
                        .arg(ev.startTime.toString("HH:mm"), ev.endTime.toString("HH:mm"));
            if (!ev.location.isEmpty()) {
                html += QString("📍 %1").arg(ev.location);
            }
            html += "</div>";
        }

        html += "</td>";
    }

    html += "</tr></table></body></html>";

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Thành công", "Đã xuất PDF Lịch tuần thành công:\n" + fileName);
}
