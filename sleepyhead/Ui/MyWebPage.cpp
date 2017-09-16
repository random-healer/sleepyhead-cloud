#include "MyWebPage.h"

#include <QtCore>

#include "mainwindow.h"

extern MainWindow * mainwin;

MyWebPage::MyWebPage(QObject *parent) : QWebPage(parent)
{
	// Enable plugin support
	settings()->setAttribute(QWebSettings::PluginsEnabled, true);
}

QObject *MyWebPage::createPlugin(const QString &classid, const QUrl &url, const QStringList & paramNames, const QStringList & paramValues)
{
	Q_UNUSED(paramNames)
	Q_UNUSED(paramValues)
	Q_UNUSED(url)
	
	if (classid=="SessionBar") {
		return mainwin->getDaily()->sessionBar();
	}
	qDebug() << "Request for unknown MyWebPage plugin";
	return new QWidget();
}
