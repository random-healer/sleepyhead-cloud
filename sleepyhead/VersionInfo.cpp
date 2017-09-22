#include "VersionInfo.h"

const QString VersionInfo::ReleaseStatus = "beta";

const QString VersionInfo::VersionString = QString("%1.%2.%3-%4-%5").arg(VersionInfo::major_version).arg(VersionInfo::minor_version).arg(VersionInfo::revision_number).arg(VersionInfo::ReleaseStatus).arg(build_number);

#ifdef Q_OS_MAC
const QString VersionInfo::PlatformString = "MacOSX";
#elif defined(Q_OS_WIN32)
const QString VersionInfo::PlatformString = "Win32";
#elif defined(Q_OS_LINUX)
const QString VersionInfo::PlatformString = "Linux";
#elif defined(Q_OS_HAIKU)
const QString VersionInfo::PlatformString = "Haiku";
#endif
