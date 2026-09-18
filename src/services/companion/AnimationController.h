#pragma once
#include <QObject>
#include <QPixmap>
#include <QTimer>

enum class ChibiState {
    Idle,
    Thinking,
    Talking,
    Happy,
    Excited,
    Sleep,
    Confused,
    Worried,
    Reminder,
    Celebrating
};

class AnimationController : public QObject {
    Q_OBJECT
public:
    explicit AnimationController(QObject *parent = nullptr);

    void setState(ChibiState state);
    ChibiState currentState() const {
        return m_currentState;
    }

    // Returns current frame to draw
    QPixmap currentFrame() const;

signals:
    void frameUpdated();

private slots:
    void updateFrame();

private:
    ChibiState m_currentState = ChibiState::Idle;
    QTimer *m_timer;
    int m_frameIndex = 0;

    // Generates a fallback programmatic drawing if assets are missing
    QPixmap generateFallbackFrame(ChibiState state, int frame) const;
};
