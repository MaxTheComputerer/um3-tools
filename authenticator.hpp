#ifndef UM3_AUTHENTICATOR_HEADER
#define UM3_AUTHENTICATOR_HEADER

#include "ultimaker3.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

class Authenticator
{
    Ultimaker3 printer;

public:
    string id, key;

    Authenticator(Ultimaker3 printer) : printer(printer)
    {
    }

    bool authorize(string username, string application)
    {
        cout << "Requesting authorization..." << endl;
        auto credentials = printer.request_authorization(username, application);
        id = credentials["id"];
        key = credentials["key"];

        cout << "Confirm authorization on the printer:" << endl << "Application: " << application << endl << "User: " << username << endl;
        string check;
        auto counter = 0;
        while ((check = printer.check_authorization_progress(id)) == "unknown" && counter < 120)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            counter++;
        }

        if (counter >= 120)
        {
            cerr << "Timeout waiting for authorization." << endl;
            return false;
        }
        if (check == "unauthorized")
        {
            cerr << "Authorization declined." << endl;
            return false;
        }
        else if (check == "authorized" && printer.authenticate(id, key))
        {
            cout << "Successfully authenticated to " << printer.ip_address << endl;
            return true;
        }
        else
        {
            throw std::runtime_error("Unknown response from printer authorization.");
        }
    }

    void save_credentials(string out_path)
    {
        if (id == "" || key == "")
        {
            cerr << "No credentials have been created." << endl;
            return;
        }

        std::ofstream file(out_path);
        file << id << endl << key;

        if (file.good())
        {
            cout << "Credentials saved to " << out_path << endl;
        }
        else
        {
            cerr << "Error writing credentials." << endl;
        }
        
    }
};


#endif