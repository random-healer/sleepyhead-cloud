/*
 Overview GUI Implementation
 Copyright (c)2011 Mark Watkins <jedimark@users.sourceforge.net>
 License: GPL
*/

#include <QCalendarWidget>
#include <QTextCharFormat>
#include <QSystemLocale>
#include <QDebug>
#include <QDateTimeEdit>
#include <QCalendarWidget>
#include <QFileDialog>
#include "overview.h"
#include "ui_overview.h"
#include "Graphs/gXAxis.h"
#include "Graphs/gLineChart.h"
#include "Graphs/gYAxis.h"

Overview::Overview(QWidget *parent,Profile * _profile,gGraphView * shared) :
    QWidget(parent),
    ui(new Ui::Overview),
    profile(_profile),
    m_shared(shared)
{
    ui->setupUi(this);
    Q_ASSERT(profile!=NULL);


    // Create dummy day & session for holding eventlists.
    //day=new Day(mach);

    layout=new QHBoxLayout(ui->graphArea);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(0,0,0,0);
    ui->graphArea->setLayout(layout);
    ui->graphArea->setAutoFillBackground(false);

    GraphView=new gGraphView(ui->graphArea,m_shared);
    GraphView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    scrollbar=new MyScrollBar(ui->graphArea);
    scrollbar->setOrientation(Qt::Vertical);
    scrollbar->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Expanding);
    scrollbar->setMaximumWidth(20);

    GraphView->setScrollBar(scrollbar);
    layout->addWidget(GraphView,1);
    layout->addWidget(scrollbar,0);

    layout->layout();

    AHI=new gGraph(GraphView,"AHI",default_height,0);
    UC=new gGraph(GraphView,"Usage",default_height,0);
    PR=new gGraph(GraphView,"Pressure",default_height,0);
    SET=new gGraph(GraphView,"Settings",default_height,0);
    LK=new gGraph(GraphView,"Leaks",default_height,0);

    uc=new SummaryChart(profile,"Hours",GT_BAR);
    uc->addSlice("",QColor("green"),ST_HOURS);
    UC->AddLayer(new gYAxis(),LayerLeft,gYAxis::Margin);
    gXAxis *gx=new gXAxis();
    gx->setUtcFix(true);
    UC->AddLayer(gx,LayerBottom,0,gXAxis::Margin);
    UC->AddLayer(uc);
    UC->AddLayer(new gXGrid());


    bc=new SummaryChart(profile,"AHI",GT_BAR);
    bc->addSlice(CPAP_Hypopnea,QColor("blue"),ST_CPH);
    bc->addSlice(CPAP_Apnea,QColor("dark green"),ST_CPH);
    bc->addSlice(CPAP_Obstructive,QColor("#40c0ff"),ST_CPH);
    bc->addSlice(CPAP_ClearAirway,QColor("purple"),ST_CPH);
    AHI->AddLayer(new gYAxis(),LayerLeft,gYAxis::Margin);
    gx=new gXAxis();
    gx->setUtcFix(true);
    AHI->AddLayer(gx,LayerBottom,0,gXAxis::Margin);
    AHI->AddLayer(bc);
    AHI->AddLayer(new gXGrid());

    set=new SummaryChart(profile,"",GT_LINE);
    //set->addSlice("SysOneResistSet",QColor("grey"),ST_SETAVG);
    set->addSlice("HumidSet",QColor("blue"),ST_SETAVG);
    set->addSlice("FlexSet",QColor("red"),ST_SETAVG);
    //set->addSlice("PAPMode",QColor("red"),ST_SETAVG);
    SET->forceMaxY(6);
    SET->AddLayer(new gYAxis(),LayerLeft,gYAxis::Margin);
    gx=new gXAxis();
    gx->setUtcFix(true);
    SET->AddLayer(gx,LayerBottom,0,gXAxis::Margin);
    SET->AddLayer(set);
    SET->AddLayer(new gXGrid());

    pr=new SummaryChart(profile,"cmH2O",GT_LINE);
    PR->forceMinY(4.0);
    //PR->forceMaxY(12.0);
    pr->addSlice(CPAP_Pressure,QColor("dark green"),ST_WAVG);
    pr->addSlice(CPAP_Pressure,QColor("orange"),ST_MIN);
    pr->addSlice(CPAP_Pressure,QColor("red"),ST_MAX);
    //pr->addSlice(CPAP_Pressure,QColor("grey"),ST_90P);
    pr->addSlice(CPAP_EPAP,QColor("light green"),ST_MIN);
    pr->addSlice(CPAP_IPAP,QColor("light blue"),ST_MAX);

    PR->AddLayer(new gYAxis(),LayerLeft,gYAxis::Margin);
    gx=new gXAxis();
    gx->setUtcFix(true);
    PR->AddLayer(gx,LayerBottom,0,gXAxis::Margin);
    PR->AddLayer(pr);
    PR->AddLayer(new gXGrid());

    lk=new SummaryChart(profile,"Avg Leak",GT_LINE);
    lk->addSlice(CPAP_Leak,QColor("dark grey"),ST_90P);
    lk->addSlice(CPAP_Leak,QColor("dark blue"),ST_WAVG);
    //lk->addSlice(CPAP_Leak,QColor("dark yellow"));
    //pr->addSlice(CPAP_IPAP,QColor("red"));
    LK->AddLayer(new gYAxis(),LayerLeft,gYAxis::Margin);
    gx=new gXAxis();
    gx->setUtcFix(true);
    LK->AddLayer(gx,LayerBottom,0,gXAxis::Margin);
    LK->AddLayer(lk);
    LK->AddLayer(new gXGrid());

    NPB=new gGraph(GraphView,"% in PB",default_height,0);
    NPB->AddLayer(npb=new SummaryChart(profile,"% PB",GT_BAR));
    npb->addSlice(CPAP_CSR,QColor("light green"),ST_SPH);
    NPB->AddLayer(new gYAxis(),LayerLeft,gYAxis::Margin);
    gx=new gXAxis();
    gx->setUtcFix(true);
    NPB->AddLayer(gx,LayerBottom,0,gXAxis::Margin);
    NPB->AddLayer(new gXGrid());


    QLocale locale=QLocale::system();
    QString shortformat=locale.dateFormat(QLocale::ShortFormat);
    if (!shortformat.toLower().contains("yyyy")) {
        shortformat.replace("yy","yyyy");
    }
    ui->dateStart->setDisplayFormat(shortformat);
    ui->dateEnd->setDisplayFormat(shortformat);

    QTextCharFormat format = ui->dateStart->calendarWidget()->weekdayTextFormat(Qt::Saturday);
    format.setForeground(QBrush(Qt::black, Qt::SolidPattern));
    ui->dateStart->calendarWidget()->setWeekdayTextFormat(Qt::Saturday, format);
    ui->dateStart->calendarWidget()->setWeekdayTextFormat(Qt::Sunday, format);
    ui->dateEnd->calendarWidget()->setWeekdayTextFormat(Qt::Saturday, format);
    ui->dateEnd->calendarWidget()->setWeekdayTextFormat(Qt::Sunday, format);

    connect(ui->dateStart->calendarWidget(),SIGNAL(currentPageChanged(int,int)),SLOT(on_dateStart_currentPageChanged(int,int)));
    connect(ui->dateEnd->calendarWidget(),SIGNAL(currentPageChanged(int,int)),SLOT(on_dateEnd_currentPageChanged(int,int)));
    report=NULL;
}
Overview::~Overview()
{
    disconnect(this,SLOT(on_dateStart_currentPageChanged(int,int)));
    disconnect(this,SLOT(on_dateEnd_currentPageChanged(int,int)));
    if (report) {
        report->close();
        delete report;
    }
    //delete day;
    delete ui;
}
void Overview::ReloadGraphs()
{
    ui->dateStart->setDate(profile->FirstDay());
    ui->dateEnd->setDate(profile->LastDay());
    GraphView->setDay(NULL);

}

