cmake_minimum_required(VERSION 3.8)

add_library(RtMidiW)
add_library(RtMidiW::RtMidiW ALIAS RtMidiW)
target_compile_features(RtMidiW PUBLIC cxx_std_11)

# ---Add source files---
target_include_directories(RtMidiW SYSTEM PUBLIC include)
target_sources(RtMidiW PRIVATE rtmidi/RtMidi.cpp)

# ---rtmidi license---
install(FILES "rtmidi/LICENSE" DESTINATION "license/rtmidi")

# ---Link with available backend---
# Check for Jack (any OS)
find_library(JACK_LIB jack)
find_package(PkgConfig)
pkg_check_modules(jack jack)

if(JACK_LIB OR jack_FOUND)
    set(HAVE_JACK TRUE)
endif()

# Necessary for Windows
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Init variables
set(LINKLIBS)
set(PUBLICLINKLIBS)
set(INCDIRS)
set(PKGCONFIG_REQUIRES)
set(LIBS_REQUIRES)
set(API_DEFS)
set(API_LIST)
set(PACKAGE_DEPENDENCIES)

# Tweak API-specific configuration.

# Jack
if(RTMIDI_API_JACK)
    if(NOT HAVE_JACK)
        message(FATAL_ERROR "Jack API requested but no Jack dev libraries found")
    endif()

    set(NEED_PTHREAD ON)
    list(APPEND PKGCONFIG_REQUIRES "jack")
    list(APPEND API_DEFS "-D__UNIX_JACK__")
    list(APPEND API_LIST "jack")

    if(jack_FOUND)
        list(APPEND LINKLIBS ${jack_LIBRARIES})
        list(APPEND INCDIRS ${jack_INCLUDEDIR})
    else()
        list(APPEND LINKLIBS ${JACK_LIB})
    endif()

    # Check for jack_port_rename
    include(CheckSymbolExists)
    set(tmp_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    list(APPEND CMAKE_REQUIRED_LIBRARIES jack)
    check_symbol_exists(jack_port_rename ${jack_INCLUDEDIR}/jack/jack.h JACK_HAS_PORT_RENAME)
    set(CMAKE_REQUIRED_LIBRARIES ${tmp_CMAKE_REQUIRED_LIBRARIES})

    if(JACK_HAS_PORT_RENAME)
        list(APPEND API_DEFS "-DJACK_HAS_PORT_RENAME")
    endif()
endif()

# ALSA
find_package(ALSA)

if(ALSA_FOUND OR ALSA)
    set(NEED_PTHREAD ON)
    list(APPEND INCDIRS ${ALSA_INCLUDE_DIR})
    list(APPEND LINKLIBS ALSA::ALSA)
    list(APPEND PKGCONFIG_REQUIRES "alsa")
    list(APPEND API_DEFS "-D__LINUX_ALSA__")
    list(APPEND API_LIST "alsa")
    list(APPEND PACKAGE_DEPENDENCIES "find_dependency(ALSA)")
endif()

# WinMM
if(RTMIDI_API_WINMM)
    list(APPEND API_DEFS "-D__WINDOWS_MM__")
    list(APPEND API_LIST "winmm")
    list(APPEND LINKLIBS winmm)
endif()

# CoreMIDI
if(RTMIDI_API_CORE)
    find_library(CORESERVICES_LIB CoreServices)
    find_library(COREAUDIO_LIB CoreAudio)
    find_library(COREMIDI_LIB CoreMIDI)
    find_library(COREFOUNDATION_LIB CoreFoundation)
    list(APPEND API_DEFS "-D__MACOSX_CORE__")
    list(APPEND API_LIST "coremidi")
    list(APPEND LINKLIBS ${CORESERVICES_LIB} ${COREAUDIO_LIB} ${COREMIDI_LIB} ${COREFOUNDATION_LIB})
    list(APPEND LIBS_REQUIRES "-framework CoreServices -framework CoreAudio -framework CoreMIDI -framework CoreFoundation")
    list(APPEND LINKFLAGS "-Wl,-F/Library/Frameworks")
endif()

# Android AMIDI
if(ANDROID)
    set(NEED_PTHREAD ON)
    set(JAVA_INCLUDE_PATH2 NotNeeded)
    set(JAVA_AWT_INCLUDE_PATH NotNeeded)
    find_package(JNI)

    # find_library(ALOG_LIB log android)
    list(APPEND API_DEFS "-D__AMIDI__")
    list(APPEND API_LIST "amidi")
    list(APPEND LINKLIBS log ${JNI_LIBRARIES} amidi)
endif()

# pthread
if(NEED_PTHREAD)
    find_package(Threads REQUIRED
        CMAKE_THREAD_PREFER_PTHREAD
        THREADS_PREFER_PTHREAD_FLAG)
    list(APPEND PUBLICLINKLIBS Threads::Threads)
    list(APPEND PACKAGE_DEPENDENCIES "find_dependency(Threads)")
endif()

# Set compile-time definitions
target_compile_definitions(RtMidiW PRIVATE ${API_DEFS})
target_compile_definitions(RtMidiW PRIVATE RTMIDI_EXPORT)
target_link_libraries(RtMidiW
    PUBLIC ${PUBLICLINKLIBS}
    PRIVATE ${LINKLIBS}
)

# API Options
option(RTMIDI_API_JACK "Compile with JACK support." ${HAVE_JACK})
option(RTMIDI_API_WINMM "Compile with WINMM support." ${WIN32})
option(RTMIDI_API_CORE "Compile with CoreMIDI support." ${APPLE})
option(RTMIDI_API_ALSA "Compile with ALSA support." ${ALSA})
option(RTMIDI_API_AMIDI "Compile with Android support." ${ANDROID})