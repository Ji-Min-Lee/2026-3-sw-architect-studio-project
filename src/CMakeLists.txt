cmake_minimum_required(VERSION 3.16)

project(TimeGrapher VERSION 0.1 LANGUAGES CXX)

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

#DP REMOVED NOT NEED IN THIS VERSION
#IF(WIN32)
#add_library(fftw SHARED IMPORTED)
#set_property(TARGET fftw PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/fftw/libfftw3f-3.dll")
#set_property(TARGET fftw PROPERTY IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/fftw/libfftw3f-3.lib")
#ENDIF()
#DP-END

set(PROJECT_SOURCES
    Main.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TimeGrapher
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        qcustomplot.h
        qcustomplot.cpp
        AudioWorker.h
        AudioWorker.cpp
        SharedAudio.h
        PlaybackWorker.h
        PlaybackWorker.cpp
        WaveHeader.h
        WavStreamWriter.h
        WavStreamWriter.cpp
        Timegrapher.h
        Timegrapher.cpp
        Bph.h
        Bph.cpp
        Detector.h
        Detector.cpp
        Dsp.h
        Dsp.cpp
        RollingLeastSquares.h
        RollingLeastSquares.cpp
        SoundImageWidget.h
        SoundImageWidget.cpp
        SoundImageRenderer.h
        SoundImageRenderer.cpp
        RollingAverage.h
        RollingAverage.cpp
        Logger.h
        Logger.cpp
        SysStats.h
        SysStats.cpp
        SimWorker.cpp
        SimWorker.h
        WatchSynthStream.cpp
        WatchSynthStream.h
        Splash.qrc
        LinuxAudio.cpp
        LinuxAudio.h
        WindowsAudio.h
        WindowsAudio.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TimeGrapher APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TimeGrapher SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TimeGrapher
            ${PROJECT_SOURCES}
        )
    endif()
endif()

#DP-START REMOVED NOT NEED IN THIS VERSION
#target_include_directories(TimeGrapher PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/fftw")
#DP-END

target_link_libraries(TimeGrapher PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(TimeGrapher PRIVATE Qt${QT_VERSION_MAJOR}::PrintSupport)
target_link_libraries(TimeGrapher PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
IF(WIN32)
#DP REMOVED NOT NEED IN THIS VERSION
#target_link_libraries(TimeGrapher PRIVATE fftw)
#DP-END
target_link_libraries(TimeGrapher PRIVATE winmm)
target_link_libraries(TimeGrapher PRIVATE Ole32)
target_link_libraries(TimeGrapher PRIVATE Uuid)
target_link_libraries(TimeGrapher PRIVATE Propsys)
ELSEIF(LINUX)
target_link_libraries(TimeGrapher PRIVATE asound)
#DP REMOVED NOT NEED IN THIS VERSION
#target_link_libraries(TimeGrapher PRIVATE fftw3f)
#DP-END
ELSE()

ENDIF()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
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
