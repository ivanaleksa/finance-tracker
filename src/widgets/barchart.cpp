#include "widgets/barchart.h"
#include <QtMath>

BarChart::BarChart(QWidget *parent)
    : QWidget(parent)
    , m_barColor("#3498db")
{
    setMinimumSize(400, 300);
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

void BarChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // background
    painter.fillRect(rect(), QColor("#ffffff"));
    
    // title
    if (!m_title.isEmpty()) {
        painter.setPen(QColor("#2c3e50"));
        QFont titleFont = font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        painter.setFont(titleFont);
        painter.drawText(QRect(0, 10, width(), 30), Qt::AlignCenter, m_title);
    }
    
    // area for chart
    QRect chartRect(70, 50, width() - 90, height() - 100);
    
    // max value
    double maxValue = 0;
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        if (it.value() > maxValue) maxValue = it.value();
    }
    
    if (maxValue <= 0) {
        painter.setPen(QColor("#7f8c8d"));
        QFont msgFont = font();
        msgFont.setPointSize(12);
        painter.setFont(msgFont);
        painter.drawText(rect(), Qt::AlignCenter, "Нет данных за выбранный период");
        return;
    }
    
    // ceil the value for prettier chart
    double scale = qPow(10, qFloor(qLn(maxValue) / qLn(10)));
    maxValue = qCeil(maxValue / scale) * scale * 1.1;
    
    drawAxes(painter, chartRect, maxValue);
    drawBars(painter, chartRect);
}

void BarChart::drawAxes(QPainter& painter, const QRect& rect, double maxValue)
{
    painter.setPen(QPen(QColor("#bdc3c7"), 1));
    
    // horizontal grid line and y axes title
    QFont axisFont = font();
    axisFont.setPointSize(9);
    painter.setFont(axisFont);
    
    int numLines = 5;
    for (int i = 0; i <= numLines; ++i) {
        int y = rect.bottom() - (rect.height() * i / numLines);
        
        // grid lines
        painter.setPen(QPen(QColor("#ecf0f1"), 1, Qt::DashLine));
        painter.drawLine(rect.left(), y, rect.right(), y);
        
        double value = maxValue * i / numLines;
        QString label;
        if (value >= 1000000) {
            label = QString("%1М").arg(value / 1000000, 0, 'f', 1);
        } else if (value >= 1000) {
            label = QString("%1К").arg(value / 1000, 0, 'f', 0);
        } else {
            label = QString::number(static_cast<int>(value));
        }
        
        painter.setPen(QColor("#7f8c8d"));
        painter.drawText(QRect(0, y - 10, rect.left() - 5, 20), Qt::AlignRight | Qt::AlignVCenter, label);
    }
    
    // X axes
    painter.setPen(QPen(QColor("#bdc3c7"), 2));
    painter.drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
}

void BarChart::drawBars(QPainter& painter, const QRect& rect)
{
    if (m_data.isEmpty()) return;
    
    double maxValue = 0;
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        if (it.value() > maxValue) maxValue = it.value();
    }
    
    if (maxValue <= 0) return;

    double scale = qPow(10, qFloor(qLn(maxValue) / qLn(10)));
    maxValue = qCeil(maxValue / scale) * scale * 1.1;
    
    int barCount = m_data.size();
    double barWidth = (rect.width() - 20.0) / barCount * 0.7;
    double gap = (rect.width() - 20.0) / barCount * 0.3;
    double x = rect.left() + 10 + gap / 2;
    
    QFont labelFont = font();
    labelFont.setPointSize(9);
    painter.setFont(labelFont);
    
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

        QLinearGradient gradient(barRect.topLeft(), barRect.bottomLeft());
        gradient.setColorAt(0, m_barColor.lighter(110));
        gradient.setColorAt(1, m_barColor);
        
        painter.setBrush(gradient);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(barRect, 4, 4);

        if (value > 0) {
            painter.setPen(QColor("#2c3e50"));
            QString valueStr;
            if (value >= 1000000) {
                valueStr = QString("%1М").arg(value / 1000000, 0, 'f', 1);
            } else if (value >= 1000) {
                valueStr = QString("%1К").arg(value / 1000, 0, 'f', 0);
            } else {
                valueStr = QString::number(static_cast<int>(value));
            }
            
            QRect valueRect(static_cast<int>(x - 10), barRect.top() - 20, static_cast<int>(barWidth + 20), 18);
            painter.drawText(valueRect, Qt::AlignCenter, valueStr);
        }

        painter.setPen(QColor("#7f8c8d"));
        QString label = (index < m_labels.size()) ? m_labels[index] : QString::number(it.key());
        QRect labelRect(static_cast<int>(x - 5), rect.bottom() + 5, static_cast<int>(barWidth + 10), 20);
        painter.drawText(labelRect, Qt::AlignCenter, label);
        
        x += barWidth + gap;
        index++;
    }
}
