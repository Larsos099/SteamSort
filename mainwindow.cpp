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
std::vector<std::string> steamLibLocations{};
std::vector<json> jVec{};
std::string AppData{};
std::string steamURL{"https://store.steampowered.com/api/appdetails?appids="};

std::mutex responseMutex{};
std::mutex jsonMutex{};
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , jServVector(new JServVector())
{
    const char* env = getenv("LOCALAPPDATA");
    if (!env) {
        throw std::runtime_error("LOCALAPPDATA not set");
    }
    AppData = env; // 🛠️ FIX: AppData korrekt setzen
    std::string folderLocation = AppData + "\\SteamSort";
    std::string locationsFilePath = folderLocation + "\\locations.json";

    if (!fs::exists(folderLocation)) {
        fs::create_directories(folderLocation);
    }

    std::vector<std::string> paths;

    if (!fs::exists(locationsFilePath)) {
        QString qLocation = QInputDialog::getText(nullptr, "Steam Library location",
                                                  "Location of your steam library: (multiple possible, separate with ;)");
        if (qLocation.isEmpty()) {
            QMessageBox::critical(nullptr, "Error", "You must enter at least one Steam library path.");
            throw std::runtime_error("No Steam library location provided.");
        }

        paths = split(qLocation.toStdString(), ';');

        JServVector jsV;
        json locJson = jsV.buildLocationsJSON(paths);

        std::ofstream locationsJson(locationsFilePath);
        locationsJson << locJson.dump(4);
        locationsJson.close();
    } else {
        std::ifstream locationsFile(locationsFilePath);
        std::string content((std::istreambuf_iterator<char>(locationsFile)), std::istreambuf_iterator<char>());

        try {
            json jljf = json::parse(content);
            paths = jljf["locations"].get<std::vector<std::string>>();
        } catch (const json::parse_error& e) {
            QMessageBox::critical(nullptr, "JSON Error", "Could not parse locations.json:\n" + QString::fromStdString(e.what()));
            throw;
        }
    }

    steamLibLocations = paths;

    JsonHandlerVector jhv(paths);
    jVec = jhv.parseAllManifests();
    std::vector<json> responses;
    std::atomic<int> doneCount{0};
    const int total = static_cast<int>(jVec.size());

    jServVector->clearJSON();

    if (!progresser) {
        progresser = new Progresser(300, 70, nullptr);
        progresser->setWindowModality(Qt::ApplicationModal);
        progresser->show();
        progresser->raise();
        progresser->setProgress(0);
    }

    connect(jServVector, &JServVector::progressUpdated, this, [this](int done, int total){
        int percent = total ? (done * 100 / total) : 0;
        progresser->setProgress(percent);
    });

    Threader t(THREAD_JOIN);

    for (const auto& element : jVec) {
        t.run<void>([&, element]() {
            std::string appID;

            if (!element.contains("appid") || element["appid"].is_null()) {
                qWarning() << "Invalid or missing appid in element!";
                doneCount.fetch_add(1);
                emit jServVector->progressUpdated(doneCount.load(), total);
                return;
            }

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

    // Main thread waits implicitly due to THREAD_JOIN policy
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

    QMessageBox::information(nullptr, "built games.json", "Successfully built games.json:\n" + QString::fromStdString(jServVector->exportJSON().dump(4)));
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


