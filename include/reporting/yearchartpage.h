#ifndef YEARCHARTPAGE_H
#define YEARCHARTPAGE_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>

class BarChart;

class YearChartPage : public QWidget
{
    Q_OBJECT

public:
    explicit YearChartPage(QWidget *parent = nullptr);

public slots:
    void refreshChart();

private slots:
    void onFilterChanged();

private:
    void setupUi();
    void updateChart();

    QComboBox *m_typeCombo;
    QSpinBox *m_yearSpin;
    BarChart *m_chart;
};

#endif // YEARCHARTPAGE_H
