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
    JsonHandler(std::string steamapps) : steamAppsPath(steamapps) {}
    ~JsonHandler() {
        delete a;
    }
    std::unordered_map<std::string, std::string> readGameInfo(const std::string &gameName);
    std::vector<json> parseAllManifests();
    std::string getAppID(const json &appManifest);
};

#endif // JSONHANDLER_HPP
