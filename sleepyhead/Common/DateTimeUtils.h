#pragma once

#include <QLocale>

namespace SleepyHead
{
	namespace Common
	{
		class DateTimeUtils
		{
		public:
			//! \brief Gets the first day of week from the system locale, to show in the calendars.
			static Qt::DayOfWeek firstDayOfWeekFromLocale();
		};
	}
}