void Overview::RedrawGraphs()
{
    GraphView->updateGL();
}

void Overview::UpdateCalendarDay(QDateEdit * dateedit,QDate date)
{
    QCalendarWidget *calendar=dateedit->calendarWidget();
    QTextCharFormat bold;
    QTextCharFormat cpapcol;
    QTextCharFormat normal;
    QTextCharFormat oxiday;
    bold.setFontWeight(QFont::Bold);
    cpapcol.setForeground(QBrush(Qt::blue, Qt::SolidPattern));
    cpapcol.setFontWeight(QFont::Bold);
    oxiday.setForeground(QBrush(Qt::red, Qt::SolidPattern));
    oxiday.setFontWeight(QFont::Bold);
    bool hascpap=profile->GetDay(date,MT_CPAP)!=NULL;
    bool hasoxi=profile->GetDay(date,MT_OXIMETER)!=NULL;

    if (hascpap) {
        if (hasoxi) {
            calendar->setDateTextFormat(date,oxiday);
        } else {
            calendar->setDateTextFormat(date,cpapcol);
        }
    } else if (profile->GetDay(date)) {
        calendar->setDateTextFormat(date,bold);
    } else {
        calendar->setDateTextFormat(date,normal);
    }
    calendar->setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
}
void Overview::on_dateStart_currentPageChanged(int year, int month)
{
    QDate d(year,month,1);
    int dom=d.daysInMonth();

    for (int i=1;i<=dom;i++) {
        d=QDate(year,month,i);
        UpdateCalendarDay(ui->dateStart,d);
    }
}
void Overview::on_dateEnd_currentPageChanged(int year, int month)
{
    QDate d(year,month,1);
    int dom=d.daysInMonth();

    for (int i=1;i<=dom;i++) {
        d=QDate(year,month,i);
        UpdateCalendarDay(ui->dateEnd,d);
    }
}


