#include "jserv.hpp"
#include <QDebug>

void JServ::buildGamesJSON(const json &j) {
    qDebug() << j.dump(4);
    std::string appid = j["steam_appid"].get<std::string>();
    std::string gameName = j["name"].get<std::string>();
    std::vector<std::string> cDes{};
    std::vector<std::string> gDes{};
    for(const auto &c : j["categories"]) {
        cDes.emplace_back(c["description"]);
    }
    for(const auto &g : j["genres"]) {
        gDes.emplace_back(g["description"]);
    }
    json js;
    js["games"][gameName] = {
        {"appid", appid},
        {"categories", cDes},
        {"genres", gDes}
    };
    final = std::move(js);

}
json JServ::exportJSON() {
    return final;
}

void JServ::clearJSON() {
    final = json();
}
