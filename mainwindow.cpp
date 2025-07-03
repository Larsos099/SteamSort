#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "jsonhandler.hpp"
#include "process.hpp"
#include "HttpRequest.hpp"
#include "jserv.hpp"
#include "Progresser.hpp"
#include "threader.hpp"
#include "jsonparser.hpp"
#include <windows.h>
#include <QStandardItemModel>
#include <sstream>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <mutex>
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }

    return result;
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , jServVector(new JServVector())
{
    ui->setupUi(this);

    // Your AppData, folder, and steamLibLocations initialization here (no heavy network calls!)
    const char* LocalAppD = getenv("LOCALAPPDATA");
    AppData = std::string(LocalAppD);
    if(!fs::exists(AppData + "\\SteamSort\\locations.json")) {
        QInputDialog qInDiag;
        QMessageBox qMsgBox;
        QString sllqs = qInDiag.getText(nullptr, "Steam Lib Locations", "Enter steam lib paths (multiple possible, split with ;");
        steamLibLocations = split(sllqs.toStdString(), ';');
        JSONParser p;
        std::ofstream jlf(AppData + "\\SteamSort\\locations.json");
        jlf << jServVector->buildLocationsJSON(steamLibLocations).dump(4);
    }
    else {
        std::ifstream jlfe(AppData + "\\SteamSort\\locations.json");
        std::string content((std::istreambuf_iterator<char>(jlfe)), std::istreambuf_iterator<char>());
        json yh = json::parse(content);
        steamLibLocations = yh["locations"].get<std::vector<std::string>>();
    }
    // Parse manifests etc. -- also fine here if synchronous:
    JsonHandlerVector jhv(steamLibLocations);
    jVec = jhv.parseAllManifests();

    // Now schedule the heavy network/thread processing to run after event loop starts:
    QTimer::singleShot(0, this, &MainWindow::startProcessing);
}

void MainWindow::startProcessing()
{
    responses.clear();

    if (!progresser) {
        progresser = new Progresser(300, 70, nullptr);
        progresser->setWindowModality(Qt::ApplicationModal);
        progresser->show();
        progresser->raise();
        progresser->setProgress(0);
    }

    std::atomic<int> doneCount{0};
    const int total = static_cast<int>(jVec.size());

    jServVector->clearJSON();

    connect(jServVector, &JServVector::progressUpdated, this, [this](int done, int total){
        int percent = total ? (done * 100 / total) : 0;
        progresser->setProgress(percent);
    });

    Threader t(THREAD_JOIN);  // Will block until threads finish

    for (const auto& element : jVec) {
        t.run<void>([element, this, &doneCount, total]() {
            std::string appID;

            if (element["appid"].is_string()) {
                appID = element["appid"].get<std::string>();
            } else if (element["appid"].is_number_integer()) {
                appID = std::to_string(element["appid"].get<int>());
            } else {
                qWarning() << "Unsupported appid type!";
                doneCount.fetch_add(1);
                emit jServVector->progressUpdated(doneCount.load(), total);
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
            emit jServVector->progressUpdated(doneCount.load(), total);
        });
    }
    jServVector->setVec(responses);
    jServVector->buildGamesJSON();

    std::string outPath = AppData + "\\SteamSort\\games.json";
    std::ofstream file(outPath);
    if (!file) {
        qWarning() << "Failed to open file for writing:" << QString::fromStdString(outPath);
    } else {
        file << jServVector->exportJSON().dump(4);
        file.close();
        qDebug() << "games.json successfully written to:" << QString::fromStdString(outPath);
    }

    QMessageBox::information(this, "built games.json", "Successfully built games.json:\n" + QString::fromStdString(jServVector->exportJSON().dump(4)));
}

MainWindow::~MainWindow()
{
    delete jServVector;
    delete ui;
    if (progresser) {
        delete progresser;
        progresser = nullptr;
    }
}
