#include "SettingsManager.h"
#include <QSettings>

static QSettings &s() {
    static QSettings cfg("TimeGrapher", "TimeGrapher");
    return cfg;
}

SettingsManager::SettingsManager(QObject *parent) : QObject(parent) {}

int    SettingsManager::bph()             const { return s().value("bph", 0).toInt(); }
int    SettingsManager::sampleRate()      const { return s().value("sampleRate", 48000).toInt(); }
double SettingsManager::liftAngle()       const { return s().value("liftAngle", 52.0).toDouble(); }
int    SettingsManager::averagingPeriod() const { return s().value("averagingPeriod", 20).toInt(); }
bool   SettingsManager::useOnset()        const { return s().value("useOnset", false).toBool(); }
QString SettingsManager::deviceName()     const { return s().value("deviceName", "").toString(); }

void SettingsManager::setBph(int v)              { s().setValue("bph", v); }
void SettingsManager::setSampleRate(int v)        { s().setValue("sampleRate", v); }
void SettingsManager::setLiftAngle(double v)      { s().setValue("liftAngle", v); }
void SettingsManager::setAveragingPeriod(int v)   { s().setValue("averagingPeriod", v); }
void SettingsManager::setUseOnset(bool v)         { s().setValue("useOnset", v); }
void SettingsManager::setDeviceName(const QString &v) { s().setValue("deviceName", v); }
