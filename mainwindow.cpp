#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "jsonhandler.hpp"
#include "process.hpp"
#include "HttpRequest.hpp"
#include "jserv.hpp"
#include "Progresser.hpp"
#include "threader.hpp"
#include <windows.h>
#include <mutex>
std::string AppData{};
std::string steamURL{"https://store.steampowered.com/api/appdetails?appids="};
std::vector<json> responses{};
std::mutex responseMutex{};
std::mutex jsonMutex{};
std::vector<json> yeah{};
json GamesJSON{};
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , jServVector(new JServVector())  // initialize here
{
    const char* env = getenv("LOCALAPPDATA");
    if (env == nullptr) {
        qDebug() << "uhhmmm okay :(";
        exit(2);
    }
    AppData = std::string(env);
    JsonHandler jH("C:\\Program Files (x86)\\Steam\\steamapps");
    yeah = jH.parseAllManifests();
    ui->setupUi(this);

}
std::string jsonPath = AppData + "\\SteamSort\\games.json";
MainWindow::~MainWindow()
{
    delete jServVector;  // free it on destruction
    delete ui;
    if (progresser) {
        delete progresser;
        progresser = nullptr;
    }
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
    responses.clear();
    ui->textEdit->clear();

    // Show progress bar or create it if not existing
    if (!progresser) {
        progresser = new Progresser(300, 70, nullptr);
        progresser->setWindowModality(Qt::ApplicationModal);
        progresser->show();
        progresser->raise();
        progresser->setProgress(0);
    }

    std::atomic<int> doneCount{0};
    const int total = static_cast<int>(yeah.size());

    // Clear previous data in jServVector before new run
    jServVector->clearJSON();
    // We'll set responses after collecting

    // Connect signal for progress update once
    connect(jServVector, &JServVector::progressUpdated, this, [this](int done, int total){
        int percent = total ? (done * 100 / total) : 0;
        progresser->setProgress(percent);
    });


    Threader t(THREAD_JOIN);  // Join to wait for threads to finish

    // Mutex to protect responses vector
    std::mutex responseMutex;

    for (const auto &element : yeah) {
        t.run<void>([this, &element, &responseMutex, &doneCount, total]() {
            std::string appID;

            // get appid safely
            if (element["appid"].is_string()) {
                appID = element["appid"].get<std::string>();
            } else if (element["appid"].is_number_integer()) {
                appID = std::to_string(element["appid"].get<int>());
            } else {
                qWarning() << "Invalid appid type!";
                doneCount.fetch_add(1);
                return;
            }

            HttpRequest h;
            h.SetURL(steamURL + appID);
            json res = h.JSONResponse();

            {
                std::lock_guard<std::mutex> lock(responseMutex);
                responses.emplace_back(std::move(res));
            }

            doneCount.fetch_add(1);
        });
    }

    // All threads joined here (THREAD_JOIN)
    // Now update UI on main thread

    QMetaObject::invokeMethod(this, [this]() {
        ui->textEdit->clear();
        for (const auto &resp : responses) {  // accesses global directly
            ui->textEdit->append(QString::fromStdString(resp.dump(4)));
        }
        jServVector->setVec(responses);
        jServVector->buildGamesJSON();
        qDebug() << QString::fromStdString(jServVector->exportJSON().dump(4));
        ui->textEdit->setText(QString::fromStdString(jServVector->exportJSON().dump(4)));
    }, Qt::QueuedConnection);

}





