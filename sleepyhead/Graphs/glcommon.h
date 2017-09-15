/* glcommon GL code & font stuff Header
 *
 * Copyright (c) 2011-2016 Mark Watkins <jedimark@users.sourceforge.net>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of the Linux
 * distribution for more details. */

#ifndef GLCOMMON_H
#define GLCOMMON_H

#include "Common/CommonColors.h"

#include <QColor>

#ifndef nullptr
#define nullptr NULL
#endif

//! \brief Returns the grayscale brightness (between 0 and 1) of a color
float brightness(QColor color);

#define MIN(a,b) (((a)<(b)) ? (a) : (b));
#define MAX(a,b) (((a)<(b)) ? (b) : (a));

const QColor COLOR_Text = Qt::black;
const QColor COLOR_Outline = Qt::black;

const QColor COLOR_ALT_BG1 = QColor(0xc8, 0xff, 0xc8, 0x7f); // Alternating Background Color 1 (Event Flags)
const QColor COLOR_ALT_BG2 = SleepyHead::Common::CommonColors::COLOR_White;                    // Alternating Background Color 2 (Event Flags)


QColor brighten(QColor color, float mult = 2.0);

const int max_history = 50;

#ifndef M_PI
const double M_PI = 3.141592653589793;
#endif

#ifdef BUILD_WITH_MSVC
// Visual C++ earlier than 2013 doesn't have round in it's maths header..
#if (_MSC_VER < 1800)
double round(double number);
#endif
#endif

#endif // GLCOMMON_H
