#include "widgets/barchart.h"
#include <QMouseEvent>
#include <QtMath>

BarChart::BarChart(QWidget *parent)
    : QWidget(parent)
    , m_barColor("#3498db")
{
    setMinimumSize(400, 300);
    setMouseTracking(true);

    m_labels = {"Янв", "Фев", "Мар", "Апр", "Май", "Июн",
                "Июл", "Авг", "Сен", "Окт", "Ноя", "Дек"};
}

void BarChart::setData(const QMap<int, double>& data)
{
    m_data = data;
    update();
}

void BarChart::setTitle(const QString& title)
{
    m_title = title;
    update();
}

void BarChart::setBarColor(const QColor& color)
{
    m_barColor = color;
    update();
}

void BarChart::setLabels(const QStringList& labels)
{
    m_labels = labels;
    update();
}

void BarChart::clear()
{
    m_data.clear();
    m_title.clear();
    update();
}

// mouse events

void BarChart::mouseMoveEvent(QMouseEvent *event)
{
    int idx = hitTestBar(event->pos());
    if (idx != m_hoveredIndex) {
        m_hoveredIndex = idx;
        update();
    }
}

void BarChart::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hoveredIndex = -1;
    update();
}

int BarChart::hitTestBar(const QPoint& pos) const
{
    for (int i = 0; i < m_bars.size(); ++i) {
        if (m_bars[i].rect.contains(pos))
            return i;
    }
    return -1;
}

// drawing

void BarChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255));
    painter.drawRoundedRect(rect(), 10, 10);
    painter.setPen(QPen(QColor(0, 0, 0, 15), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 10, 10);

    if (!m_title.isEmpty()) {
        painter.setPen(QColor("#2c3e50"));
        QFont titleFont = font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        painter.setFont(titleFont);
        painter.drawText(QRect(0, 10, width(), 30), Qt::AlignCenter, m_title);
    }

    QRect chartRect(70, 50, width() - 90, height() - 100);

    double maxValue = 0;
    for (auto it = m_data.begin(); it != m_data.end(); ++it)
        if (it.value() > maxValue) maxValue = it.value();

    if (maxValue <= 0) {
        painter.setPen(QColor("#7f8c8d"));
        QFont msgFont = font();
        msgFont.setPointSize(12);
        painter.setFont(msgFont);
        painter.drawText(rect(), Qt::AlignCenter, "Нет данных за выбранный период");
        return;
    }

    double scale = qPow(10, qFloor(qLn(maxValue) / qLn(10)));
    maxValue = qCeil(maxValue / scale) * scale * 1.1;

    drawAxes(painter, chartRect, maxValue);
    drawBars(painter, chartRect);

    if (m_hoveredIndex >= 0 && m_hoveredIndex < m_bars.size())
        drawTooltip(painter, m_bars[m_hoveredIndex]);
}

void BarChart::drawAxes(QPainter& painter, const QRect& rect, double maxValue)
{
    QFont axisFont = font();
    axisFont.setPointSize(9);
    painter.setFont(axisFont);

    int numLines = 5;
    for (int i = 0; i <= numLines; ++i) {
        int y = rect.bottom() - (rect.height() * i / numLines);

        painter.setPen(QPen(QColor("#ecf0f1"), 1, Qt::DashLine));
        painter.drawLine(rect.left(), y, rect.right(), y);

        double value = maxValue * i / numLines;
        QString label;
        if (value >= 1000000)
            label = QString("%1М").arg(value / 1000000, 0, 'f', 1);
        else if (value >= 1000)
            label = QString("%1К").arg(value / 1000, 0, 'f', 0);
        else
            label = QString::number(static_cast<int>(value));

        painter.setPen(QColor("#7f8c8d"));
        painter.drawText(QRect(0, y - 10, rect.left() - 5, 20),
                         Qt::AlignRight | Qt::AlignVCenter, label);
    }

    painter.setPen(QPen(QColor("#bdc3c7"), 2));
    painter.drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
}

