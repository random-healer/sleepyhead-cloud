#include "GraphicsEngine.h"

using namespace SleepyHead::Common;

QString GraphicsEngine::CSTR_GFX_ANGLE = "ANGLE";
QString GraphicsEngine::CSTR_GFX_OpenGL = "OpenGL";
QString GraphicsEngine::CSTR_GFX_BrokenGL = "BrokenGL";

QString GraphicsEngine::getGraphicsEngine()
{
	QString gfxEngine = QString();
#ifdef BROKEN_OPENGL_BUILD
	gfxEngine =  GraphicsEngine::CSTR_GFX_BrokenGL;
#else
	QString glversion = getOpenGLVersionString();
	if (glversion.contains(GraphicsEngine::CSTR_GFX_ANGLE)) {
		gfxEngine = GraphicsEngine::CSTR_GFX_ANGLE;
	} else {
		gfxEngine = GraphicsEngine::CSTR_GFX_OpenGL;
	}
#endif
	return gfxEngine;
}

QString GraphicsEngine::getOpenGLVersionString()
{
	static QString glversion;
	
	if (glversion.isEmpty()) {
		QGLWidget w;
		w.makeCurrent();
		
		QOpenGLFunctions f;
		f.initializeOpenGLFunctions();
		glversion = QString(QLatin1String(reinterpret_cast<const char*>(f.glGetString(GL_VERSION))));
		
		qDebug() << "OpenGL Version:" << glversion;
	}
	return glversion;
}

float GraphicsEngine::getOpenGLVersion()
{
	QString glversion = getOpenGLVersionString();
	glversion = glversion.section(" ",0,0);
	bool ok;
	float v = glversion.toFloat(&ok);
	
	if (!ok) {
		QString tmp = glversion.section(".",0,1);
		v = tmp.toFloat(&ok);
		if (!ok) {
			// just look at major, we are only interested in whether we have OpenGL 2.0 anyway
			tmp = glversion.section(".",0,0);
			v = tmp.toFloat(&ok);
		}
	}
	return v;
}