void Overview::on_dateEnd_dateChanged(const QDate &date)
{
    qint64 d1=qint64(QDateTime(ui->dateStart->date(),QTime(0,0,0),Qt::UTC).toTime_t())*1000L;
    qint64 d2=qint64(QDateTime(date,QTime(23,59,59),Qt::UTC).toTime_t())*1000L;
    GraphView->SetXBounds(d1,d2);

}

void Overview::on_dateStart_dateChanged(const QDate &date)
{
    qint64 d1=qint64(QDateTime(date,QTime(0,0,0),Qt::UTC).toTime_t())*1000L;
    qint64 d2=qint64(QDateTime(ui->dateEnd->date(),QTime(23,59,59),Qt::UTC).toTime_t())*1000L;
    GraphView->SetXBounds(d1,d2);

}

void Overview::on_toolButton_clicked()
{
    qint64 d1=qint64(QDateTime(ui->dateStart->date(),QTime(0,0,0),Qt::UTC).toTime_t())*1000L;
    qint64 d2=qint64(QDateTime(ui->dateEnd->date(),QTime(23,59,59),Qt::UTC).toTime_t())*1000L;
    GraphView->SetXBounds(d1,d2);
}

QString Overview::GetHTML()
{
    if (!report) {
        report=new Report(this,profile,m_shared,this);
        report->hide();
    }

    QString html;
    if (report) {
        bc->deselect();
        uc->deselect();
        pr->deselect();
        lk->deselect();
        npb->deselect();

        report->ReloadGraphs();
        QString reportname="overview";
        html=report->GenerateReport(reportname,ui->dateStart->date(),ui->dateEnd->date());
        if (html.isEmpty()) {
            qDebug() << "Faulty Report" << reportname;
        }
    }
    return html;
}
void Overview::on_printButton_clicked()
{

    report->Print(GetHTML());
}

void Overview::on_htmlButton_clicked()
{
    QString html=GetHTML();
    QString filename=QFileDialog::getSaveFileName(this,tr("Save HTML Report"),pref.Get("{home}"),tr("HTML Documents (*.html)"));
    if (!filename.isEmpty()) {
        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        QByteArray ba;
        ba.append(html);
        file.write(ba);
        file.close();
    }

}
void Overview::ResetGraphLayout()
{
    GraphView->resetLayout();
}
