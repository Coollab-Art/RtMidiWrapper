cmake_minimum_required(VERSION 3.8)

add_library(RtMidiW)
add_library(RtMidiW::RtMidiW ALIAS RtMidiW)
target_compile_features(RtMidiW PUBLIC cxx_std_11)

# ---Add source files---
target_include_directories(RtMidiW SYSTEM PUBLIC include)
target_sources(RtMidiW PRIVATE rtmidi/RtMidi.cpp)