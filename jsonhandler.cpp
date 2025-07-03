#include "jsonhandler.hpp"



std::vector<json> JsonHandler::parseAllManifests() {
    std::vector<json> result{};
    for (const auto& file : std::filesystem::directory_iterator(steamAppsPath)) {
        if (file.path().filename().string().starts_with("appmanifest_")) {
            auto gameData = a->parseAcf(file.path());
            result.emplace_back(gameData);
        }
    }
    return result;
}

std::string JsonHandler::getAppID(const json &appManifest) {
    return appManifest["appid"].get<std::string>();
}

std::vector<json> JsonHandlerVector::parseAllManifests() {
    std::vector<json> temp{};
    for(const auto &e : steamAppsPathsVectors) {
        j.setLibPath(e);
        temp.insert(temp.end(), j.parseAllManifests());
    }
    return temp;
}
