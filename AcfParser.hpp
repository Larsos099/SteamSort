#ifndef ACFPARSER_HPP
#define ACFPARSER_HPP
#include <fstream>
#include <regex>
#include <QDebug>
#include "json.hpp"
class AcfParser {
public:

AcfParser() = default;
nlohmann::json parseAcf(const std::filesystem::path& path) {
    std::ifstream in(path);
    if(!in.is_open()) {
        qDebug() << "tf? es is nd offen";
    }
    nlohmann::json result;

    std::string line;
    std::regex kv_regex("\"([^\"]+)\"\\s+\"([^\"]+)\"");
    std::smatch match;

    while (std::getline(in, line)) {
        if (std::regex_search(line, match, kv_regex)) {
            std::string key = match[1].str();
            std::string value = match[2].str();
            result[key] = value;
        }
    }

    return result;
}
};

#endif // ACFPARSER_HPP
