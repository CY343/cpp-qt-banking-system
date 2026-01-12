#ifndef TOAST_H
#define TOAST_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

class Toast : public QWidget {
    Q_OBJECT
public:
    enum Kind { Success, Error, Info };

    explicit Toast(const QString& message,
                   Kind kind = Info,
                   QWidget* parent = nullptr)
        : QWidget(nullptr)   // <-- top-level window (not a child widget)
    {
        // Window behavior (better than ToolTip for this use)
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_ShowWithoutActivating);
        setAttribute(Qt::WA_TranslucentBackground);

        // Container (single layer, avoids “double background” look)
        auto *container = new QWidget(this);
        container->setObjectName("toastContainer");

        auto *layout = new QHBoxLayout(container);
        layout->setContentsMargins(14, 10, 14, 10);
        layout->setSpacing(10);

        auto *icon = new QLabel(container);
        icon->setObjectName("toastIcon");
        icon->setText(kind == Success ? "✅" : (kind == Error ? "⚠️" : "ℹ️"));
        icon->setAlignment(Qt::AlignVCenter);

        auto *text = new QLabel(message, container);
        text->setObjectName("toastText");
        text->setWordWrap(true);

        layout->addWidget(icon);
        layout->addWidget(text, 1);

        auto *root = new QHBoxLayout(this);
        root->setContentsMargins(0, 0, 0, 0);
        root->addWidget(container);

        // Color palette that matches your blue theme
        // (Success = teal/green that fits your #1F497D)
        QString bg = (kind == Success) ? "#1E7B6D" : (kind == Error) ? "#B3261E" : "#1F497D";

        container->setStyleSheet(QString(R"(
            QWidget#toastContainer{
                background-color:%1;
                border-radius:12px;
            }
            QLabel#toastText{
                color:white;
                font-weight:600;
            }
            QLabel#toastIcon{
                color:white;
                font-size:14pt;
            }
        )").arg(bg));

        // Real Qt shadow (instead of CSS box-shadow)
        auto *shadow = new QGraphicsDropShadowEffect(container);
        shadow->setBlurRadius(18);
        shadow->setOffset(0, 6);
        container->setGraphicsEffect(shadow);

        // Force final size ONCE to avoid layered-window resize bugs
        adjustSize();
        setFixedSize(sizeHint());

        setWindowOpacity(0.0);
    }

    // Center in parent window
    void popupCentered(QWidget* parentWindow, int durationMs = 1800)
    {
        if (!parentWindow) { show(); return; }

        // Center in the parent window (global)
        QRect pr = parentWindow->geometry();
        int x = pr.x() + (pr.width() - width()) / 2;
        int y = pr.y() + (pr.height() - height()) / 2;
        move(x, y);

        show();
        raise();

        // Fade in (windowOpacity is more stable on Windows than QGraphicsOpacityEffect)
        auto *fadeIn = new QPropertyAnimation(this, "windowOpacity", this);
        fadeIn->setDuration(160);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

        // Fade out then delete
        QTimer::singleShot(durationMs, this, [this]() {
            auto *fadeOut = new QPropertyAnimation(this, "windowOpacity", this);
            fadeOut->setDuration(220);
            fadeOut->setStartValue(1.0);
            fadeOut->setEndValue(0.0);
            connect(fadeOut, &QPropertyAnimation::finished, this, &QWidget::deleteLater);
            fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }
};

#endif // TOAST_H
