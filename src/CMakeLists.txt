cmake_minimum_required(VERSION 3.16)

project(TimeGrapher VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Performance logging (console + CSV). OFF by default so release builds carry
# zero overhead (instrumentation folds away via TG_NOW()==0). Enable with:
#   cmake -DENABLE_LOGGING=ON ...
option(ENABLE_LOGGING "Enable performance logging (console + CSV)" OFF)

if(MINGW)
    add_compile_options("-Wa,-mbig-obj")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS PrintSupport REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia Widgets)

set(LOGGING_SOURCES
    logging/Logger.h
    logging/Logger.cpp
    logging/SysStats.h
    logging/SysStats.cpp
)

set(PROJECT_SOURCES
    Main.cpp
    ui/MainWindow.cpp
    ui/MainWindow.h
    ui/MainWindow.ui
    ui/SettingsManager.h
    ui/SettingsManager.cpp
)

set(AUDIO_SOURCES
    audio/IAudioSource.h
    audio/IAudioSource.cpp
    audio/AudioWorker.h
    audio/AudioWorker.cpp
    audio/PlaybackWorker.h
    audio/PlaybackWorker.cpp
    audio/SimWorker.h
    audio/SimWorker.cpp
    audio/SharedAudio.h
    audio/AudioManager.h
    audio/AudioManager.cpp
    audio/DSPWorker.h
    audio/DSPWorker.cpp
)

set(ENGINE_SOURCES
    engine/Measurement.h
    engine/MeasurementEngine.h
    engine/MeasurementEngine.cpp
    engine/RollingLeastSquares.h
    engine/RollingLeastSquares.cpp
    engine/RollingAverage.h
    engine/RollingAverage.cpp
    engine/WatchMath.h
    engine/WatchMath.cpp
    engine/WatchDiagnostics.h
    engine/WatchDiagnostics.cpp
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
    tabs/SweepScopeTab.h
    tabs/SweepScopeTab.cpp
    tabs/FilterScopeTab.h
    tabs/FilterScopeTab.cpp
    tabs/RadarChartTab.h
    tabs/RadarChartTab.cpp
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
    external/kiss_fft.h
    external/kiss_fft.c
    external/_kiss_fft_guts.h
)

# ── Logging build option ──────────────────────────────────────────────────────
option(ENABLE_LOGGING "Enable per-frame performance logging (EXP-02)" OFF)
if(ENABLE_LOGGING)
    add_compile_definitions(ENABLE_LOGGING)
    message(STATUS "[TimeGrapher] ENABLE_LOGGING=ON — per-frame CSV logging active")
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TimeGrapher
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${AUDIO_SOURCES}
        ${ENGINE_SOURCES}
        ${TABS_SOURCES}
        ${EXTERNAL_SOURCES}
        ${LOGGING_SOURCES}
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
# Kiss FFT: 순수 C 파일을 별도 정적 라이브러리로 빌드 후 링크
# CMAKE_AUTOMOC/AUTORCC 가 C 라이브러리에 붙으면 kiss_fft.c 가 빌드되지 않으므로 명시적으로 OFF
add_library(kissfft STATIC ${CMAKE_CURRENT_SOURCE_DIR}/external/kiss_fft.c)
set_target_properties(kissfft PROPERTIES AUTOMOC OFF AUTORCC OFF AUTOUIC OFF)
target_include_directories(kissfft PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external)

target_include_directories(TimeGrapher PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/tabs
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/logging
)

target_link_libraries(TimeGrapher PRIVATE kissfft)
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
    # When logging is on we need a console subsystem to see qInfo output;
    # otherwise keep the normal GUI subsystem (no console window).
    WIN32_EXECUTABLE $<IF:$<BOOL:${ENABLE_LOGGING}>,FALSE,TRUE>
)

if(ENABLE_LOGGING)
    target_compile_definitions(TimeGrapher PRIVATE ENABLE_LOGGING)
    message(STATUS "TimeGrapher: performance logging ENABLED")
else()
    message(STATUS "TimeGrapher: performance logging disabled (release/default)")
endif()

# Build-time git commit, injected so each log self-identifies its source build.
# Captured at configure time; reconfigure (or clean build) to refresh.
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE TG_GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
if(NOT TG_GIT_COMMIT)
    set(TG_GIT_COMMIT "unknown")
endif()
target_compile_definitions(TimeGrapher PRIVATE TG_GIT_COMMIT="${TG_GIT_COMMIT}")
message(STATUS "TimeGrapher: git_commit=${TG_GIT_COMMIT}")

include(GNUInstallDirs)
install(TARGETS TimeGrapher
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TimeGrapher)
endif()

# ── Unit Tests ────────────────────────────────────────────────────────────────
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
enable_testing()

set(TEST_SOURCES
    tests/test_graph_tabs.cpp
    tabs/TraceTab.cpp
    tabs/BeatErrorTab.cpp
    tabs/VarioTab.cpp
    tabs/BaseGraphTab.h   # MOC 생성 트리거용 (no .cpp)
    external/qcustomplot.cpp
)

