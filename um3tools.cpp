#include <iostream>
#include <fstream>

#include "um3tools_config.h"
#include "ultimaker3.hpp"
#include "music.hpp"

using std::cout;
using std::endl;
using std::pair;
using std::string;

pair<string, string> load_auth_file(string path = "auth.txt")
{
    std::ifstream file(path);
    string username, password;
    if (!getline(file, username))
    {
        throw std::ifstream::failure("No username in file");
    }
    if (!getline(file, password))
    {
        throw std::ifstream::failure("No password in file");
    }
    return pair<string, string>{username, password};
}

int main()
{
    cout << endl
         << "===== Ultimaker 3 Tools v" << UM3_Tools_VERSION_MAJOR << "." << UM3_Tools_VERSION_MINOR << " =====" << endl
         << endl;

    auto printer = Ultimaker3("10.146.47.127");

    auto status = printer.status();
    cout << "Printer status: " << status << endl;

    auto credentials = load_auth_file("/home/max/um3-tools/auth.txt");
    cout << "Auth: " << printer.authenticate(credentials.first, credentials.second) << endl;

    MidiPlayer player(printer, "/home/max/um3-tools/data/imperialmarch.xml");
    player.play();

    return 0;
}
