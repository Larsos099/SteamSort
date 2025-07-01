#pragma once
#include <QObject>
#include <QString>
#include "json.hpp"

using json = nlohmann::json;

class Worker : public QObject {
    Q_OBJECT
public:
    Worker(const std::string& uri_, const json& i_, QObject* parent = nullptr);
public slots:
    void process();

signals:
    void resultReady(const QString& result);
    void finished();

private:
    std::string uri;
    json i;
};
