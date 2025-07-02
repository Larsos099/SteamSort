#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <stdexcept>
#include <curl/curl.h>
#include "json.hpp"

using json = nlohmann::json;

class HttpRequest {
private:
    std::string uri{};
public:
    HttpRequest() = default;
    void SetURL(const std::string &url) {
        uri = url;
    }

    json JSONResponse() {
        CURL* curl = curl_easy_init();
        if (!curl) throw std::runtime_error("Curl init failed");

        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                         +[](void* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
                             std::string* str = static_cast<std::string*>(userdata);
                             str->append(static_cast<char*>(ptr), size * nmemb);
                             return size * nmemb;
                         });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            throw std::runtime_error(curl_easy_strerror(res));
        }

        return json::parse(response);
    }
};

#endif // HTTPREQUEST_HPP
