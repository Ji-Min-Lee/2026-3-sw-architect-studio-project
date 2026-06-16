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

void SettingsManager::setBph(int value)              { s().setValue("bph", value); }
void SettingsManager::setSampleRate(int value)        { s().setValue("sampleRate", value); }
void SettingsManager::setLiftAngle(double value)      { s().setValue("liftAngle", value); }
void SettingsManager::setAveragingPeriod(int value)   { s().setValue("averagingPeriod", value); }
void SettingsManager::setUseOnset(bool value)         { s().setValue("useOnset", value); }
void SettingsManager::setDeviceName(const QString &value) { s().setValue("deviceName", value); }
