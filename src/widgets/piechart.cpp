#include "widgets/piechart.h"
#include <QPainterPath>
#include <QMouseEvent>
#include <QtMath>
#include <algorithm>

PieChart::PieChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    setMouseTracking(true);

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

// mouse events

void PieChart::mouseMoveEvent(QMouseEvent *event)
{
    int idx = hitTestPie(event->pos());
    if (idx != m_hoveredIndex) {
        m_hoveredIndex = idx;
        update();
    }
}

void PieChart::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hoveredIndex = -1;
    update();
}

int PieChart::hitTestPie(const QPoint& pos) const
{
    for (int i = 0; i < m_slices.size(); ++i) {
        QPainterPath path;
        path.moveTo(m_lastPieRect.center());
        path.arcTo(QRectF(m_lastPieRect),
                   m_slices[i].startAngle / 16.0,
                   m_slices[i].spanAngle  / 16.0);
        path.closeSubpath();
        if (path.contains(pos)) return i;
    }
    return -1;
}

// drawing

void PieChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // rounded background
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
    for (auto it = m_data.begin(); it != m_data.end(); ++it)
        total += it.value();
    if (total <= 0) return;

    QList<QPair<QString, double>> sortedData;
    for (auto it = m_data.begin(); it != m_data.end(); ++it)
        sortedData.append(qMakePair(it.key(), it.value()));
    std::sort(sortedData.begin(), sortedData.end(),
              [](const QPair<QString, double>& a, const QPair<QString, double>& b) {
                  return a.second > b.second;
              });

    const int OFFSET = 12;
    int diameter = qMin(rect.width(), rect.height()) - 20 - OFFSET * 2;
    QRect basePieRect(
        rect.x() + (rect.width()  - diameter) / 2,
        rect.y() + (rect.height() - diameter) / 2,
        diameter, diameter
        );
    m_lastPieRect = basePieRect;
    m_slices.clear();

    int startAngle = 90 * 16;
    for (int i = 0; i < sortedData.size(); ++i) {
        double percentage = sortedData[i].second / total;
        int spanAngle = static_cast<int>(-percentage * 360 * 16);

        SliceInfo info;
        info.startAngle = startAngle;
        info.spanAngle  = spanAngle;
        info.name       = sortedData[i].first;
        info.value      = sortedData[i].second;
        info.percentage = percentage;
        info.color      = m_colors[i % m_colors.size()];
        m_slices.append(info);

        startAngle += spanAngle;
    }

    for (int pass = 0; pass < 2; ++pass) {
        for (int i = 0; i < m_slices.size(); ++i) {
            bool isHovered = (i == m_hoveredIndex);
            if (pass == 0 && isHovered)  continue;
            if (pass == 1 && !isHovered) continue;

            const SliceInfo& s = m_slices[i];

            QRect sliceRect = basePieRect;
            if (isHovered) {
                double midAngle = qDegreesToRadians((s.startAngle + s.spanAngle / 2) / 16.0);
                int dx = static_cast<int>(OFFSET * qCos(midAngle));
                int dy = static_cast<int>(-OFFSET * qSin(midAngle));
                sliceRect.translate(dx, dy);
            }

            QColor color = isHovered ? s.color.lighter(115) : s.color;
            painter.setBrush(color);
            painter.setPen(QPen(Qt::white, 2));
            painter.drawPie(sliceRect, static_cast<int>(s.startAngle),
                            static_cast<int>(s.spanAngle));

            if (s.percentage >= 0.05) {
                double midAngle = qDegreesToRadians((s.startAngle + s.spanAngle / 2) / 16.0);
                int labelRadius = diameter / 2 - 35;
                int labelX = sliceRect.center().x() + static_cast<int>(labelRadius * qCos(midAngle));
                int labelY = sliceRect.center().y() - static_cast<int>(labelRadius * qSin(midAngle));

                painter.setPen(Qt::white);
                QFont labelFont = font();
                labelFont.setPointSize(9);
                labelFont.setBold(true);
                painter.setFont(labelFont);
                QString label = QString("%1%").arg(s.percentage * 100, 0, 'f', 1);
                painter.drawText(QRect(labelX - 25, labelY - 10, 50, 20), Qt::AlignCenter, label);
            }
        }
    }

    if (m_hoveredIndex >= 0 && m_hoveredIndex < m_slices.size())
        drawTooltip(painter, m_slices[m_hoveredIndex]);
}

