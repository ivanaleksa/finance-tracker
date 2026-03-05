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

private:
    QMap<int, double> m_data;
    QString m_title;
    QColor m_barColor;
    QStringList m_labels;
    
    void drawBars(QPainter& painter, const QRect& rect);
    void drawAxes(QPainter& painter, const QRect& rect, double maxValue);
};

#endif // BARCHART_H
