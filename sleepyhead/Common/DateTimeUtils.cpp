#include "DateTimeUtils.h"

using namespace SleepyHead::Common;

Qt::DayOfWeek DateTimeUtils::firstDayOfWeekFromLocale()
{
	return QLocale::system().firstDayOfWeek();
}