add_executable(TestGraphTabs ${TEST_SOURCES})
set_target_properties(TestGraphTabs PROPERTIES AUTOMOC ON)
target_include_directories(TestGraphTabs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/tabs
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/logging
)
target_link_libraries(TestGraphTabs PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
)
add_test(NAME GraphTabs COMMAND TestGraphTabs)

# ── Added-tab value-fidelity tests ───────────────────────────────────────────
add_executable(TestAddedTabs
    tests/test_added_tabs.cpp
    tabs/SequenceTab.cpp
    tabs/EscapementTab.cpp
    tabs/LongTermTab.cpp
    tabs/BeatNoiseScopeTab.cpp
    tabs/WaveformCompTab.cpp
    tabs/SpectrogramTab.cpp
    tabs/RadarChartTab.cpp
    tabs/BaseGraphTab.h
    external/qcustomplot.cpp
)
set_target_properties(TestAddedTabs PROPERTIES AUTOMOC ON)
target_include_directories(TestAddedTabs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/tabs
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/logging
)
target_link_libraries(TestAddedTabs PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    kissfft
)
add_test(NAME AddedTabs COMMAND TestAddedTabs)

# ── WatchMath pure-function tests ─────────────────────────────────────────────
add_executable(TestWatchMath
    tests/test_watch_math.cpp
    engine/WatchMath.h
    engine/WatchMath.cpp
)
set_target_properties(TestWatchMath PROPERTIES AUTOMOC ON)
target_include_directories(TestWatchMath PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
)
target_link_libraries(TestWatchMath PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME WatchMath COMMAND TestWatchMath)

# ── RollingAverage tests ──────────────────────────────────────────────────────
add_executable(TestRollingAverage
    tests/test_rolling_average.cpp
    engine/RollingAverage.h
    engine/RollingAverage.cpp
)
set_target_properties(TestRollingAverage PROPERTIES AUTOMOC ON)
target_include_directories(TestRollingAverage PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
)
target_link_libraries(TestRollingAverage PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME RollingAverage COMMAND TestRollingAverage)

# ── RollingLeastSquares tests ─────────────────────────────────────────────────
add_executable(TestRollingLeastSquares
    tests/test_rolling_least_squares.cpp
    engine/RollingLeastSquares.h
    engine/RollingLeastSquares.cpp
)
set_target_properties(TestRollingLeastSquares PROPERTIES AUTOMOC ON)
target_include_directories(TestRollingLeastSquares PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
)
target_link_libraries(TestRollingLeastSquares PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME RollingLeastSquares COMMAND TestRollingLeastSquares)

# ── MeasurementEngine tests ───────────────────────────────────────────────────
add_executable(TestMeasurementEngine
    tests/test_measurement_engine.cpp
    engine/Measurement.h
    engine/MeasurementEngine.h
    engine/MeasurementEngine.cpp
    engine/RollingAverage.h
    engine/RollingAverage.cpp
    engine/RollingLeastSquares.h
    engine/RollingLeastSquares.cpp
    engine/WatchMath.h
    engine/WatchMath.cpp
    external/Timegrapher.cpp
    external/Bph.cpp
    external/Detector.cpp
    external/Dsp.cpp
)
set_target_properties(TestMeasurementEngine PROPERTIES AUTOMOC ON)
target_include_directories(TestMeasurementEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/audio
)
target_link_libraries(TestMeasurementEngine PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    kissfft
)
add_test(NAME MeasurementEngine COMMAND TestMeasurementEngine)

# ── Remaining-tabs tests (RateScopeTab, SweepScopeTab, FilterScopeTab, SoundPrintTab) ─
add_executable(TestRemainingTabs
    tests/test_remaining_tabs.cpp
    tabs/RateScopeTab.cpp
    tabs/SweepScopeTab.cpp
    tabs/FilterScopeTab.cpp
    tabs/SoundPrintTab.cpp
    tabs/BaseGraphTab.h
    external/qcustomplot.cpp
    external/SoundImageWidget.cpp
    external/SoundImageRenderer.cpp
)
set_target_properties(TestRemainingTabs PROPERTIES AUTOMOC ON)
target_include_directories(TestRemainingTabs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/tabs
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/logging
)
target_link_libraries(TestRemainingTabs PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
)
add_test(NAME RemainingTabs COMMAND TestRemainingTabs)

# ── Engine file-verification harness (wav → MeasurementEngine → ground truth) ─
add_executable(EngineFileCheck
    tests/engine_file_check.cpp
    ${ENGINE_SOURCES}
    external/Timegrapher.cpp
    external/Bph.cpp
    external/Detector.cpp
    external/Dsp.cpp
)
set_target_properties(EngineFileCheck PROPERTIES AUTOMOC ON)
target_include_directories(EngineFileCheck PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/audio
)
target_link_libraries(EngineFileCheck PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    kissfft
)
