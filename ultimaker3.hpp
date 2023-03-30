#ifndef ULTIMAKER3_HEADER
#define ULTIMAKER3_HEADER

#include <string>
#include <cpr/cpr.h>
#include <iostream>

class Ultimaker3
{
    const std::string ip_address;
    std::string api_base_url;
    std::string id;
    std::string key;

    cpr::Response get_request(const std::string request)
    {
        return cpr::Get(cpr::Url{api_base_url + request});
    }

    cpr::Response get_request_authenticated(const std::string request)
    {
        if (!is_authenticated())
        {
            throw std::invalid_argument("You must provide authentication credentials to make this request.");
        }
        return cpr::Get(cpr::Url{api_base_url + request}, cpr::Authentication{id, key, cpr::AuthMode::DIGEST});
    }

    cpr::Response post_request(const std::string request, const std::string body)
    {
        return cpr::Post(cpr::Url{api_base_url + request}, cpr::Body{body}, cpr::Authentication{id, key, cpr::AuthMode::DIGEST}, cpr::Header{{"Content-Type", "application/json"}});
    }

public:
    Ultimaker3(const std::string ip_address) : ip_address(ip_address)
    {
        api_base_url = "http://" + ip_address + "/api/v1";
    }

    std::string status()
    {
        return get_request("/printer/status").text;
    }

    bool authenticate(const std::string id, const std::string key)
    {
        this->id = id;
        this->key = key;
        return get_request_authenticated("/auth/verify").status_code == 200;
    }

    bool is_authenticated()
    {
        return id != "" && key != "";
    }

    bool beep(const double frequency, const double duration)
    {
        std::string body = "{\"frequency\": " + std::to_string(frequency) + ", \"duration\": " + std::to_string(duration) + "}";
        auto result = post_request("/printer/beep", body);
        return result.status_code == 204;
    }
};

#endif