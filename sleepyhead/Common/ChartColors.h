#pragma once

#include <QColor>

#include "CommonColors.h"

namespace SleepyHead
{
	namespace Common
	{
		class ChartColors
		{
		public:
			static QColor COLOR_EPAP;
			static QColor COLOR_IPAP;
			static QColor COLOR_IPAPLo;
			static QColor COLOR_IPAPHi;
			static QColor COLOR_Plethy;
			static QColor COLOR_Pulse;
			static QColor COLOR_SPO2;
			static QColor COLOR_FlowRate;
			static QColor COLOR_Pressure;
			static QColor COLOR_RDI;
			static QColor COLOR_AHI;
			static QColor COLOR_Leak;
			static QColor COLOR_LeakTotal;
			static QColor COLOR_MaxLeak;
			static QColor COLOR_Snore;
			static QColor COLOR_RespRate;
			static QColor COLOR_MaskPressure;
			static QColor COLOR_PTB;            // Patient Triggered Breathing
			static QColor COLOR_MinuteVent;
			static QColor COLOR_TgMV;
			static QColor COLOR_TidalVolume;
			static QColor COLOR_FLG;            // Flow Limitation Graph
			static QColor COLOR_IE;             // Inspiratory Expiratory Ratio
			static QColor COLOR_Te;
			static QColor COLOR_Ti;
			static QColor COLOR_SleepStage;
		};
	}
}
