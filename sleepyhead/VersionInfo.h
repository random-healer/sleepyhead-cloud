#pragma once

#include <QString>
#include "build_number.h"

class VersionInfo
{
public:
	static const int major_version = 1;   // incompatible API changes
	static const int minor_version = 0;   // new features that don't break things
	static const int revision_number = 0; // bugfixes, revisions

	static const QString ReleaseStatus;
	static const QString VersionString;

	static const QString PlatformString;
};
