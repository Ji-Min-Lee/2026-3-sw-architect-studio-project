#include "MainWindow.h"
#include "Measurement.h"
#include "SplashScreen.h"

#include <QApplication>
#include <QMetaType>
#include <QEventLoop>
#include <QGuiApplication>
#include <QScreen>
#include <QThread>
#ifdef Q_OS_WIN
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <csignal>
static void handleSignal(int) { QCoreApplication::quit(); }
#endif

int main(int argc, char *argv[])
{
  int result;

#ifdef Q_OS_WIN
 PROCESS_POWER_THROTTLING_STATE PowerThrottling = {0};
 PowerThrottling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
 PowerThrottling.ControlMask = PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION;
 PowerThrottling.StateMask = 0; //This will turn off - PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION;
 SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling));
 timeBeginPeriod(1);
 if (SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS)) {
     qInfo()<<"WINDOWS OS - Process successfully set to realtime";
 }
#endif

 QApplication app(argc, argv);
 qRegisterMetaType<Measurement>("Measurement");

#ifndef Q_OS_WIN
 // Ensure Ctrl+C / SIGTERM trigger a clean Qt shutdown so Logger destructor
 // writes the CSV before the process exits (critical when running under sudo).
 signal(SIGINT,  handleSignal);
 signal(SIGTERM, handleSignal);
#endif

 SplashScreen splash;
 const QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
 splash.move((screenGeometry.width() - splash.width()) / 2,
             (screenGeometry.height() - splash.height()) / 2);
 splash.start();

 QThread::msleep(100); // Needed for Linux.... not sure why
 app.processEvents();

 while (!splash.isAnimationComplete()) {
  app.processEvents(QEventLoop::AllEvents, 50);
  QThread::msleep(10);
 }

 MainWindow mainWindow;
 mainWindow.show();

 splash.close();

 result = app.exec();

#ifdef Q_OS_WIN
 timeEndPeriod(1);
#endif

 return result;
}