void BarChart::drawBars(QPainter& painter, const QRect& rect)
{
    if (m_data.isEmpty()) return;

    double maxValue = 0;
    for (auto it = m_data.begin(); it != m_data.end(); ++it)
        if (it.value() > maxValue) maxValue = it.value();
    if (maxValue <= 0) return;

    double scale = qPow(10, qFloor(qLn(maxValue) / qLn(10)));
    maxValue = qCeil(maxValue / scale) * scale * 1.1;

    int barCount = m_data.size();
    double barWidth = (rect.width() - 20.0) / barCount * 0.7;
    double gap      = (rect.width() - 20.0) / barCount * 0.3;
    double x        = rect.left() + 10 + gap / 2;

    QFont labelFont = font();
    labelFont.setPointSize(9);
    painter.setFont(labelFont);

    m_bars.clear();

    int index = 0;
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        double value = it.value();
        int barHeight = static_cast<int>((value / maxValue) * rect.height());

        QRect barRect(
            static_cast<int>(x),
            rect.bottom() - barHeight,
            static_cast<int>(barWidth),
            barHeight
            );

        BarInfo info;
        info.rect  = barRect;
        info.value = value;
        info.label = (index < m_labels.size()) ? m_labels[index] : QString::number(it.key());
        m_bars.append(info);

        bool isHovered = (index == m_hoveredIndex);

        QRect drawRect = barRect;
        if (isHovered)
            drawRect.adjust(-2, -3, 2, 0);

        QColor baseColor = isHovered ? m_barColor.lighter(120) : m_barColor;
        QLinearGradient gradient(drawRect.topLeft(), drawRect.bottomLeft());
        gradient.setColorAt(0, baseColor.lighter(110));
        gradient.setColorAt(1, baseColor);

        painter.setBrush(gradient);
        painter.setPen(isHovered ? QPen(m_barColor.darker(120), 1) : Qt::NoPen);
        painter.drawRoundedRect(drawRect, 4, 4);

        if (value > 0) {
            painter.setPen(isHovered ? QColor(m_barColor.darker(140)) : QColor("#2c3e50"));
            QString valueStr;
            if (value >= 1000000)
                valueStr = QString("%1М").arg(value / 1000000, 0, 'f', 1);
            else if (value >= 1000)
                valueStr = QString("%1К").arg(value / 1000, 0, 'f', 0);
            else
                valueStr = QString::number(static_cast<int>(value));

            QRect valueRect(static_cast<int>(x - 10), drawRect.top() - 20,
                            static_cast<int>(barWidth + 20), 18);
            painter.drawText(valueRect, Qt::AlignCenter, valueStr);
        }

        painter.setPen(isHovered ? QColor("#2c3e50") : QColor("#7f8c8d"));
        QRect labelRect(static_cast<int>(x - 5), rect.bottom() + 5,
                        static_cast<int>(barWidth + 10), 20);
        painter.drawText(labelRect, Qt::AlignCenter, info.label);

        x += barWidth + gap;
        index++;
    }
}

void BarChart::drawTooltip(QPainter& painter, const BarInfo& bar)
{
    QString line1 = bar.label;
    QString line2 = QString("Сумма: %1").arg(bar.value, 0, 'f', 2);

    QFont f = font();
    f.setPointSize(10);
    painter.setFont(f);
    QFontMetrics fm(f);

    const int padding = 10;
    int w = std::max(fm.horizontalAdvance(line1),
                     fm.horizontalAdvance(line2)) + padding * 2 + 8;
    int h = fm.height() * 2 + padding * 2 + 4;

    int tx = bar.rect.center().x() - w / 2;
    int ty = bar.rect.top() - h - 8;

    tx = qMax(tx, 4);
    tx = qMin(tx, width() - w - 4);

    if (ty < 4) ty = bar.rect.bottom() + 8;

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 30));
    painter.drawRoundedRect(tx + 3, ty + 3, w, h, 6, 6);

    painter.setBrush(QColor(44, 62, 80, 230));
    painter.drawRoundedRect(tx, ty, w, h, 6, 6);

    painter.setBrush(m_barColor);
    painter.drawRoundedRect(tx, ty, 4, h, 2, 2);

    QFont boldF = f;
    boldF.setBold(true);
    painter.setFont(boldF);
    painter.setPen(Qt::white);
    painter.drawText(tx + padding, ty + padding, w, fm.height(), Qt::AlignLeft, line1);

    painter.setFont(f);
    painter.setPen(QColor(189, 195, 199));
    painter.drawText(tx + padding, ty + padding + fm.height() + 3, w, fm.height(), Qt::AlignLeft, line2);
}