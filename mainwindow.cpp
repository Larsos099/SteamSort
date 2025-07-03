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
    responses.clear();
    ui->textEdit->clear();
    if (!progresser) {
        progresser = new Progresser(300, 70, nullptr);  // nullptr parent means top-level window
        progresser->setWindowModality(Qt::ApplicationModal); // Blocks input to other windows if you want
        progresser->show();
        progresser->raise();
        progresser->setProgress(0);
    }
    std::mutex responseMutex;
    std::atomic<int> doneCount{0};
    int total = static_cast<int>(yeah.size());

    // Create JServVector with empty vector; we'll fill later
    auto jServVector = new JServVector();

    // Connect progressUpdated signal to your UI progress widget
    connect(jServVector, &JServVector::progressUpdated, this, [this](int done, int total){
        int percent = total ? (done * 100 / total) : 0;
        // Example with QProgressBar
        progresser->setProgress(percent);
    });

    Threader t(THREAD_DETACH);

    for (const auto& element : yeah) {
        t.run<void>([this, &element, &responseMutex, &doneCount, total, jServVector]() {
            std::string appID;
            if (element["appid"].is_string()) {
                appID = element["appid"].get<std::string>();
            } else if (element["appid"].is_number_integer()) {
                appID = std::to_string(element["appid"].get<int>());
            } else {
                qWarning() << "Invalid appid type!";
                return;
            }

            HttpRequest h;
            h.SetURL(steamURL + appID);
            json res = h.JSONResponse();

            {
                std::lock_guard<std::mutex> lock(responseMutex);
                responses.emplace_back(res);
            }

            int done = ++doneCount;

            // Once all responses are collected, update UI and start processing JSON
            if (done == total) {
                // Update vector inside JServVector *safely* from UI thread
                QMetaObject::invokeMethod(this, [this, jServVector, &responseMutex]() {
                    {
                        // Lock while copying responses to avoid data race
                        std::lock_guard<std::mutex> lock(responseMutex);
                        jServVector->setVec(responses); // Directly assign vector here
                    }

                    // Show responses in textEdit
                    ui->textEdit->clear();
                    for (const auto& resp : responses) {
                        ui->textEdit->append(QString::fromStdString(resp.dump(4)));
                    }

                    // Start building JSON with progress updates
                    jServVector->buildGamesJSON();

                    // Optional: print final JSON to debug
                    qDebug() << QString::fromStdString(jServVector->exportJSON().dump(4));
                }, Qt::QueuedConnection);
            }
        });
    }
}




