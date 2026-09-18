#pragma once
#include <QObject>
#include <QTimer>

#include "../../ui/companion/AIChibiWidget.h"
#include "../../utils/GlobalHotkeyManager.h"
// #include "../GeminiService.h"

class AssistantService : public QObject {
    Q_OBJECT
public:
    explicit AssistantService(QObject *parent = nullptr);
    ~AssistantService();

    void start();

public slots:
    void simulateProactiveReminder();
    void onUserAsking();
    void onGeminiReplied(const QString &reply);

private:
    AIChibiWidget *m_chibi;
    GlobalHotkeyManager *m_hotkey;
    QTimer *m_proactiveTimer;
};
