#pragma once

#include <QtCore>
#include <QString>
#include <QGLWidget>
#include <QOpenGLFunctions>

namespace SleepyHead
{
	namespace Common
	{
		class GraphicsEngine
		{
		public:
			static QString CSTR_GFX_ANGLE;
			static QString CSTR_GFX_OpenGL;
			static QString CSTR_GFX_BrokenGL;
		
			//! \brief Returns a text string naming the current graphics engine
			static QString getGraphicsEngine();
			static QString getOpenGLVersionString();
			static float getOpenGLVersion();
		};
	}
}
