#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "jsonhandler.hpp"
#include "process.hpp"
#include "HttpRequest.hpp"
#include <windows.h>
#include <future>
#include "threader.hpp"
std::string AppData{};
std::string steamURL{"https://store.steampowered.com/api/appdetails?appids="};
std::vector<json> responses{};
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
std::string jsonPath = AppData + "\\SteamSort\\JSONS\\games.json";
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
    bool worked = fs::create_directories(jsonPath);
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
    if(!fs::exists(jsonPath)) {
        std::ofstream json_file(jsonPath);
        if(!json_file.is_open()) {
            qDebug() << "leck eier";
        }
        else {
            json_file.close();
        }
    }
    else {
        // Vector to hold futures for async calls
        std::vector<std::future<json>> futures;

        for(const auto &i : yeah) {
            // Launch async task, each with its own HttpRequest instance
            futures.push_back(std::async(std::launch::async, [this, i]() -> json {
                HttpRequest h;
                h.SetURL(steamURL + i["appid"].get<std::string>());
                json jn = h.JSONResponse();

                // Manipulate jn here safely
                jn["name"] = i["name"];  // Or whatever you want

                json categories = json::array();
                for (const auto& item : jn["categories"]) {
                    if (item.contains("description")) {
                        categories.push_back(item["description"]);
                    }
                }
                jn["categories"] = categories;

                return jn;
            }));
        }

        // Collect all results
        json allResults = json::array();
        for(auto &fut : futures) {
            allResults.push_back(fut.get());
        }

        // Write to file once, after all threads finish
        std::ofstream outFile(jsonPath);
        if(!outFile.is_open()) {
            qDebug() << "leck eier";
        }
        else {
            outFile << allResults.dump(4);
            outFile.close();
        }
    }

    // Now read file and put content into textEdit
    std::ifstream inStream(jsonPath);
    QString contents = QString::fromStdString(std::string((std::istreambuf_iterator<char>(inStream)),
                                                          std::istreambuf_iterator<char>()));
    ui->textEdit->setText(contents);

}




