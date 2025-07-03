#ifndef JSERV_HPP
#define JSERV_HPP

#include "json.hpp"
#include <vector>
#include <string>
#include <atomic>
#include <QObject>
#include <mutex>
#include <functional>

using json = nlohmann::json;

class JServ {
public:
    JServ() = default;
    void buildGamesJSON(const json &j);
    void clearJSON();
    json exportJSON();
private:
    json final{};
};

class JServVector : public QObject {
    Q_OBJECT

private:
    std::vector<json> jV;
    json final{};
    std::atomic<int> doneCount{0};
    std::function<void(int, int)> progressCallback;

public:
    explicit JServVector(std::vector<json> jsonV) : jV(std::move(jsonV)) {}

    JServVector() = default;
    json buildLocationsJSON(const std::vector<std::string> &loc);
    void buildGamesJSON();
    void clearJSON();
    json exportJSON();
    void setVec(std::vector<json> &v) {
        jV = v;
    }
    void setProgressCallback(std::function<void(int, int)> cb) {
        progressCallback = std::move(cb);
    }

signals:
    void progressUpdated(int done, int total);

};

#endif // JSERV_HPP
