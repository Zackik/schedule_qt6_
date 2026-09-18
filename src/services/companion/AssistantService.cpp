#include "AssistantService.h"

#include <QDebug>

AssistantService::AssistantService(QObject *parent) : QObject(parent) {
    m_chibi = new AIChibiWidget(nullptr);  // No parent so it's a top-level window
    m_hotkey = new GlobalHotkeyManager(this);

    connect(m_hotkey, &GlobalHotkeyManager::hotkeyActivated, this, [this]() {
        m_chibi->say("Bạn cần mình giúp gì?", 4000);
        // Here we would also show the ChatWindow
    });

    connect(m_chibi, &AIChibiWidget::openChatRequested, this, &AssistantService::onUserAsking);

    m_proactiveTimer = new QTimer(this);
    connect(m_proactiveTimer, &QTimer::timeout, this, &AssistantService::simulateProactiveReminder);
}

AssistantService::~AssistantService() {
    delete m_chibi;
}

void AssistantService::start() {
    m_chibi->show();
    m_hotkey->registerHotkey();

    // Simulate a reminder popping up every 60 seconds for demonstration
    m_proactiveTimer->start(60000);

    // Initial greeting
    QTimer::singleShot(1000, this,
                       [this]() { m_chibi->say("Xin chào! Mình là trợ lý AI của bạn.", 4000); });
}

void AssistantService::onUserAsking() {
    m_chibi->setState(ChibiState::Thinking);
    m_chibi->say("Hmm... để mình suy nghĩ nhé...", 3000);

    // Simulate Gemini API network delay
    QTimer::singleShot(2500, this, [this]() {
        onGeminiReplied("✨ Mình đề xuất học C++ từ 14:00 - 15:30. Bạn thấy sao?");
    });
}

void AssistantService::onGeminiReplied(const QString &reply) {
    m_chibi->say(reply, 6000);
}

void AssistantService::simulateProactiveReminder() {
    m_chibi->setState(ChibiState::Reminder);
    m_chibi->say("⏰ Còn 15 phút nữa đến giờ học C++ nhé!", 5000);
}
