#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <QtWidgets/qtreewidget.h>
#include "datetimeaxiswidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_updateMainDatabase_clicked();

    void on_ListOfStations_itemClicked(QTreeWidgetItem *item, int column);

    void on_ListOfSensor_itemClicked(QTreeWidgetItem *item, int column);

    void on_saveButton_clicked();

private:
    Ui::MainWindow *ui;
    QString SelectedStationId,SelectedStationName;
    QString SelectedSensorId,SelectedSensorName;
    QJsonDocument ObjectFromString(const QString& in);
    DateTimeAxisWidget * displayWidget = nullptr;
    void prepareDataForChart(std::vector<QString>*vectorOfDatesResult, std::vector<double>*meanValuesForCorespondingDates);
    std::vector<double> toSaveValues;
    std::vector<QString> toSaveDates;
};

#endif // MAINWINDOW_H
