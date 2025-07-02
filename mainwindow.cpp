#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "jsonhandler.hpp"
#include "process.hpp"
#include "HttpRequest.hpp"
#include "jserv.hpp"
#include <windows.h>
#include <mutex>
#include "threader.hpp"
std::string AppData{};
std::string steamURL{"https://store.steampowered.com/api/appdetails?appids="};
std::vector<json> responses{};
std::mutex responseMutex{};
std::vector<json> yeah{};
json GamesJSON{};
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
std::string jsonPath = AppData + "\\SteamSort\\games.json";
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
    bool worked = fs::create_directories(AppData + "\\SteamSort");
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
    bool allReceived = false;
    responses.clear();
    ui->textEdit->clear();
    Threader t(THREAD_DETACH);
    for(const auto &element : yeah) {
        t.run<void>([element, this, &allReceived]() {
            std::string appID{element["appid"].get<std::string>()};
            HttpRequest h;
            h.SetURL(steamURL + appID);
            json res = h.JSONResponse();
            std::lock_guard<std::mutex> lock(responseMutex);
            responses.emplace_back(res);
            allReceived = (responses.size() == yeah.size());
            if(allReceived) {
                QMetaObject::invokeMethod(this, [this]() {
                    std::lock_guard<std::mutex> lock(responseMutex);
                    for(const auto &it : responses) {
                        ui->textEdit->append(QString::fromStdString(it.dump(4)));
                        JServ j;
                        j.buildGamesJSON(responses[0]);
                        ui->textEdit->setText(QString::fromStdString(j.exportJSON().dump(4)));
                    }
                }, Qt::QueuedConnection);
            }

        });
    }


}




