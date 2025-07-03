#ifndef PROGRESSER_H
#define PROGRESSER_H
#include <QWidget>
#include <QProgressBar>
#include <QVBoxLayout>
class Progresser : public QWidget {
    Q_OBJECT
public:
    explicit Progresser(int w = 300, int h = 70, QWidget* parent = nullptr) : QWidget(parent), height(h), width(w) {
        setWindowTitle("Parsing Progress");
        resize(width, height);
        progressBar = new QProgressBar(this);
        progressBar->setRange(0, 100);
        auto layout = new QVBoxLayout(this);
        layout->addWidget(progressBar);
        setLayout(layout);
    }

public slots:
    void setProgress(int percent) {
        progressBar->setValue(percent);
    }

private:
    int height;
    int width;
    QProgressBar* progressBar;
};

#endif // PROGRESSER_H
