/* Common GUI Functions Header
 *
 * Copyright (c) 2011-2016 Mark Watkins <jedimark@users.sourceforge.net>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of the Linux
 * distribution for more details. */

#ifndef COMMON_GUI_H
#define COMMON_GUI_H

#include "Common/FlagColors.h"

#include <QLocale>
#include "Graphs/glcommon.h"

//! \brief Gets the first day of week from the system locale, to show in the calendars.
Qt::DayOfWeek firstDayOfWeekFromLocale();

#endif // COMMON_GUI_H
