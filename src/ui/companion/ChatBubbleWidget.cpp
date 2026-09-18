#include "ChatBubbleWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <algorithm>

ChatBubbleWidget::ChatBubbleWidget(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                   Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground);

    m_textLabel = new QLabel(this);
    m_textLabel->setWordWrap(true);
    m_textLabel->setStyleSheet(
        "color: white; font-weight: bold; font-family: sans-serif; font-size: 13px;");
    m_textLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 10, 15, 20);  // Bottom margin for the bubble tail
    layout->addWidget(m_textLabel);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);

    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    connect(m_hideTimer, &QTimer::timeout, this, &ChatBubbleWidget::hideBubble);

    m_typingTimer = new QTimer(this);
    connect(m_typingTimer, &QTimer::timeout, this, &ChatBubbleWidget::updateTypingEffect);
}

void ChatBubbleWidget::showMessage(const QString &text, int durationMs) {
    m_fullText = text;
    m_typingIndex = 0;
    m_textLabel->setText("");

    // Auto resize
    int textWidth = static_cast<int>(m_fullText.length()) * 7;
    int width = std::clamp(textWidth, 100, 250);
    resize(width, 80);

    m_typingTimer->start(30);  // Typing speed
    show();

    if (durationMs > 0) {
        int displayDuration = durationMs + static_cast<int>(m_fullText.length()) * 30;
        m_hideTimer->start(displayDuration);
    }
}

void ChatBubbleWidget::updateTypingEffect() {
    if (m_typingIndex <= m_fullText.length()) {
        m_textLabel->setText(m_fullText.left(m_typingIndex));
        m_typingIndex++;
    } else {
        m_typingTimer->stop();
    }
}

void ChatBubbleWidget::hideBubble() {
    hide();
}

void ChatBubbleWidget::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    QRectF rect = this->rect().adjusted(5, 5, -5, -20);
    path.addRoundedRect(rect, 15, 15);

    // Tail
    path.moveTo(rect.center().x() - 10, rect.bottom());
    path.lineTo(rect.center().x(), rect.bottom() + 15);
    path.lineTo(rect.center().x() + 10, rect.bottom());

    p.fillPath(path, QColor(40, 42, 54, 230));      // Dracula-ish glass dark
    p.setPen(QPen(QColor(189, 147, 249, 150), 2));  // Neon border
    p.drawPath(path);
}
