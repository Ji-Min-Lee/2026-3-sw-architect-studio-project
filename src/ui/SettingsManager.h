#pragma once
#include <QObject>
#include <QString>

// Presentation Layer: persists user preferences via QSettings.
// Key/value store for BPH, SampleRate, LiftAngle, AveragingPeriod, UseOnset.
class SettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = nullptr);

    int    bph()             const;
    int    sampleRate()      const;
    double liftAngle()       const;
    int    averagingPeriod() const;
    bool   useOnset()        const;
    QString deviceName()     const;

    void setBph(int value);
    void setSampleRate(int value);
    void setLiftAngle(double value);
    void setAveragingPeriod(int value);
    void setUseOnset(bool value);
    void setDeviceName(const QString &value);
};
