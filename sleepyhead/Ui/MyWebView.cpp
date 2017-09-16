#include "MyWebView.h"

MyWebView::MyWebView(QWidget *parent) : QWebView(parent), m_page(this)
{
	// Set the page of our own PageView class, MyPageView,
	// because only objects of this class will handle
	// object-tags correctly.
	setPage(&m_page);
}
