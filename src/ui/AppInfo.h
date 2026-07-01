#ifndef APPINFO_H
#define APPINFO_H

#include <QString>

#ifndef TG_APP_VERSION
#define TG_APP_VERSION "1.0.2"
#endif
#ifndef TG_GIT_COMMIT
#define TG_GIT_COMMIT "dev"
#endif

namespace AppInfo {

inline QString version()
{
    return QStringLiteral(TG_APP_VERSION);
}

inline QString gitCommit()
{
    return QStringLiteral(TG_GIT_COMMIT);
}

inline QString versionLabel()
{
    return QStringLiteral("v%1").arg(version());
}

inline QString versionDetail()
{
    return QStringLiteral("v%1  \u00b7  build %2").arg(version(), gitCommit());
}

inline QString teamLabel()
{
    return QStringLiteral("Team 3  \u00b7  Blue Sky");
}

inline QString programLabel()
{
    return QStringLiteral("LG SW Architect Training Program 2026");
}

} // namespace AppInfo

#endif // APPINFO_H
