cmake_minimum_required(VERSION 3.21)
project(KainoteTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Headless tests for the modules that need neither wxWidgets nor a window.
add_executable(kainote_tests
    check_main.cpp
    FrameQueueTests.cpp
    PlaybackTests.cpp
    TimebaseTests.cpp
    UndoHistoryTests.cpp
    ../Kainote/FrameQueue.cpp
    ../Kainote/Playback.cpp
    ../Kainote/Timebase.cpp
)

enable_testing()
add_test(NAME kainote_tests COMMAND kainote_tests)
