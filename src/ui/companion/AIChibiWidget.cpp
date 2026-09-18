#include "AIChibiWidget.h"

#include <QApplication>
#include <QPainter>
#include <QScreen>

AIChibiWidget::AIChibiWidget(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setFixedSize(120, 120);

    // Initial default position: bottom right
    if (auto screen = QApplication::primaryScreen()) {
        QRect geo = screen->availableGeometry();
        move(geo.width() - width() - 50, geo.height() - height() - 50);
    }

    m_animController = new AnimationController(this);
    connect(m_animController, &AnimationController::frameUpdated, this,
            &AIChibiWidget::onFrameUpdated);

    m_bubble = new ChatBubbleWidget(nullptr);  // No parent so it floats freely
}

AIChibiWidget::~AIChibiWidget() {
    delete m_bubble;
}

void AIChibiWidget::say(const QString &text, int duration) {
    m_bubble->showMessage(text, duration);
    // Position bubble above head
    QPoint globalPos = mapToGlobal(QPoint(0, 0));
    m_bubble->move(globalPos.x() + (width() - m_bubble->width()) / 2,
                   globalPos.y() - m_bubble->height() + 10);
    setState(ChibiState::Talking);

    // Auto reset to idle after duration
    QTimer::singleShot(duration, this, [this]() { setState(ChibiState::Idle); });
}

void AIChibiWidget::setState(ChibiState state) {
    m_animController->setState(state);
}

void AIChibiWidget::onFrameUpdated() {
    update();
}

void AIChibiWidget::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.drawPixmap(0, 0, m_animController->currentFrame());
}

void AIChibiWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
    // Simple click -> Open chat
    emit openChatRequested();
}

void AIChibiWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        // Move bubble alongside
        if (m_bubble->isVisible()) {
            QPoint globalPos = mapToGlobal(QPoint(0, 0));
            m_bubble->move(globalPos.x() + (width() - m_bubble->width()) / 2,
                           globalPos.y() - m_bubble->height() + 10);
        }
        event->accept();
    }
}

void AIChibiWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    emit openMainWindowRequested();
}
