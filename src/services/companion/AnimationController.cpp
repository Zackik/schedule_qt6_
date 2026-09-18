#include "AnimationController.h"

#include <QPainter>
#include <QPainterPath>

AnimationController::AnimationController(QObject *parent) : QObject(parent) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &AnimationController::updateFrame);
    m_timer->start(200);  // 5 fps for simple idle, can be dynamic
}

void AnimationController::setState(ChibiState state) {
    if (m_currentState == state)
        return;
    m_currentState = state;
    m_frameIndex = 0;

    // Adjust animation speed based on state
    if (state == ChibiState::Excited || state == ChibiState::Talking)
        m_timer->start(100);
    else
        m_timer->start(300);

    emit frameUpdated();
}

void AnimationController::updateFrame() {
    m_frameIndex = (m_frameIndex + 1) % 4;  // 4 frames per animation cycle
    emit frameUpdated();
}

QPixmap AnimationController::currentFrame() const {
    // In a real app, this loads from QPixmapCache or QImage array.
    // For this boilerplate, we draw a dynamic procedural Chibi using QPainter.
    return generateFallbackFrame(m_currentState, m_frameIndex);
}

QPixmap AnimationController::generateFallbackFrame(ChibiState state, int frame) const {
    QPixmap pixmap(120, 120);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    // Bouncing offset
    int yOffset = (frame % 2 == 0) ? 2 : 0;
    if (state == ChibiState::Excited)
        yOffset = (frame % 2 == 0) ? -5 : 5;
    if (state == ChibiState::Sleep)
        yOffset = 5;

    // Draw Body
    p.setBrush(QColor("#8A2BE2"));  // Neon Purple/Blue vibe
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(30, 50 + yOffset, 60, 50, 20, 20);

    // Draw Head
    p.setBrush(QColor("#1E90FF"));  // Head color
    p.drawEllipse(20, 10 + yOffset, 80, 70);

    // Draw Eyes based on state
    p.setBrush(Qt::white);
    if (state == ChibiState::Sleep || state == ChibiState::Happy) {
        p.setPen(QPen(Qt::white, 3));
        p.drawLine(35, 45 + yOffset, 45, 40 + yOffset);
        p.drawLine(75, 45 + yOffset, 85, 40 + yOffset);
    } else if (state == ChibiState::Thinking) {
        p.drawEllipse(35, 40 + yOffset, 10, 10);
        p.drawEllipse(75, 35 + yOffset, 10, 10);
    } else if (state == ChibiState::Worried || state == ChibiState::Confused) {
        p.drawEllipse(35, 35 + yOffset, 12, 12);
        p.drawEllipse(75, 35 + yOffset, 8, 8);
    } else {
        // Idle/Normal
        p.drawEllipse(35, 40 + yOffset, 10, 10);
        p.drawEllipse(75, 40 + yOffset, 10, 10);
    }

    // Mouth
    p.setPen(QPen(Qt::white, 2));
    if (state == ChibiState::Talking) {
        int mouthH = (frame % 2 == 0) ? 10 : 4;
        p.drawEllipse(55, 55 + yOffset, 10, mouthH);
    } else if (state == ChibiState::Thinking) {
        p.drawLine(55, 60 + yOffset, 65, 60 + yOffset);
    } else if (state == ChibiState::Happy || state == ChibiState::Excited) {
        p.drawArc(50, 55 + yOffset, 20, 10, 180 * 16, 180 * 16);
    }

    return pixmap;
}
