#include "mainwindow.h"
#include "api_handle.h"
#include "./ui_mainwindow.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <fstream>


QJsonDocument MainWindow::ObjectFromString(const QString& in)
{
    QJsonObject obj;

    QJsonDocument doc = QJsonDocument::fromJson(in.toLocal8Bit());

    // check validity of the document
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            obj = doc.object();
        }
        else
        {
            qDebug() << "Document is not an object";
        }
    }
    else
    {
        qDebug() << "Invalid JSON...\n" << in;
    }

    return doc;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->saveButton->blockSignals(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_updateMainDatabase_clicked()  //update
{
    ui->saveButton->blockSignals(true);
    QString url = "https://api.gios.gov.pl/pjp-api/v1/rest/station/findAll?size=500&sort=Id";
    if (!isNetworkAvailable()) {
        QMessageBox msgbox;
        msgbox.setText("Brak połączenia sieciowego"); // tekst do okienka
        msgbox.exec();
    }
    else {
        ui->ListOfStations->clear();
        QString result;
        apiRequest(url,&result);
        result.remove(0,371);
        Json ::Value root;
        parseJsonResponse(result.toStdString(),root);
        QTreeWidgetItem *localizationItem = nullptr;
        for(auto iter = root.begin();iter!=root.end();++iter)
        {
            QString city,id;
            city = QString(root[iter.index()]["Nazwa miasta"].asCString());
            id = QString::number(root[iter.index()]["Identyfikator stacji"].asInt());
            localizationItem = new QTreeWidgetItem(ui->ListOfStations);
            localizationItem->setText(0,id);
            localizationItem->setText(1,city);
            ui->ListOfStations->addTopLevelItem(localizationItem);
        }
    }
}


void MainWindow::on_ListOfStations_itemClicked(QTreeWidgetItem *item, int column)   //wybór stacji
{
    ui->saveButton->blockSignals(true);
    ui->ListOfSensor->clear();
    SelectedStationId = item->text(0);
    SelectedStationName = item->text(1);
    QString result,url = "https://api.gios.gov.pl/pjp-api/v1/rest/station/sensors/"+ SelectedStationId + "?size=500";
    apiRequest(url,&result);
    result.remove(0,393);
    Json::Value root;
    parseJsonResponse(result.toStdString(),root);
    QTreeWidgetItem *sensorItem = nullptr;
    for(auto iter = root.begin();iter!=root.end();++iter)
    {
        QString sensorId,sensorName;
        sensorId = QString::number(root[iter.index()]["Identyfikator stanowiska"].asInt());
        sensorName = QString(root[iter.index()]["Wskaźnik"].asCString());
        sensorItem = new QTreeWidgetItem(ui->ListOfSensor);
        sensorItem->setText(0,sensorId);
        sensorItem->setText(1,sensorName);
        ui->ListOfSensor->addTopLevelItem(sensorItem);
    }
}


void MainWindow::on_ListOfSensor_itemClicked(QTreeWidgetItem *item, int column) //wybór sensora
{
    int selection;
    SelectedSensorId = item->text(0);
    SelectedSensorName = item->text(1);
    QMessageBox selectionBox;
    selectionBox.addButton("Last 3 Days",QMessageBox::ActionRole); //2
    selectionBox.addButton("Last Month",QMessageBox::ActionRole);//3
    selectionBox.addButton("Last Year",QMessageBox::ActionRole); //4
    selectionBox.addButton("Cancel",QMessageBox::ActionRole); //5
    selection = selectionBox.exec();
    qDebug()<<selection;
    Json::Value root;
    switch(selection)
    {
        case 2:
        { //pobieranie danych z 3 dni
            QString result;
            QString url = "https://api.gios.gov.pl/pjp-api/v1/rest/data/getData/" + QString(item->text(0)) + "?size=500&sort=Data";
            apiRequest(url,&result);
            result.remove(0,346);
            parseJsonResponse(result.toStdString(),root);
            break;
        }
        case 3:
        { //pobieranie danych z tygodnia
            QString result;
            QString url = "https://api.gios.gov.pl/pjp-api/v1/rest/archivalData/getDataBySensor/" + QString(item->text(0)) + "?size=500&sort=Data&dayNumber=7";
            apiRequest(url,&result);
            result.remove(0,383);
            parseJsonResponse(result.toStdString(),root);
            break;
        }
        case 4:
        { //pobieranie danych z roku
            QString result;
            QString url = "https://api.gios.gov.pl/pjp-api/v1/rest/archivalData/getDataBySensor/" + QString(item->text(0)) + "?size=10000&sort=Data&dayNumber=365";
            apiRequest(url,&result);
            result.remove(0,383);
            parseJsonResponse(result.toStdString(),root);
            break;
        }
        case 5:
        { //cancel
            return;
            break;
        }
    }

    vector<QString>vectorOfDatesResult;
    vector<double>vectorOfValues;
    for(auto iter = root.begin();iter!=root.end();++iter)
    {
        try {
            QString date = root[iter.index()]["Data"].toStyledString().c_str();
            date = date.slice(1,13);
            vectorOfDatesResult.push_back(date);
        } catch (...) {
            continue;
        }
        try {
            vectorOfValues.push_back(root[iter.index()]["Wartość"].asDouble());
        } catch (...) {
            vectorOfValues.push_back(0);
        }
    }
    displayWidget = new DateTimeAxisWidget(ui->areaForChart);
    displayWidget->myWidget(vectorOfDatesResult,vectorOfValues);
    displayWidget->resize(ui->areaForChart->size());
    displayWidget->setVisible(true);
    toSaveValues = vectorOfValues;
    toSaveDates = vectorOfDatesResult;
    ui->saveButton->blockSignals(false);
}

void MainWindow::prepareDataForChart(std::vector<QString>*vectorOfDatesResult, std::vector<double>*meanValuesForCorespondingDates)
{
    /*
    auto root = getDataFromThisSensor.result();
    for(auto iter = root.begin();iter!=root.end();++iter)
    {
        try {
            QString date = root[iter.index()]["Data"].toStyledString().c_str();
            date = date.slice(1,13);
            vectorOfDatesResult->push_back(date);
        } catch (...) {
            continue;
        }
        try {
            meanValuesForCorespondingDates->push_back(root[iter.index()]["Wartość"].asDouble());
        } catch (...) {
            meanValuesForCorespondingDates->push_back(0);
        }
    }*/
}

void MainWindow::on_saveButton_clicked() //save
{

    std::string home{std::getenv("HOME")};
    std::ofstream fs{home + "/Documents/" + SelectedSensorId.toStdString() + ".json"};

    qDebug()<<"SAVING";
    //ofstream file;
    QString url = SelectedSensorId+".json";


    //fs.open("save.json");
    for(int i = 0;i<toSaveDates.size();++i)
    {
        fs<<toSaveDates[i].toStdString()<<" "<<toSaveValues[i]<<"\n";
    }
    fs.close();
}

