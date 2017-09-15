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

const QString STR_GRAPH_EventBreakdown = "EventBreakdown";
const QString STR_GRAPH_SleepFlags = "SF";
const QString STR_GRAPH_Weight = "Weight";
const QString STR_GRAPH_BMI = "BMI";
const QString STR_GRAPH_Zombie = "Zombie";
const QString STR_GRAPH_Sessions = "Sessions";
const QString STR_GRAPH_SessionTimes = "SessionTimes";
const QString STR_GRAPH_Usage = "Usage";
const QString STR_GRAPH_AHI = "AHI";
const QString STR_GRAPH_PeakAHI = "PeakAHI";

//! \brief Returns a text string naming the current graphics engine
QString getGraphicsEngine();
QString getOpenGLVersionString();
float getOpenGLVersion();

#endif // COMMON_GUI_H
