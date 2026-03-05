#include "widgets/piechart.h"
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

void PieChart::setNoDataMessage(const QString& message)
{
    m_noDataMessage = message;
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
        painter.drawText(rect(), Qt::AlignCenter, m_noDataMessage);
        return;
    }

    int legendWidth = 220;
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

    QList<QPair<QString, double>> sortedData;
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        sortedData.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedData.begin(), sortedData.end(),
              [](const QPair<QString, double>& a, const QPair<QString, double>& b) {
                  return a.second > b.second;
              });

    // circle size
    int diameter = qMin(rect.width(), rect.height()) - 20;
    QRect pieRect(
        rect.x() + (rect.width() - diameter) / 2,
        rect.y() + (rect.height() - diameter) / 2,
        diameter, diameter
        );

    int startAngle = 90 * 16; // begin from the top
    int colorIndex = 0;

    for (const auto& item : sortedData) {
        double percentage = item.second / total;
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
    legendFont.setPointSize(9);
    painter.setFont(legendFont);

    double total = 0;
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        total += it.value();
    }

    QList<QPair<QString, double>> sortedData;
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        sortedData.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedData.begin(), sortedData.end(),
              [](const QPair<QString, double>& a, const QPair<QString, double>& b) {
                  return a.second > b.second;
              });

    for (const auto& item : sortedData) {
        if (y > rect.bottom() - 20) break;

        QColor color = m_colors[colorIndex % m_colors.size()];

        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect.x(), y, 16, 16, 3, 3);

        painter.setPen(QColor("#2c3e50"));

        double percentage = (total > 0) ? (item.second / total * 100) : 0;
        QString fullText = QString("%1 - %2 (%3%)")
                               .arg(item.first)
                               .arg(item.second, 0, 'f', 2)
                               .arg(percentage, 0, 'f', 1);

        QFontMetrics fm(legendFont);
        int textWidth = rect.width() - 22;
        QStringList lines;

        if (fm.horizontalAdvance(fullText) > textWidth) {
            QString shortName = item.first;
            if (shortName.length() > 15) {
                shortName = shortName.left(12) + "...";
            }

            QString line1 = shortName;
            QString line2 = QString("%1 (%2%)")
                                .arg(item.second, 0, 'f', 2)
                                .arg(percentage, 0, 'f', 1);

            lines << line1 << line2;
        } else {
            lines << fullText;
        }

        int lineHeight = fm.height();
        int currentY = y;

        for (const QString& line : lines) {
            if (currentY + lineHeight > rect.bottom()) break;

            QString displayLine = fm.elidedText(line, Qt::ElideRight, textWidth);
            painter.drawText(rect.x() + 22, currentY, textWidth, lineHeight,
                             Qt::AlignLeft | Qt::AlignVCenter, displayLine);
            currentY += lineHeight;
        }

        y += lineHeight * lines.size() + 8;
        colorIndex++;
    }
}
