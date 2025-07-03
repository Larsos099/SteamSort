    #include "jserv.hpp"
    #include <QDebug>
    #include "threader.hpp"
    void JServ::buildGamesJSON(const json &j) {
        if (j.empty()) {
            qDebug() << "JSON is empty";
            return;
        }

        auto it = j.begin();

        if (!it.value().contains("data")) {
            qDebug() << "No data field found";
            return;
        }

        json jn = it.value()["data"];
        std::string appid = std::to_string(jn["steam_appid"].get<int>());
        std::string name = jn["name"].get<std::string>();
        std::vector<std::string> categories{};
        std::vector<std::string> genres{};
        for (const auto &c : jn["categories"]) {
            if (c.contains("description"))
                categories.emplace_back(c["description"].get<std::string>());
        }
        for (const auto &g : jn["genres"]) {
            if (g.contains("description"))
                genres.emplace_back(g["description"].get<std::string>());
        }
        final["games"][name] = {
            {"appid", appid},
            {"genres", genres},
            {"categories", categories}
        };
    }
    json JServ::exportJSON() {
        return final;
    }

    void JServ::clearJSON() {
        final = json();
    }

    void JServVector::buildGamesJSON() {
        Threader t(THREAD_JOIN);
        Threader p(THREAD_DETACH);
        std::mutex finalMutex;
        doneCount.store(0);
        int total = static_cast<int>(jV.size());
        std::atomic<bool> progressRunning{true};

        p.run<void>([this, total, &progressRunning]() {
            while (progressRunning) {
                int done = doneCount.load();
                emit progressUpdated(done, total);
                if (done >= total) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            emit progressUpdated(total, total);
        });

        for (const auto& j : jV) {
            t.run<void>([&, j]() {
                if (j.empty()) {
                    qDebug() << "JSON is empty";
                    return;
                }

                auto it = j.begin();
                if (!it.value().contains("data")) {
                    qDebug() << "No data field found";
                    return;
                }

                json jn = it.value()["data"];
                std::string appid = std::to_string(jn["steam_appid"].get<int>());
                std::string name = jn["name"].get<std::string>();

                std::vector<std::string> categories;
                std::vector<std::string> genres;

                for (const auto& c : jn["categories"]) {
                    if (c.contains("description"))
                        categories.emplace_back(c["description"].get<std::string>());
                }

                for (const auto& g : jn["genres"]) {
                    if (g.contains("description"))
                        genres.emplace_back(g["description"].get<std::string>());
                }

                json gameEntry = {
                    {"appid", appid},
                    {"genres", genres},
                    {"categories", categories}
                };

                {
                    std::lock_guard<std::mutex> lock(finalMutex);
                    final["games"][name] = std::move(gameEntry);
                }

                doneCount.fetch_add(1);
            });
        }

        progressRunning = false; // Stop progress reporting thread when done
    }


    json JServVector::exportJSON() {
        return final;
    }
    void JServVector::clearJSON() {
        final = json();
    }
