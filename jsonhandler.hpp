#ifndef JSONHANDLER_HPP
#define JSONHANDLER_HPP
#include "json.hpp"
#include <vector>
#include "AcfParser.hpp"
#include <string>
#include <filesystem>
#include <unordered_map>
using json = nlohmann::json;
namespace fs = std::filesystem;
class JsonHandler
{
private:
    std::string steamAppsPath{};
    AcfParser *a = new AcfParser();
public:
    JsonHandler() = default;
    JsonHandler(std::string steamapps) : steamAppsPath(steamapps) {}
    ~JsonHandler() {
        delete a;
    }
    std::vector<json> parseAllManifests();
    std::string getAppID(const json &appManifest);
    void setLibPath(const std::string &path) {
        steamAppsPath = path;
    }
};
class JsonHandlerVector {
private:
    std::vector<std::string> steamAppsPathsVectors{};
    JsonHandler j;
public:
    JsonHandlerVector() = default;
    JsonHandlerVector(std::vector<std::string> steamLibVec) : steamAppsPathsVectors(steamLibVec) {};
    std::vector<json> parseAllManifests();
};

#endif // JSONHANDLER_HPP
