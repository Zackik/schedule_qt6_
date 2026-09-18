#pragma once
#include <QMouseEvent>
#include <QWidget>

#include "../../services/companion/AnimationController.h"
#include "ChatBubbleWidget.h"

class CompanionChatWindow;

class AIChibiWidget : public QWidget {
    Q_OBJECT
public:
    explicit AIChibiWidget(QWidget *parent = nullptr);
    ~AIChibiWidget();

    void say(const QString &text, int duration = 5000);
    void setState(ChibiState state);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
    void openChatRequested();
    void openMainWindowRequested();

private slots:
    void onFrameUpdated();

private:
    QPoint m_dragPosition;
    AnimationController *m_animController;
    ChatBubbleWidget *m_bubble;
    CompanionChatWindow
        *m_chatWindow;  // Not full implementation here to save space, assuming signal based
};
