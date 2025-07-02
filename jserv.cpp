#include "jserv.hpp"


void JServ::buildGamesJSON(const json &j) {

    std::string name = j["name"].get<std::string>();
    auto categories = j["categories"];
    auto genres = j["genres"];
    auto appid = j["appid"].get<std::string>();
    json catArray = json::array();
    for (auto& cat : categories) {
        catArray.push_back(cat["description"]);
    }

    json genreArray = json::array();
    for (auto& gen : genres) {
        genreArray.push_back(gen["description"]);
    }

    final["games"][name]["categories"] = catArray;
    final["games"][name]["genres"] = genreArray;
    final["games"][name]["appid"] = {appid};

}
json JServ::exportJSON() {
    return final;
}

void JServ::clearJSON() {
    final = json();
}
