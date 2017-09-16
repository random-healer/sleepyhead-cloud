#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QStringList>
#include <QtWebKitWidgets>

class MyWebPage : public QWebPage
{
public:
	MyWebPage(QObject *parent = 0);
protected:
	QObject *createPlugin(const QString & classid, const QUrl & url, const QStringList & paramNames, const QStringList & paramValues);
};
