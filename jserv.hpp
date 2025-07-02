#ifndef JSERV_HPP
#define JSERV_HPP
#include "json.hpp"
#include <vector>
#include <string>
using json = nlohmann::json;
class JServ {
public:
    JServ() = default;
    void buildGamesJSON(const json &j);
    void rebuildJSON();
    void clearJSON();
    json exportJSON();
private:
    json final{};
};

class JServVector{
private:
    std::vector<json> jV;
    json final{};
public:
    JServVector(std::vector<json> &jsonV) : jV(std::move(jsonV)) {}
    void rebuildJSON();
    void clearJSON();
    json exportJSON();
    void buildGamesJSON();
};

#endif // JSERV_HPP
