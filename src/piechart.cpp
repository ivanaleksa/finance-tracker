#include "piechart.h"
#include <QPainterPath>
#include <QtMath>

PieChart::PieChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    
    // colore pallete
    m_colors = {
        QColor("#3498db"), QColor("#e74c3c"), QColor("#2ecc71"), QColor("#f39c12"),
        QColor("#9b59b6"), QColor("#1abc9c"), QColor("#e67e22"), QColor("#34495e"),
        QColor("#16a085"), QColor("#c0392b"), QColor("#8e44ad"), QColor("#27ae60"),
        QColor("#d35400"), QColor("#2980b9"), QColor("#f1c40f"), QColor("#7f8c8d")
    };
}

void PieChart::setData(const QMap<QString, double>& data)
{
    m_data = data;
    update();
}

void PieChart::setTitle(const QString& title)
{
    m_title = title;
    update();
}

void PieChart::clear()
{
    m_data.clear();
    m_title.clear();
    update();
}

void PieChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor("#ffffff"));

    if (!m_title.isEmpty()) {
        painter.setPen(QColor("#2c3e50"));
        QFont titleFont = font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        painter.setFont(titleFont);
        painter.drawText(QRect(0, 10, width(), 30), Qt::AlignCenter, m_title);
    }
    
    if (m_data.isEmpty()) {
        painter.setPen(QColor("#7f8c8d"));
        QFont msgFont = font();
        msgFont.setPointSize(12);
        painter.setFont(msgFont);
        painter.drawText(rect(), Qt::AlignCenter, "Нет данных за выбранный период");
        return;
    }

    int legendWidth = 200;
    QRect pieRect(20, 50, width() - legendWidth - 40, height() - 70);
    QRect legendRect(width() - legendWidth - 10, 50, legendWidth, height() - 70);
    
    drawPie(painter, pieRect);
    drawLegend(painter, legendRect);
}

void PieChart::drawPie(QPainter& painter, const QRect& rect)
{
    double total = 0;
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        total += it.value();
    }
    
    if (total <= 0) return;
    
    // circle size
    int diameter = qMin(rect.width(), rect.height()) - 20;
    QRect pieRect(
        rect.x() + (rect.width() - diameter) / 2,
        rect.y() + (rect.height() - diameter) / 2,
        diameter, diameter
    );
    
    int startAngle = 90 * 16; // begin from the top
    int colorIndex = 0;
    
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        double percentage = it.value() / total;
        int spanAngle = static_cast<int>(-percentage * 360 * 16);
        
        QColor color = m_colors[colorIndex % m_colors.size()];
        painter.setBrush(color);
        painter.setPen(QPen(Qt::white, 2));
        painter.drawPie(pieRect, startAngle, spanAngle);
        
        // titels for large sectors (>5%)
        if (percentage >= 0.05) {
            double midAngle = (startAngle + spanAngle / 2) / 16.0;
            double radians = qDegreesToRadians(midAngle);
            
            int labelRadius = diameter / 2 - 40;
            int labelX = pieRect.center().x() + static_cast<int>(labelRadius * qCos(radians));
            int labelY = pieRect.center().y() - static_cast<int>(labelRadius * qSin(radians));
            
            painter.setPen(Qt::white);
            QFont labelFont = font();
            labelFont.setPointSize(9);
            labelFont.setBold(true);
            painter.setFont(labelFont);
            
            QString label = QString("%1%").arg(percentage * 100, 0, 'f', 1);
            QRect labelRect(labelX - 25, labelY - 10, 50, 20);
            painter.drawText(labelRect, Qt::AlignCenter, label);
        }
        
        startAngle += spanAngle;
        colorIndex++;
    }
}

void PieChart::drawLegend(QPainter& painter, const QRect& rect)
{
    int y = rect.y() + 10;
    int colorIndex = 0;
    
    QFont legendFont = font();
    legendFont.setPointSize(10);
    painter.setFont(legendFont);
    
    double total = 0;
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        total += it.value();
    }
    
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        if (y > rect.bottom() - 20) break;
        
        QColor color = m_colors[colorIndex % m_colors.size()];
        
        // color rect
        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect.x(), y, 16, 16, 3, 3);
        
        painter.setPen(QColor("#2c3e50"));
        QString text = it.key();
        if (text.length() > 18) {
            text = text.left(15) + "...";
        }
        
        double percentage = (total > 0) ? (it.value() / total * 100) : 0;
        QString label = QString("%1 (%2%)").arg(text).arg(percentage, 0, 'f', 1);
        painter.drawText(rect.x() + 22, y, rect.width() - 22, 20, Qt::AlignLeft | Qt::AlignVCenter, label);
        
        y += 24;
        colorIndex++;
    }
}
