#ifndef PIECHART_H
#define PIECHART_H

#include <QWidget>
#include <QMap>
#include <QPainter>

class PieChart : public QWidget
{
    Q_OBJECT

public:
    explicit PieChart(QWidget *parent = nullptr);

    void setData(const QMap<QString, double>& data);
    void setTitle(const QString& title);
    void setNoDataMessage(const QString& message);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    struct SliceInfo {
        double startAngle;
        double spanAngle;
        QString name;
        double value;
        double percentage;
        QColor color;
    };

    void drawPie(QPainter& painter, const QRect& rect);
    void drawLegend(QPainter& painter, const QRect& rect);
    void drawTooltip(QPainter& painter, const SliceInfo& slice);
    int  hitTestPie(const QPoint& pos) const;

    QMap<QString, double> m_data;
    QString m_title;
    QString m_noDataMessage = "Нет данных за выбранный период";
    QList<QColor> m_colors;

    QList<SliceInfo> m_slices;
    QRect m_lastPieRect;
    int m_hoveredIndex = -1;
};

#endif // PIECHART_H