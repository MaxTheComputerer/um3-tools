#ifndef UM3_TIMELAPSE_HEADER
#define UM3_TIMELAPSE_HEADER

#include "ultimaker3.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <nlohmann/json.hpp>
#include <subprocess.hpp>

using json = nlohmann::json;
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

class Timelapse
{
    Ultimaker3 printer;
    string job_uuid;
    std::filesystem::path temp_dir;

    double get_head_z_coord()
    {
        for (size_t i = 0; i < 10; i++)
        {
            try
            {
                auto head_pos = printer.head_position(0);
                return head_pos["z"];
            }
            catch (json::exception)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }

        return -1;
    }

    inline std::string format_coord(const double coord)
    {
        auto coord_str = std::to_string(coord);
        std::replace(coord_str.begin(), coord_str.end(), '.', '_');
        return coord_str;
    }

    void save_snapshot(const int image_number, const double z_coord)
    {
        auto image_data = printer.take_snapshot();
        std::ofstream file(temp_dir.string() + "/" + std::to_string(image_number) + ".jpg", std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Failed to open image file." << endl;
            return;
        }

        file.write(image_data.c_str(), image_data.size());
        if (!file.good())
        {
            file.close();
            std::cerr << "Failed to write image file." << endl;
            return;
        }

        cout << "Saved snapshot at z = " << z_coord << endl;
        file.close();
    }

    void create_temp_directory()
    {
        if (!std::filesystem::create_directory(temp_dir))
        {
            throw std::runtime_error("Failed to create temporary directory.");
        }
        cout << "Created temporary directory at " << temp_dir << endl;
    }

    void remove_temp_directory()
    {
        std::filesystem::remove_all(temp_dir);
    }

    void make_video(int frame_rate, string out_path)
    {
        // ffmpeg -i %d.jpg -c:v libx264 -crf 18 -pix_fmt yuv420p test.mp4
        auto obuf = subprocess::check_output({
            "ffmpeg",
            "-hide_banner",
            "-loglevel", "error",
            "-i", temp_dir.string() + "/" + "\%d.jpg",
            "-c:v", "libx264",
            "-crf", "18",
            "-pix_fmt", "yuv420p",
            "-r", std::to_string(frame_rate),
            out_path
        });
    }

    void get_video_frames()
    {
        double current_z, previous_z;
        current_z = previous_z = get_head_z_coord();

        save_snapshot(0, current_z);
        int counter = 1;

        while (printer.progress() < 1)
        {
            while ((current_z = get_head_z_coord()) == previous_z)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            save_snapshot(counter, current_z);
            previous_z = current_z;
            counter++;
        }
    }

    void setup_job_info()
    {
        job_uuid = printer.job_uuid();
        temp_dir = std::filesystem::temp_directory_path() / "um3tools_temp" / job_uuid;
    }

public:
    Timelapse(Ultimaker3 printer) : printer(printer)
    {
    }

    void start(int frame_rate, string out_path = "")
    {
        if (printer.progress() == 1)
        {
            cerr << "No print in progress" << endl;
            return;
        }

        setup_job_info();

        // Clean directory if exists, then create
        remove_temp_directory();
        create_temp_directory();

        cout << "Recording timelapse..." << endl;

        get_video_frames();

        cout << "Recording completed." << endl;

        if (out_path == "")
        {
            out_path = job_uuid + ".mp4";
        }
        cout << "Generating video..." << endl;
        make_video(frame_rate, out_path);

        remove_temp_directory();

        cout << "Done. Timelapse saved to " << out_path << endl;
    }
};

#endif