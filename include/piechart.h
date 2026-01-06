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
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QMap<QString, double> m_data;
    QString m_title;
    QList<QColor> m_colors;
    
    void drawPie(QPainter& painter, const QRect& rect);
    void drawLegend(QPainter& painter, const QRect& rect);
};

#endif // PIECHART_H
