#pragma once

#include <QtWebKitWidgets>

#include "MyWebPage.h"

class MyWebView : public QWebView
{
private:
	MyWebPage m_page;
public:
	MyWebView(QWidget *parent = 0);
};
