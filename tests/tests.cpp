#include <RtMidiW/RtMidiW.hpp>
#include <cstdlib>

auto main() -> int
{
    std::vector<RtMidi::Api> apis;
    RtMidi::getCompiledApi(apis);
    if (apis.empty())
    {
        std::cerr << "No APIs found\n";
        return EXIT_FAILURE;
    }
    if (apis.size() == 1 && apis[0] == RtMidi::Api::RTMIDI_DUMMY)
    {
        std::cerr << "Only dummy API found\n";
        return EXIT_FAILURE;
    }
}