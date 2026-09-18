#pragma once
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWidget>

class ChatBubbleWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChatBubbleWidget(QWidget *parent = nullptr);

    void showMessage(const QString &text, int durationMs = 5000);
    void hideBubble();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateTypingEffect();

private:
    QLabel *m_textLabel;
    QTimer *m_hideTimer;
    QTimer *m_typingTimer;

    QString m_fullText;
    int m_typingIndex;
};
