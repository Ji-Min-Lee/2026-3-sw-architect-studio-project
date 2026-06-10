cmake_minimum_required(VERSION 3.16)

project(TimeGrapher VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(MINGW)
    add_compile_options("-Wa,-mbig-obj")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS PrintSupport REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia Widgets)

set(PROJECT_SOURCES
    Main.cpp
    ui/MainWindow.cpp
    ui/MainWindow.h
    ui/MainWindow.ui
    ui/SettingsManager.h
    ui/SettingsManager.cpp
)

set(AUDIO_SOURCES
    audio/AudioWorker.h
    audio/AudioWorker.cpp
    audio/PlaybackWorker.h
    audio/PlaybackWorker.cpp
    audio/SimWorker.h
    audio/SimWorker.cpp
    audio/SharedAudio.h
    audio/AudioManager.h
    audio/AudioManager.cpp
)

set(ENGINE_SOURCES
    engine/Measurement.h
    engine/MeasurementEngine.h
    engine/MeasurementEngine.cpp
    engine/RollingLeastSquares.h
    engine/RollingLeastSquares.cpp
    engine/RollingAverage.h
    engine/RollingAverage.cpp
)

set(TABS_SOURCES
    tabs/BaseGraphTab.h
    tabs/RateScopeTab.h
    tabs/RateScopeTab.cpp
    tabs/TraceTab.h
    tabs/TraceTab.cpp
    tabs/SoundPrintTab.h
    tabs/SoundPrintTab.cpp
    tabs/BeatErrorTab.h
    tabs/BeatErrorTab.cpp
    tabs/VarioTab.h
    tabs/VarioTab.cpp
    tabs/SequenceTab.h
    tabs/SequenceTab.cpp
    tabs/BeatNoiseScopeTab.h
    tabs/BeatNoiseScopeTab.cpp
    tabs/LongTermTab.h
    tabs/LongTermTab.cpp
    tabs/EscapementTab.h
    tabs/EscapementTab.cpp
    tabs/SpectrogramTab.h
    tabs/SpectrogramTab.cpp
    tabs/WaveformCompTab.h
    tabs/WaveformCompTab.cpp
)

set(EXTERNAL_SOURCES
    external/qcustomplot.h
    external/qcustomplot.cpp
    external/Timegrapher.h
    external/Timegrapher.cpp
    external/Bph.h
    external/Bph.cpp
    external/Detector.h
    external/Detector.cpp
    external/Dsp.h
    external/Dsp.cpp
    external/SoundImageWidget.h
    external/SoundImageWidget.cpp
    external/SoundImageRenderer.h
    external/SoundImageRenderer.cpp
    external/WaveHeader.h
    external/WavStreamWriter.h
    external/WavStreamWriter.cpp
    external/WatchSynthStream.h
    external/WatchSynthStream.cpp
    external/LinuxAudio.cpp
    external/LinuxAudio.h
    external/WindowsAudio.h
    external/WindowsAudio.cpp
    external/Splash.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TimeGrapher
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${AUDIO_SOURCES}
        ${ENGINE_SOURCES}
        ${TABS_SOURCES}
        ${EXTERNAL_SOURCES}
    )
else()
    if(ANDROID)
        add_library(TimeGrapher SHARED ${PROJECT_SOURCES})
    else()
        add_executable(TimeGrapher ${PROJECT_SOURCES})
    endif()
endif()

# All subdirectories are in the include path so every file can use
# #include "File.h" without relative-path prefixes.
target_include_directories(TimeGrapher PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/tabs
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

target_link_libraries(TimeGrapher PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(TimeGrapher PRIVATE Qt${QT_VERSION_MAJOR}::PrintSupport)
target_link_libraries(TimeGrapher PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
IF(WIN32)
    target_link_libraries(TimeGrapher PRIVATE winmm Ole32 Uuid Propsys)
ELSEIF(LINUX)
    target_link_libraries(TimeGrapher PRIVATE asound)
ENDIF()

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TimeGrapher)
endif()
set_target_properties(TimeGrapher PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS TimeGrapher
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TimeGrapher)
endif()
