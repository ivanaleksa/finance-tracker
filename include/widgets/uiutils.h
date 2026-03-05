#pragma once

#include <QWidget>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPen>
#include <QColor>

namespace UiUtils {

inline void applyShadow(QWidget *widget)
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(15);
    shadow->setXOffset(0);
    shadow->setYOffset(2);
    shadow->setColor(QColor(0, 0, 0, 40));
    widget->setGraphicsEffect(shadow);
}

} // namespace UiUtils

// Базовый виджет-остров: рисует скруглённый белый фон в paintEvent,
// оставляя углы прозрачными — тень QGraphicsDropShadowEffect получается скруглённой.
class IslandWidget : public QWidget
{
public:
    explicit IslandWidget(QWidget *parent = nullptr) : QWidget(parent) {}

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255));
        painter.drawRoundedRect(rect(), 10, 10);

        painter.setPen(QPen(QColor(0, 0, 0, 15), 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 10, 10);
    }
};
