#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "jsonhandler.hpp"
#include "process.hpp"
#include "HttpRequest.hpp"
#include <windows.h>
#include <QThread>
#include "worker.hpp"
std::string AppData{}; // https://store.steampowered.com/api/appdetails?appids=[APPID]
std::vector<json> yeah{};
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    const char* env = getenv("LOCALAPPDATA");
    if (env == nullptr) {
        qDebug() << "uhhmmm okay :(";
        exit(2);
    }
    AppData = std::string(env);
    JsonHandler jH("D:\\SteamLibrary\\steamapps");
    yeah = jH.parseAllManifests();
    JsonHandler jHCDrive("C:\\Program Files (x86)\\Steam\\steamapps");
    for(const auto &i : jHCDrive.parseAllManifests()) {
        yeah.emplace_back(i);
    }
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}
json jsonObjForTest{};
void MainWindow::on_pushButton_clicked()
{

    if(yeah.size() < 1 ) {
        qDebug() << "tfff??";
        ui->textEdit->setPlainText("ja ka tbh HAHAHA");
    }
    else {
        for(const auto &json : yeah) {
            ui->textEdit->append(QString::fromStdString(json.dump(4)));
        }

    }



}


void MainWindow::on_pushButton_2_clicked()
{
    for(const auto &i : yeah) {
        ui->textEdit->append(QString::fromStdString(i["appid"].get<std::string>()));
    }
}


void MainWindow::on_pushButton_3_clicked()
{
    ui->textEdit->setText(QString::fromStdString(AppData));
}


void MainWindow::on_pushButton_4_clicked()
{
    bool worked = fs::create_directories(AppData + "\\SteamSort\\JSONS");
    if(worked) {
        ui->textEdit->setText("hölle ja " + QString::fromStdString(std::to_string(worked)));
    }
}


void MainWindow::on_pushButton_5_clicked()
{
    Process proc;
    proc.Exec("dir " + AppData + "\\SteamSort");
    ui->textEdit->setText(QString::fromStdString(proc.output));
}


void MainWindow::on_pushButton_6_clicked()
{
    std::string uri{"https://store.steampowered.com/api/appdetails?appids="};
    for (const auto &i : yeah) {
        QThread* thread = new QThread;
        Worker* worker = new Worker(uri, i);

        worker->moveToThread(thread);

        connect(thread, &QThread::started, worker, &Worker::process);
        connect(worker, &Worker::resultReady, this, [this](const QString& result){
            ui->textEdit->append(result);
        });
        connect(worker, &Worker::finished, thread, &QThread::quit);
        connect(worker, &Worker::finished, worker, &Worker::deleteLater);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        thread->start();
    }
}





