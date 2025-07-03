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

private:
    Progresser *progresser = nullptr;
    JServVector* jServVector = nullptr;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
