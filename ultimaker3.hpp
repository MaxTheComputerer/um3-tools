#ifndef ULTIMAKER3_HEADER
#define ULTIMAKER3_HEADER

#include <string>
#include <cpr/cpr.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using std::cerr;
using std::cout;
using std::endl;
using std::string;

class Ultimaker3
{
    const string ip_address;
    string api_base_url;
    string id;
    string key;

    cpr::Response get_request(const string request)
    {
        return cpr::Get(cpr::Url{api_base_url + request});
    }

    cpr::Response get_request_authenticated(const string request)
    {
        if (!is_authenticated())
        {
            throw std::invalid_argument("You must provide authentication credentials to make this request.");
        }
        return cpr::Get(cpr::Url{api_base_url + request}, cpr::Authentication{id, key, cpr::AuthMode::DIGEST});
    }

    cpr::Response post_request(const string request, const string body)
    {
        return cpr::Post(cpr::Url{api_base_url + request}, cpr::Body{body}, cpr::Authentication{id, key, cpr::AuthMode::DIGEST}, cpr::Header{{"Content-Type", "application/json"}});
    }

public:
    Ultimaker3(const string ip_address) : ip_address(ip_address)
    {
        api_base_url = "http://" + ip_address + "/api/v1";
    }

    string status()
    {
        auto result = get_request("/printer/status");
        return json::parse(result.text);
    }

    bool authenticate(const string id, const string key)
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
        string body = "{\"frequency\": " + std::to_string(frequency) + ", \"duration\": " + std::to_string(duration) + "}";
        auto result = post_request("/printer/beep", body);
        return result.status_code == 204;
    }

    json head_position(const int head_id)
    {
        auto result = get_request("/printer/heads/" + std::to_string(head_id) + "/position");
        if (result.status_code != 200)
        {
            cerr << result.text << endl;
            throw std::runtime_error("Failed to get head position.");
        }
        return json::parse(result.text);
    }

    string take_snapshot()
    {
        auto result = cpr::Get(cpr::Url{"http://" + ip_address + ":8080"}, cpr::Parameters{{"action", "snapshot"}});
        if (result.status_code != 200)
        {
            cerr << result.text << endl;
            throw std::runtime_error("Failed to get head position.");
        }
        return result.text;
    }

    double progress()
    {
        if (status() != "printing")
        {
            return 1;
        }
        auto result = get_request("/print_job/progress");
        return std::stod(result.text);
    }

    string job_uuid()
    {
        if (status() != "printing")
        {
            return "";
        }
        auto text = get_request("/print_job/uuid").text;
        text.erase(remove(text.begin(), text.end(), '\"'), text.end());
        return text;
    }
};

#endif