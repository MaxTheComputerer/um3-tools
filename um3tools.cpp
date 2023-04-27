#include <iostream>
#include <fstream>
#include <argparse/argparse.hpp>

#include "um3tools_config.h"
#include "ultimaker3.hpp"
#include "music.hpp"
#include "timelapse.hpp"
#include "authenticator.hpp"

using std::cerr;
using std::cout;
using std::endl;
using std::pair;
using std::string;

pair<string, string> load_auth_file(string path = "auth.txt")
{
    std::ifstream file(path);
    string id, key;
    if (!getline(file, id))
    {
        throw std::ifstream::failure("No ID in file");
    }
    if (!getline(file, key))
    {
        throw std::ifstream::failure("No key in file");
    }
    return pair<string, string>{id, key};
}

Ultimaker3 connect_to_printer(string ip)
{
    cout << "Connecting to " << ip << endl;
    auto printer = Ultimaker3(ip);

    auto status = printer.status();
    cout << "Printer status: " << status << endl;

    return printer;
}

void timelapse(argparse::ArgumentParser parser)
{
    auto ip = parser.get<string>("printer-address");
    auto out_path = parser.present("--out-file");
    auto frame_rate = parser.get<int>("--frame-rate");

    auto printer = connect_to_printer(ip);

    Timelapse timelapse(printer);
    if (out_path.has_value())
    {
        timelapse.start(frame_rate, out_path.value());
    }
    else
    {
        timelapse.start(frame_rate);
    }
}

void music(argparse::ArgumentParser parser, string id, string key)
{
    auto ip = parser.get<string>("printer-address");
    auto xml_path = parser.get<string>("xml-path");
    
    auto printer = connect_to_printer(ip);

    auto credentials = load_auth_file("/home/max/um3-tools/auth.txt");
    cout << "Authenticated: " << std::boolalpha << printer.authenticate(credentials.first, credentials.second) << endl;

    MusicXmlPlayer player(printer, xml_path);
    player.play();
}

void authenticate(argparse::ArgumentParser parser)
{
    auto ip = parser.get<string>("printer-address");
    auto username = parser.get<string>("username");
    auto out_path = parser.present("--output");

    auto printer = connect_to_printer(ip);

    Authenticator auth(printer);

    if (auth.authorize(username, "um3tools"))
    {
        cout << "ID: " << auth.id << endl;
        cout << "Key: " << auth.key << endl;

        if (out_path.has_value())
        {
            auth.save_credentials(out_path.value());
        }
    }
}

int main(int argc, char *argv[])
{
    cout << endl
         << "===== Ultimaker 3 Tools v" << um3tools_VERSION_MAJOR << "." << um3tools_VERSION_MINOR << " =====" << endl
         << endl;

    argparse::ArgumentParser parser("um3tools", um3tools_VERSION_MAJOR + "." + um3tools_VERSION_MINOR);


    argparse::ArgumentParser timelapse_tool("timelapse");
    timelapse_tool.add_description("Records a timelapse of the current print job. Requires ffmpeg to be installed.");
    timelapse_tool.add_argument("printer-address")
        .help("IP or network address of the printer.");
    timelapse_tool.add_argument("--out-file")
        .help("Path to timelapse video output file.")
        .metavar("PATH");
    timelapse_tool.add_argument("-r", "--frame-rate")
        .help("Frame rate of timelapse video in frames per second (integer).")
        .default_value(25)
        .scan<'i', int>()
        .metavar("RATE");

    
    argparse::ArgumentParser music_tool("music");
    music_tool.add_description("Plays a melody from a MusicXML file using the printer beep.");
    music_tool.add_argument("printer-address")
        .help("IP or network address of the printer.");
    music_tool.add_argument("xml-path")
        .help("Path to MusicXML (uncompressed, .xml) file to play. Only the notes in the first 'part' will be played.");

    parser.add_argument("-c", "--credentials")
        .help("Path to file containing authentication credentials with the ID on the first line, and key on the second. Required for all POST requests.")
        .metavar("PATH");
    parser.add_argument("-id", "--id")
        .help("Authentication ID.")
        .metavar("VALUE");
    parser.add_argument("-k", "--key")
        .help("Authentication key.")
        .metavar("VALUE");
    

    argparse::ArgumentParser authenticate_tool("authenticate");
    authenticate_tool.add_description("Generate authentication credentials for a printer. Requires physical access to the printer.");
    authenticate_tool.add_argument("printer-address")
        .help("IP or network address of the printer.");
    authenticate_tool.add_argument("username")
        .help("Username to be associated with your authentication credentials.");
    authenticate_tool.add_argument("--output")
        .help("Path to a file to store credentials.")
        .metavar("PATH");

    parser.add_subparser(timelapse_tool);
    parser.add_subparser(music_tool);
    parser.add_subparser(authenticate_tool);

    try
    {
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err)
    {
        cerr << err.what() << endl;
        cerr << parser;
        return 1;
    }

    if (parser.is_subcommand_used(timelapse_tool))
    {
        timelapse(parser.at<argparse::ArgumentParser>("timelapse"));
    }
    else if (parser.is_subcommand_used(music_tool))
    {
        string id, key;
        auto cred_arg = parser.present("--credentials");
        auto id_arg = parser.present("--id");
        auto key_arg = parser.present("--key");

        if (cred_arg.has_value())
        {
            auto credentials = load_auth_file(cred_arg.value());
            id = credentials.first;
            key = credentials.second;
        }
        else if (id_arg.has_value() && key_arg.has_value())
        {
            id = id_arg.value();
            key = key_arg.value();
        }
        else
        {
            throw std::invalid_argument("Music tool needs credentials to be set. See 'um3tools music -h' for details.");
        }

        music(parser.at<argparse::ArgumentParser>("music"), id, key);
    }
    else if (parser.is_subcommand_used(authenticate_tool))
    {
        authenticate(parser.at<argparse::ArgumentParser>("authenticate"));
    }
    else
    {
        cerr << "Invalid tool entered." << endl;
        return 1;
    }

    return 0;
}
