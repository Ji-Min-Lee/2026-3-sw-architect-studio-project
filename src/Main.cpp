#include "MainWindow.h"
#include "Measurement.h"

#include <QApplication>
#include <QMetaType>
#include <QElapsedTimer>
#include <QEvent>
#include <QEventLoop>
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

 // Show splash up to 2s; click / key / double-click skips early.
 bool splashSkipped = false;
 class SplashSkipFilter : public QObject {
 public:
  explicit SplashSkipFilter(bool *flag, QObject *parent = nullptr)
      : QObject(parent), mFlag(flag) {}
 protected:
  bool eventFilter(QObject *, QEvent *event) override {
   switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    case QEvent::KeyPress:
     *mFlag = true;
     break;
    default:
     break;
   }
   return false;
  }
 private:
  bool *mFlag;
 };
 SplashSkipFilter splashSkip(&splashSkipped);
 splash.installEventFilter(&splashSkip);

 QElapsedTimer splashClock;
 splashClock.start();
 constexpr int kSplashMaxMs = 2000;
 while (splashClock.elapsed() < kSplashMaxMs && !splashSkipped) {
  app.processEvents(QEventLoop::AllEvents, 50);
  QThread::msleep(10);
 }

 MainWindow mainWindow;
 mainWindow.show();

 splash.finish(&mainWindow);

 result = app.exec();

#ifdef Q_OS_WIN
 timeEndPeriod(1);
#endif

 return result;
}
