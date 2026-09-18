#pragma once
#include <QObject>

class GlobalHotkeyManager : public QObject {
    Q_OBJECT
public:
    explicit GlobalHotkeyManager(QObject *parent = nullptr);
    bool registerHotkey();  // e.g. Ctrl + Space

signals:
    void hotkeyActivated();

#if defined(Q_OS_WIN)
protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result);
#endif
};
