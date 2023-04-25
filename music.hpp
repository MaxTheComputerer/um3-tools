#ifndef UM3_MUSIC_HEADER
#define UM3_MUSIC_HEADER

#include "ultimaker3.hpp"
#include "mx/api/DocumentManager.h"
#include "mx/api/ScoreData.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>

using std::cout;
using std::endl;
using std::string;
using std::vector;

struct Note
{
    const double frequency;
    const double duration;

    Note(const double frequency, const double duration) : frequency(frequency), duration(duration)
    {
    }
};

class MusicXmlPlayer
{
    Ultimaker3 printer;
    mx::api::PartData part;
    int tempo;
    int ticks_per_quarter;

    mx::api::ScoreData load_mxl(string filename)
    {
        // create a reference to the singleton which holds documents in memory for us
        auto &mgr = mx::api::DocumentManager::getInstance();

        // ask the document manager to parse the xml into memory for us, returns a document ID.
        const auto documentID = mgr.createFromFile(filename);

        // get the structural representation of the score from the document manager
        const auto score = mgr.getData(documentID);

        // we need to explicitly destroy the document from memory
        mgr.destroyDocument(documentID);

        return score;
    }

    vector<mx::api::NoteData> extract_notes()
    {
        vector<mx::api::NoteData> result;
        for (auto &&measure : part.measures)
        {
            auto notes = measure.staves.front().voices[0].notes;
            result.insert(result.end(), notes.begin(), notes.end());
        }
        return result;
    }

    inline void note_sleep(double duration)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds((int)duration));
    }

    inline double get_frequency(mx::api::PitchData pitch)
    {
        static const vector<int> semitone_offsets{-9, -7, -5, -4, -2, 0, 2};
        auto semitones = (pitch.octave - 4) * 12 + semitone_offsets[(int)pitch.step] + pitch.alter;
        return 440 * pow(2, semitones / 12.0);
    }

    inline double get_duration(mx::api::DurationData data)
    {
        auto beats = (double)data.durationTimeTicks / (double)ticks_per_quarter;
        return 60000 * beats / tempo;
    }

public:
    MusicXmlPlayer(Ultimaker3 printer, std::string input_filename) : printer(printer)
    {
        if (!printer.is_authenticated())
        {
            throw std::invalid_argument("Printer must be authenticated.");
        }

        auto score = load_mxl(input_filename);

        if (score.parts.size() == 0)
        {
            throw std::runtime_error("MusicXML score has no parts.");
        }

        part = score.parts.at(0);

        if (part.measures.size() == 0)
        {
            throw std::runtime_error("Part " + part.name + " has no measures.");
        }

        tempo = part.measures.front().staves.front().directions.front().tempos.front().beatsPerMinute.beatsPerMinute;
        ticks_per_quarter = score.ticksPerQuarter;

        cout << "Loaded MusicXML " << input_filename << endl;
    }

    void play()
    {
        const auto notes = extract_notes();

        auto previous_time = notes.front().tickTimePosition;

        for (auto &&note : notes)
        {
            if (note.isChord && note.tickTimePosition == previous_time)
                continue;

            auto duration = get_duration(note.durationData);

            if (!note.isRest && !note.isUnpitched)
            {
                auto frequency = get_frequency(note.pitchData);
                printer.beep(frequency, duration);
                cout << "Playing note at " << frequency << "Hz for " << duration << "ms" << endl;
            }
            else
            {
                cout << "Resting for " << duration << "ms" << endl;
            }

            note_sleep(duration);
            previous_time = note.tickTimePosition;
        }
    }
};

#endif