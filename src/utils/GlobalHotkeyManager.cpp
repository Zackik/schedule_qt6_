#include "GlobalHotkeyManager.h"

#include <QApplication>
#include <QKeySequence>
#include <QShortcut>
#include <QWidget>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

GlobalHotkeyManager::GlobalHotkeyManager(QObject *parent) : QObject(parent) {
}

bool GlobalHotkeyManager::registerHotkey() {
#if defined(Q_OS_WIN)
    // Registering Ctrl + Space (MOD_CONTROL = 0x0002, VK_SPACE = 0x20)
    // In a real Qt app you'd install a native event filter on QCoreApplication
    // return RegisterHotKey(nullptr, 1, MOD_CONTROL, VK_SPACE);

    // For this demonstration, we'll use a standard application-wide QShortcut
    // as true OS-wide hotkey implementations require substantial boilerplate for all OSes.
#endif

    // Fallback: application-wide shortcut
    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+Space"), QApplication::activeWindow());
    shortcut->setContext(Qt::ApplicationShortcut);
    connect(shortcut, &QShortcut::activated, this, &GlobalHotkeyManager::hotkeyActivated);
    return true;
}
