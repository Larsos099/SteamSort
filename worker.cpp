#include "worker.hpp"
#include "HttpRequest.hpp"

Worker::Worker(const std::string& uri_, const json& i_, QObject* parent)
    : QObject(parent), uri(uri_), i(i_) {}

void Worker::process() {
    HttpRequest hR;
    hR.SetURL(uri + i["appid"].get<std::string>());
    QString jsonStr = QString::fromStdString(hR.JSONResponse().dump(4));
    emit resultReady(jsonStr);
    emit finished();
}
