#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Progresser.hpp"
#include "jserv.hpp"
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
    void startProcessing();  // new function for your heavy work

private:
    Ui::MainWindow *ui;
    JServVector *jServVector;
    Progresser *progresser = nullptr;

    std::vector<json> responses;          // move from local to member
    std::mutex responseMutex;             // keep this mutex as member

    std::string AppData;
    std::vector<json> jVec;
    std::vector<std::string> steamLibLocations;
    std::string steamURL = "https://store.steampowered.com/api/appdetails?appids=";
};
#endif // MAINWINDOW_H
