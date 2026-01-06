#ifndef MONTHCHARTPAGE_H
#define MONTHCHARTPAGE_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>

class PieChart;

class MonthChartPage : public QWidget
{
    Q_OBJECT

public:
    explicit MonthChartPage(QWidget *parent = nullptr);

public slots:
    void refreshChart();

private slots:
    void onFilterChanged();
    void onPeriodTypeChanged();

private:
    void setupUi();
    void updateChart();

    QComboBox *m_periodTypeCombo;
    QLabel *m_monthLabel;
    QComboBox *m_monthCombo;
    QSpinBox *m_yearSpin;
    PieChart *m_chart;
};

#endif // MONTHCHARTPAGE_H
