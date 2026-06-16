#include "MainWindow.h"

#include <QApplication>
#ifdef Q_OS_WIN
#include <windows.h>
#include <processthreadsapi.h>
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

 //QApplication::setStyle(QStyleFactory::create("Fusion"));

 QPixmap Pixmap(":/images/Splash.png");
 if (Pixmap.isNull())
  {
     qInfo() << "Failed to load splash image!";
  }
 QPixmap scaledPixmap = Pixmap.scaled(1280, 750, Qt::KeepAspectRatio, Qt::SmoothTransformation);

 QSplashScreen splash(scaledPixmap,Qt::WindowStaysOnTopHint);
 splash.show();

 QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
 int splashX = (screenGeometry.width() - splash.width()) / 2;
 int splashY = (screenGeometry.height() - splash.height()) / 2;
 splash.move(splashX, splashY);

 QThread::msleep(100); //Needed for Linux.... not sure why
 app.processEvents();

 QThread::sleep(4);

 MainWindow mainWindow;
 mainWindow.show();

 splash.finish(&mainWindow);

 result = app.exec();

#ifdef Q_OS_WIN
 timeEndPeriod(1);
#endif

 return result;
}
