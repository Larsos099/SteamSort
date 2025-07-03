#ifndef JSONPARSER_HPP
#define JSONPARSER_HPP
#include <QDebug>
#include "json.hpp"
using json = nlohmann::json;
struct GameData {
    std::string name;
    std::string appid;
    std::vector<std::string> categories;
    std::vector<std::string> genres;
};

class JSONParser
{
public:
    JSONParser();
    std::vector<GameData> parseJSON(const json& j) {
        std::vector<GameData> result;

        if (j.empty()) {
            qDebug() << "Empty JSON";
            return result;
        }
        if (!j.contains("games")) {
            qDebug() << "Invalid JSON";
            return result;
        }

        qDebug() << QString::fromStdString(j.dump(4));

        for (auto& [gameName, gameData] : j["games"].items()) {
            GameData gd;
            gd.name = gameName;

            // appid auslesen
            if (gameData.contains("appid") && gameData["appid"].is_string()) {
                gd.appid = gameData["appid"].get<std::string>();
            }

            if (gameData.contains("categories") && gameData["categories"].is_array()) {
                for (const auto& category : gameData["categories"]) {
                    if (category.is_string())
                        gd.categories.push_back(category.get<std::string>());
                }
            }

            if (gameData.contains("genres") && gameData["genres"].is_array()) {
                for (const auto& genre : gameData["genres"]) {
                    if (genre.is_string())
                        gd.genres.push_back(genre.get<std::string>());
                }
            }

            result.push_back(std::move(gd));
        }

        return result;
    }



};

#endif // JSONPARSER_HPP
