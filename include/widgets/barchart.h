#ifndef BARCHART_H
#define BARCHART_H

#include <QWidget>
#include <QMap>
#include <QPainter>

class BarChart : public QWidget
{
    Q_OBJECT

public:
    explicit BarChart(QWidget *parent = nullptr);

    void setData(const QMap<int, double>& data);
    void setTitle(const QString& title);
    void setBarColor(const QColor& color);
    void setLabels(const QStringList& labels);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    struct BarInfo {
        QRect rect;
        QString label;
        double value;
    };

    void drawBars(QPainter& painter, const QRect& rect);
    void drawAxes(QPainter& painter, const QRect& rect, double maxValue);
    void drawTooltip(QPainter& painter, const BarInfo& bar);
    int  hitTestBar(const QPoint& pos) const;

    QMap<int, double> m_data;
    QString m_title;
    QColor m_barColor;
    QStringList m_labels;

    QList<BarInfo> m_bars;
    int m_hoveredIndex = -1;
};

#endif // BARCHART_H