void PieChart::drawTooltip(QPainter& painter, const SliceInfo& slice)
{
    QString line1 = slice.name;
    QString line2 = QString("Сумма: %1").arg(slice.value, 0, 'f', 2);
    QString line3 = QString("Доля:  %1%").arg(slice.percentage * 100, 0, 'f', 1);

    QFont f = font();
    f.setPointSize(10);
    painter.setFont(f);
    QFontMetrics fm(f);

    const int padding = 10;
    int w = std::max({fm.horizontalAdvance(line1),
                      fm.horizontalAdvance(line2),
                      fm.horizontalAdvance(line3)}) + padding * 2 + 8;
    int h = fm.height() * 3 + padding * 2 + 6;

    int tx = m_lastPieRect.right() - w + 10;
    int ty = m_lastPieRect.top()   - 5;

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 30));
    painter.drawRoundedRect(tx + 3, ty + 3, w, h, 6, 6);

    painter.setBrush(QColor(44, 62, 80, 230));
    painter.drawRoundedRect(tx, ty, w, h, 6, 6);

    painter.setBrush(slice.color);
    painter.drawRoundedRect(tx, ty, 4, h, 2, 2);

    QFont boldF = f;
    boldF.setBold(true);
    painter.setFont(boldF);
    painter.setPen(Qt::white);
    painter.drawText(tx + padding, ty + padding, w, fm.height(), Qt::AlignLeft, line1);

    painter.setFont(f);
    painter.setPen(QColor(189, 195, 199));
    painter.drawText(tx + padding, ty + padding + fm.height() + 3,     w, fm.height(), Qt::AlignLeft, line2);
    painter.drawText(tx + padding, ty + padding + (fm.height() + 3)*2, w, fm.height(), Qt::AlignLeft, line3);
}

void PieChart::drawLegend(QPainter& painter, const QRect& rect)
{
    int y = rect.y() + 10;
    int colorIndex = 0;

    QFont legendFont = font();
    legendFont.setPointSize(9);
    painter.setFont(legendFont);
    QFontMetrics fm(legendFont);

    double total = 0;
    for (auto it = m_data.begin(); it != m_data.end(); ++it)
        total += it.value();

    QList<QPair<QString, double>> sortedData;
    for (auto it = m_data.begin(); it != m_data.end(); ++it)
        sortedData.append(qMakePair(it.key(), it.value()));
    std::sort(sortedData.begin(), sortedData.end(),
              [](const QPair<QString, double>& a, const QPair<QString, double>& b) {
                  return a.second > b.second;
              });

    for (int i = 0; i < sortedData.size(); ++i) {
        if (y > rect.bottom() - 20) break;

        const auto& item = sortedData[i];
        bool isHovered = (i == m_hoveredIndex);
        QColor color = m_colors[colorIndex % m_colors.size()];

        double percentage = (total > 0) ? (item.second / total * 100) : 0;
        QString fullText = QString("%1 - %2 (%3%)")
                               .arg(item.first)
                               .arg(item.second, 0, 'f', 2)
                               .arg(percentage, 0, 'f', 1);
        int textWidth = rect.width() - 22;
        QStringList lines;
        if (fm.horizontalAdvance(fullText) > textWidth) {
            QString shortName = item.first;
            if (shortName.length() > 15)
                shortName = shortName.left(12) + "...";
            lines << shortName
                  << QString("%1 (%2%)").arg(item.second, 0, 'f', 2).arg(percentage, 0, 'f', 1);
        } else {
            lines << fullText;
        }

        if (isHovered) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(color.red(), color.green(), color.blue(), 30));
            painter.drawRoundedRect(rect.x() - 4, y - 2, rect.width(),
                                    fm.height() * lines.size() + 4, 4, 4);
        }

        painter.setBrush(isHovered ? color.lighter(115) : color);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect.x(), y, 16, 16, 3, 3);

        painter.setPen(QColor("#2c3e50"));
        int currentY = y;
        for (const QString& line : lines) {
            if (currentY + fm.height() > rect.bottom()) break;
            QString displayLine = fm.elidedText(line, Qt::ElideRight, textWidth);
            painter.drawText(rect.x() + 22, currentY, textWidth, fm.height(),
                             Qt::AlignLeft | Qt::AlignVCenter, displayLine);
            currentY += fm.height();
        }

        y += fm.height() * lines.size() + 8;
        colorIndex++;
    }
}