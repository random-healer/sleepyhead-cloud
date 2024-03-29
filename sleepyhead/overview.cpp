/* Overview GUI Implementation
 *
 * Copyright (c) 2011-2016 Mark Watkins <jedimark@users.sourceforge.net>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of the Linux
 * distribution for more details. */

#include <QCalendarWidget>
#include <QTextCharFormat>
//#include <QSystemLocale>
#include <QDebug>
#include <QDateTimeEdit>
#include <QCalendarWidget>
#include <QFileDialog>
#include <QMessageBox>
//#include <QProgressBar>

#include "Common/DateTimeUtils.h"
#include "Common/GraphName.h"

#include "SleepLib/profiles.h"
#include "overview.h"
#include "ui_overview.h"
#include "Graphs/gXAxis.h"
#include "Graphs/gLineChart.h"
#include "Graphs/gYAxis.h"

#include "mainwindow.h"
extern MainWindow *mainwin;
//extern QProgressBar * qprogress;

using namespace SleepyHead::Common;

Overview::Overview(QWidget *parent, gGraphView *shared) :
    QWidget(parent),
    ui(new Ui::Overview),
    m_shared(shared)
{
    ui->setupUi(this);

    // Set Date controls locale to 4 digit years
    QLocale locale = QLocale::system();
    QString shortformat = locale.dateFormat(QLocale::ShortFormat);

    if (!shortformat.toLower().contains("yyyy")) {
        shortformat.replace("yy", "yyyy");
    }

    ui->dateStart->setDisplayFormat(shortformat);
    ui->dateEnd->setDisplayFormat(shortformat);

	Qt::DayOfWeek dow = DateTimeUtils::firstDayOfWeekFromLocale();

    ui->dateStart->calendarWidget()->setFirstDayOfWeek(dow);
    ui->dateEnd->calendarWidget()->setFirstDayOfWeek(dow);


    // Stop both calendar drop downs highlighting weekends in red
    QTextCharFormat format = ui->dateStart->calendarWidget()->weekdayTextFormat(Qt::Saturday);
    format.setForeground(QBrush(CommonColors::COLOR_Black, Qt::SolidPattern));
    ui->dateStart->calendarWidget()->setWeekdayTextFormat(Qt::Saturday, format);
    ui->dateStart->calendarWidget()->setWeekdayTextFormat(Qt::Sunday, format);
    ui->dateEnd->calendarWidget()->setWeekdayTextFormat(Qt::Saturday, format);
    ui->dateEnd->calendarWidget()->setWeekdayTextFormat(Qt::Sunday, format);

    // Connect the signals to update which days have CPAP data when the month is changed
    connect(ui->dateStart->calendarWidget(), SIGNAL(currentPageChanged(int, int)),
            SLOT(dateStart_currentPageChanged(int, int)));
    connect(ui->dateEnd->calendarWidget(), SIGNAL(currentPageChanged(int, int)),
            SLOT(dateEnd_currentPageChanged(int, int)));

    QVBoxLayout *framelayout = new QVBoxLayout;
    ui->graphArea->setLayout(framelayout);

    QFrame *border = new QFrame(ui->graphArea);

    framelayout->setMargin(1);
    border->setFrameShape(QFrame::StyledPanel);
    framelayout->addWidget(border,1);

    // Create the horizontal layout to hold the GraphView object and it's scrollbar
    layout = new QHBoxLayout(border);
    layout->setSpacing(0); // remove the ugly margins/spacing
    layout->setMargin(0);
    layout->setContentsMargins(0, 0, 0, 0);
    border->setLayout(layout);
    border->setAutoFillBackground(false);

    // Create the GraphView Object
    GraphView = new gGraphView(ui->graphArea, m_shared);
    GraphView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    GraphView->setEmptyText(STR_Empty_NoData);

    // Create the custom scrollbar and attach to GraphView
    scrollbar = new MyScrollBar(ui->graphArea);
    scrollbar->setOrientation(Qt::Vertical);
    scrollbar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    scrollbar->setMaximumWidth(20);
    GraphView->setScrollBar(scrollbar);


    // Add the graphView and scrollbar to the layout.
    layout->addWidget(GraphView, 1);
    layout->addWidget(scrollbar, 0);
    layout->layout();

    dateLabel = new MyLabel(this);
    dateLabel->setAlignment(Qt::AlignVCenter);
    dateLabel->setText("[Date Widget]");
    QFont font = dateLabel->font();
    font.setPointSizeF(font.pointSizeF()*1.3F);
    dateLabel->setFont(font);
    QPalette palette = dateLabel->palette();
    palette.setColor(QPalette::Base, Qt::blue);
    dateLabel->setPalette(palette);

    ui->dateLayout->addWidget(dateLabel,1);



//    uc = new SummaryChart(UnitTime::STR_UNIT_Hours, GT_BAR);
//    uc->addSlice(NoChannel, COLOR_Green, ST_HOURS);
//    UC->AddLayer(uc);

  /*  return;

    // TODO: Automate graph creation process





    float percentile = p_profile->general->prefCalcPercentile() / 100.0;
    int mididx = p_profile->general->prefCalcMiddle();
    SummaryType ST_mid;

    if (mididx == 0) { ST_mid = ST_PERC; }
    if (mididx == 1) { ST_mid = ST_WAVG; }
    if (mididx == 2) { ST_mid = ST_AVG; }

    SummaryType ST_max = p_profile->general->prefCalcMax() ? ST_PERC : ST_MAX;
    const EventDataType maxperc = 0.995F;

    US = createGraph(STR_GRAPH_SessionTimes, tr("Session Times"), tr("Session Times\n(hours)"), YT_Time);
    SET = createGraph("Settings", STR_TR_Settings, STR_TR_Settings);


    TGMV = createGraph(schema::channel[CPAP_TgMV].code(), schema::channel[CPAP_TgMV].label(), tr("Target\nVentilation\n(L/min)"));
    PTB = createGraph(schema::channel[CPAP_PTB].code(), schema::channel[CPAP_PTB].label(), tr("Patient\nTriggered\nBreaths\n(%)"));
    SES = createGraph(STR_GRAPH_Sessions, STR_TR_Sessions, STR_TR_Sessions + tr("\n(count)"));


    ahihr = new SummaryChart(UnitPerMinute::STR_UNIT_EventsPerHour, GT_POINTS);
    ahihr->addSlice(ahicode, COLOR_Blue, ST_MAX);
    ahihr->addSlice(ahicode, COLOR_Orange, ST_WAVG);
    AHIHR->AddLayer(ahihr);

    weight = new SummaryChart(STR_TR_Weight, GT_POINTS);
    weight->setMachineType(MT_JOURNAL);
    weight->addSlice(Journal_Weight, COLOR_Black, ST_SETAVG);
    WEIGHT->AddLayer(weight);

    bmi = new SummaryChart(STR_TR_BMI, GT_POINTS);
    bmi->setMachineType(MT_JOURNAL);
    bmi->addSlice(Journal_BMI, COLOR_DarkBlue, ST_SETAVG);
    BMI->AddLayer(bmi);

    zombie = new SummaryChart(tr("Zombie Meter"), GT_POINTS);
    zombie->setMachineType(MT_JOURNAL);
    zombie->addSlice(Journal_ZombieMeter, COLOR_DarkRed, ST_SETAVG);
    ZOMBIE->AddLayer(zombie);

    pulse = new SummaryChart(STR_TR_PulseRate, GT_POINTS);
    pulse->setMachineType(MT_OXIMETER);
    pulse->addSlice(OXI_Pulse, COLOR_Red, ST_mid, 0.5);
    pulse->addSlice(OXI_Pulse, COLOR_Pink, ST_MIN);
    pulse->addSlice(OXI_Pulse, COLOR_Orange, ST_MAX);
    PULSE->AddLayer(pulse);

    spo2 = new SummaryChart(STR_TR_SpO2, GT_POINTS);
    spo2->setMachineType(MT_OXIMETER);
    spo2->addSlice(OXI_SPO2, COLOR_Cyan, ST_mid, 0.5);
    spo2->addSlice(OXI_SPO2, COLOR_LightBlue, ST_PERC, percentile);
    spo2->addSlice(OXI_SPO2, COLOR_Blue, ST_MIN);
    SPO2->AddLayer(spo2);


    fl = new SummaryChart(STR_TR_FL, GT_POINTS);
    fl->addSlice(CPAP_FlowLimit, COLOR_Brown, ST_CPH);
    FL->AddLayer(fl);

    sa = new SummaryChart(STR_TR_SA, GT_POINTS);
    sa->addSlice(CPAP_SensAwake, COLOR_Brown, ST_CNT);
    SA->AddLayer(sa);

    us = new SummaryChart(UnitTime::STR_UNIT_Hours, GT_SESSIONS);
    us->addSlice(NoChannel, COLOR_DarkBlue, ST_HOURS);
    us->addSlice(NoChannel, COLOR_Blue, ST_SESSIONS);
    US->AddLayer(us);

    ses = new SummaryChart(STR_TR_Sessions, GT_POINTS);
    ses->addSlice(NoChannel, COLOR_Blue, ST_SESSIONS);
    SES->AddLayer(ses);

    if (ahicode == CPAP_RDI) {
        bc = new SummaryChart(STR_TR_RDI, GT_BAR);
    } else {
        bc = new SummaryChart(STR_TR_AHI, GT_BAR);
    }

    bc->addSlice(CPAP_ClearAirway, COLOR_ClearAirway, ST_CPH);
    bc->addSlice(CPAP_Obstructive, COLOR_Obstructive, ST_CPH);
    bc->addSlice(CPAP_Apnea, COLOR_Apnea, ST_CPH);
    bc->addSlice(CPAP_Hypopnea, COLOR_Hypopnea, ST_CPH);

    if (p_profile->general->calculateRDI()) {
        bc->addSlice(CPAP_RERA, COLOR_RERA, ST_CPH);
    }
//    bc->addSlice(CPAP_UserFlag1, COLOR_UserFlag1, ST_CPH);
//    bc->addSlice(CPAP_UserFlag2, COLOR_UserFlag2, ST_CPH);


    AHI->AddLayer(bc);

    set = new SummaryChart("", GT_POINTS);
    //set->addSlice(PRS1_SysOneResistSet,COLOR_Gray,ST_SETAVG);
    set->addSlice(CPAP_HumidSetting, COLOR_Blue, ST_SETWAVG);
    set->addSlice(CPAP_PresReliefLevel, COLOR_Red, ST_SETWAVG);
    set->addSlice(CPAP_PresReliefMode, COLOR_Red, ST_SETWAVG);
//    set->addSlice(RMS9_EPRLevel,COLOR_Green,ST_SETWAVG);
    //set->addSlice(INTP_SmartFlex,COLOR_Purple,ST_SETWAVG);
    SET->AddLayer(set);

    rr = new SummaryChart(tr("breaths/min"), GT_POINTS);
    rr->addSlice(CPAP_RespRate, COLOR_LightBlue, ST_MIN);
    rr->addSlice(CPAP_RespRate, COLOR_Blue, ST_mid, 0.5);
    rr->addSlice(CPAP_RespRate, COLOR_LightGreen, ST_PERC, percentile);
    rr->addSlice(CPAP_RespRate, COLOR_Green, ST_max, maxperc);
    // rr->addSlice(CPAP_RespRate,COLOR_Green,ST_MAX);
    RR->AddLayer(rr);

    tv = new SummaryChart(tr("L/b"), GT_POINTS);
    tv->addSlice(CPAP_TidalVolume, COLOR_LightBlue, ST_MIN);
    tv->addSlice(CPAP_TidalVolume, COLOR_Blue, ST_mid, 0.5);
    tv->addSlice(CPAP_TidalVolume, COLOR_LightGreen, ST_PERC, percentile);
    tv->addSlice(CPAP_TidalVolume, COLOR_Green, ST_max, maxperc);
    TV->AddLayer(tv);

    mv = new SummaryChart(UnitPerMinute::STR_UNIT_LPM, GT_POINTS);
    mv->addSlice(CPAP_MinuteVent, COLOR_LightBlue, ST_MIN);
    mv->addSlice(CPAP_MinuteVent, COLOR_Blue, ST_mid, 0.5);
    mv->addSlice(CPAP_MinuteVent, COLOR_LightGreen, ST_PERC, percentile);
    mv->addSlice(CPAP_MinuteVent, COLOR_Green, ST_max, maxperc);
    MV->AddLayer(mv);

    // should merge...
    tgmv = new SummaryChart(UnitPerMinute::STR_UNIT_LPM, GT_POINTS);
    tgmv->addSlice(CPAP_TgMV, COLOR_LightBlue, ST_MIN);
    tgmv->addSlice(CPAP_TgMV, COLOR_Blue, ST_mid, 0.5);
    tgmv->addSlice(CPAP_TgMV, COLOR_LightGreen, ST_PERC, percentile);
    tgmv->addSlice(CPAP_TgMV, COLOR_Green, ST_max, maxperc);
    TGMV->AddLayer(tgmv);

    ptb = new SummaryChart(tr("%PTB"), GT_POINTS);
    ptb->addSlice(CPAP_PTB, COLOR_Yellow, ST_MIN);
    ptb->addSlice(CPAP_PTB, COLOR_Blue, ST_mid, 0.5);
    ptb->addSlice(CPAP_PTB, COLOR_LightGray, ST_PERC, percentile);
    ptb->addSlice(CPAP_PTB, COLOR_Orange, ST_WAVG);
    PTB->AddLayer(ptb);

    pr = new SummaryChart(STR_TR_Pressure, GT_POINTS);
    // Added in summarychart.. Slightly annoying..
    PR->AddLayer(pr);


    totlk = new SummaryChart(STR_TR_TotalLeaks, GT_POINTS);
    totlk->addSlice(CPAP_LeakTotal, COLOR_LightBlue, ST_mid, 0.5);
    totlk->addSlice(CPAP_LeakTotal, COLOR_DarkGray, ST_PERC, percentile);
    totlk->addSlice(CPAP_LeakTotal, COLOR_Gray, ST_max, maxperc);
    //tot->addSlice(CPAP_Leak, COLOR_DarkBlue, ST_WAVG);
    //tot->addSlice(CPAP_Leak, COLOR_DarkYellow);
    TOTLK->AddLayer(totlk);


    NLL->AddLayer(nll = new SummaryChart(tr("% %1").arg(schema::channel[CPAP_LargeLeak].fullname()), GT_POINTS));
    nll->addSlice(CPAP_LargeLeak, schema::channel[CPAP_LargeLeak].defaultColor(), ST_SPH);
    // <--- The code to the previous marker is crap

    AHI->setPinned(false);
    SES->setRecMinY(1);
    SET->setRecMinY(0);

    //SET->setRecMaxY(5);

    */
    RebuildGraphs(false);

    ui->rangeCombo->setCurrentIndex(p_profile->general->lastOverviewRange());

    icon_on = new QIcon(":/icons/session-on.png");
    icon_off = new QIcon(":/icons/session-off.png");

    GraphView->resetLayout();
    GraphView->LoadSettings("Overview"); //no trans

    GraphView->setEmptyImage(QPixmap(":/docs/sheep.png"));

    connect(GraphView, SIGNAL(updateCurrentTime(double)), this, SLOT(on_LineCursorUpdate(double)));
    connect(GraphView, SIGNAL(updateRange(double,double)), this, SLOT(on_RangeUpdate(double,double)));

    connect(GraphView, SIGNAL(GraphsChanged()), this, SLOT(updateGraphCombo()));
}
Overview::~Overview()
{
    delete ui;
//    delete icon_on;
//    delete icon_off;
}

void Overview::RebuildGraphs(bool reset)
{
    qint64 minx, maxx;
    if (reset) {
        GraphView->GetXBounds(minx, maxx);
    }

    GraphView->trashGraphs(true);
    ChannelID ahicode = p_profile->general->calculateRDI() ? CPAP_RDI : CPAP_AHI;

    if (ahicode == CPAP_RDI) {
        AHI = createGraph("AHIBreakdown", STR_TR_RDI, tr("Respiratory\nDisturbance\nIndex"));
    } else {
        AHI = createGraph("AHIBreakdown", STR_TR_AHI, tr("Apnea\nHypopnea\nIndex"));
    }


    ahi = new gAHIChart();
    AHI->AddLayer(ahi);

	UC = createGraph(GraphName::STR_GRAPH_Usage, tr("Usage"), tr("Usage\n(hours)"));
    UC->AddLayer(uc = new gUsageChart());

    STG = createGraph("New Session", tr("Session Times"), tr("Session Times"),  YT_Time);
    stg = new gSessionTimesChart();
    STG->AddLayer(stg);

    PR = createGraph("Pressure Settings", STR_TR_Pressure, STR_TR_Pressure + "\n(" + STR_UNIT_CMH2O + ")");
    pres = new gPressureChart();
    PR->AddLayer(pres);

    TTIA = createGraph("TTIA", tr("Total Time in Apnea"), tr("Total Time in Apnea\n(Minutes)"));
    ttia = new gTTIAChart();
    TTIA->AddLayer(ttia);

//    LK = createGraph("Leaks", STR_TR_Leaks, STR_TR_UnintentionalLeaks + "\n(" + UnitPerMinute::STR_UNIT_LPM + ")");
//    LK->AddLayer(new gSummaryChart(CPAP_Leak, MT_CPAP));

//    TOTLK = createGraph("TotalLeaks", STR_TR_TotalLeaks, STR_TR_TotalLeaks + "\n(" + UnitPerMinute::STR_UNIT_LPM + ")");
//    TOTLK->AddLayer(new gSummaryChart(CPAP_LeakTotal, MT_CPAP));

//    NLL = createGraph("TimeInLL", tr("% in %1").arg(schema::channel[CPAP_LargeLeak].label()), tr("Large Leaks\n(% of night)"));
//    NLL->AddLayer(nll = new gSummaryChart("TimeInLL", MT_CPAP));
//    nll->addCalc(CPAP_LargeLeak, ST_SPH);

//    RR = createGraph(schema::channel[CPAP_RespRate].code(), schema::channel[CPAP_RespRate].label(), schema::channel[CPAP_RespRate].fullname()+"\n"+schema::channel[CPAP_RespRate].units());
//    RR->AddLayer(new gSummaryChart(CPAP_RespRate, MT_CPAP));
//    TV = createGraph(schema::channel[CPAP_TidalVolume].code(),schema::channel[CPAP_TidalVolume].label(), tr("Tidal\nVolume\n(ml)"));
//    TV->AddLayer(new gSummaryChart(CPAP_TidalVolume, MT_CPAP));
//    MV = createGraph(schema::channel[CPAP_MinuteVent].code(), schema::channel[CPAP_MinuteVent].label(), tr("Minute\nVentilation\n(L/min)"));
//    MV->AddLayer(new gSummaryChart(CPAP_MinuteVent, MT_CPAP));
//    FL = createGraph(schema::channel[CPAP_FLG].code(), schema::channel[CPAP_FLG].label(), STR_TR_FlowLimit);
//    FL->AddLayer(new gSummaryChart(CPAP_FLG, MT_CPAP));
//    SN = createGraph(schema::channel[CPAP_Snore].code(), schema::channel[CPAP_Snore].label(), schema::channel[CPAP_Snore].fullname()+"\n"+schema::channel[CPAP_Snore].units());
//    SN->AddLayer(new gSummaryChart(CPAP_Snore, MT_CPAP));

    QHash<ChannelID, schema::Channel *>::iterator chit;
    QHash<ChannelID, schema::Channel *>::iterator chit_end = schema::channel.channels.end();
    for (chit = schema::channel.channels.begin(); chit != chit_end; ++chit) {
        schema::Channel * chan = chit.value();

        if (chan->showInOverview()) {
            ChannelID code = chan->id();
            QString name = chan->fullname();
            if (name.length() > 16) name = chan->label();
            gGraph *G = createGraph(chan->code(), name, chan->description());
            if ((chan->type() == schema::FLAG) || (chan->type() == schema::MINOR_FLAG)) {
                gSummaryChart * sc = new gSummaryChart(chan->code(), MT_CPAP);
                sc->addCalc(code, ST_CPH, schema::channel[code].defaultColor());
                G->AddLayer(sc);
            } else if (chan->type() == schema::SPAN) {
                gSummaryChart * sc = new gSummaryChart(chan->code(), MT_CPAP);
                sc->addCalc(code, ST_SPH, schema::channel[code].defaultColor());
                G->AddLayer(sc);
            } else if (chan->type() == schema::WAVEFORM) {
                G->AddLayer(new gSummaryChart(code, chan->machtype()));
            } else if (chan->type() == schema::UNKNOWN) {
                gSummaryChart * sc = new gSummaryChart(chan->code(), MT_CPAP);
                sc->addCalc(code, ST_CPH, schema::channel[code].defaultColor());
                G->AddLayer(sc);
            }
        }

    }

/*    PULSE = createGraph(schema::channel[OXI_Pulse].code(), schema::channel[OXI_Pulse].label(), STR_TR_PulseRate + "\n(" + UnitPerMinute::STR_UNIT_BPM + ")");
    PULSE->AddLayer(new gSummaryChart(OXI_Pulse, MT_OXIMETER));

    SPO2 = createGraph(schema::channel[OXI_SPO2].code(), schema::channel[OXI_SPO2].label(), tr("Oxygen Saturation\n(%)"));
    SPO2->AddLayer(new gSummaryChart(OXI_SPO2, MT_OXIMETER));


    NPB = createGraph("TimeInPB", tr("% in %1").arg(schema::channel[CPAP_CSR].label()), tr("%1\n(% of night)").arg(schema::channel[CPAP_LargeLeak].description()));
    NPB->AddLayer(npb = new gSummaryChart(tr("% PB"), MT_CPAP));
    npb->addCalc(CPAP_CSR, ST_SPH);

    if (ahicode == CPAP_RDI) {
        AHIHR = createGraph(STR_GRAPH_PeakAHI, tr("Peak RDI"), tr("Peak RDI\nShows RDI Clusters\n(RDI/hr)"));
        AHIHR->AddLayer(new gSummaryChart(CPAP_RDI, MT_CPAP));
    } else {
        AHIHR = createGraph(STR_GRAPH_PeakAHI, tr("Peak AHI"), tr("Peak AHI\nShows AHI Clusters\n(AHI/hr)"));
        AHIHR->AddLayer(new gSummaryChart(CPAP_AHI, MT_CPAP));
    } */

	WEIGHT = createGraph(GraphName::STR_GRAPH_Weight, STR_TR_Weight, STR_TR_Weight, YT_Weight);
	BMI = createGraph(GraphName::STR_GRAPH_BMI, STR_TR_BMI, tr("Body\nMass\nIndex"));
	ZOMBIE = createGraph(GraphName::STR_GRAPH_Zombie, STR_TR_Zombie, tr("How you felt\n(0-10)"));

    if (reset) {
//        GraphView->setDay(nullptr);
        GraphView->resetLayout();
        GraphView->setDay(nullptr);
//        GraphView->resetLayout();
        GraphView->SetXBounds(minx, maxx, 0, false);
        GraphView->resetLayout();
        updateGraphCombo();
    }


}

void Overview::closeEvent(QCloseEvent *event)
{
    GraphView->SaveSettings("Overview");//no trans
    disconnect(this, SLOT(dateStart_currentPageChanged(int, int)));
    disconnect(this, SLOT(dateEnd_currentPageChanged(int, int)));
    QWidget::closeEvent(event);
}

gGraph *Overview::createGraph(QString code, QString name, QString units, YTickerType yttype)
{
    int default_height = p_profile->appearance->graphHeight();
    gGraph *g = new gGraph(code, GraphView, name, units, default_height, 0);

    gYAxis *yt;

    switch (yttype) {
    case YT_Time:
        yt = new gYAxisTime(true); // Time scale
        break;

    case YT_Weight:
        yt = new gYAxisWeight(p_profile->general->unitSystem());
        break;

    default:
        yt = new gYAxis(); // Plain numeric scale
        break;
    }

    g->AddLayer(yt, LayerLeft, gYAxis::Margin);
    gXAxisDay *x = new gXAxisDay();
    g->AddLayer(x, LayerBottom, 0, gXAxisDay::Margin);
    g->AddLayer(new gXGrid());
    return g;
}

void Overview::on_LineCursorUpdate(double time)
{
    if (time > 1) {
        // even though the generated string is displayed to the user
        // no time zone conversion is neccessary, so pass UTC
        // to prevent QT from automatically converting to local time
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(time, Qt::UTC);
        QString txt = dt.toString("dd MMM yyyy (dddd)");
        dateLabel->setText(txt);
    } else dateLabel->setText(QString(GraphView->emptyText()));
}

void Overview::on_RangeUpdate(double minx, double /* maxx */)
{
    if (minx > 1) {
        dateLabel->setText(GraphView->getRangeString());
    } else {
        dateLabel->setText(QString(GraphView->emptyText()));
    }
}

void Overview::ReloadGraphs()
{
    GraphView->setDay(nullptr);
    updateCube();

    on_rangeCombo_activated(ui->rangeCombo->currentIndex());
}

void Overview::updateGraphCombo()
{
    ui->graphCombo->clear();
    gGraph *g;
    //    ui->graphCombo->addItem("Show All Graphs");
    //    ui->graphCombo->addItem("Hide All Graphs");
    //    ui->graphCombo->addItem("---------------");

    for (int i = 0; i < GraphView->size(); i++) {
        g = (*GraphView)[i];

        if (g->isEmpty()) { continue; }

        if (g->visible()) {
            ui->graphCombo->addItem(*icon_on, g->title(), true);
        } else {
            ui->graphCombo->addItem(*icon_off, g->title(), false);
        }
    }

    ui->graphCombo->setCurrentIndex(0);
    updateCube();
}

void Overview::ResetGraphs()
{
    //qint64 st,et;
    //GraphView->GetXBounds(st,et);
    QDate start = ui->dateStart->date();
    QDate end = ui->dateEnd->date();
    GraphView->setDay(nullptr);
    updateCube();

    if (start.isValid() && end.isValid()) {
        setRange(start, end);
    }

    //GraphView->SetXBounds(st,et);
}

void Overview::ResetGraph(QString name)
{
    gGraph *g = GraphView->findGraph(name);

    if (!g) { return; }

    g->setDay(nullptr);
    GraphView->redraw();
}

void Overview::RedrawGraphs()
{
    GraphView->redraw();
}

void Overview::UpdateCalendarDay(QDateEdit *dateedit, QDate date)
{
    QCalendarWidget *calendar = dateedit->calendarWidget();
    QTextCharFormat bold;
    QTextCharFormat cpapcol;
    QTextCharFormat normal;
    QTextCharFormat oxiday;
    bold.setFontWeight(QFont::Bold);
    cpapcol.setForeground(QBrush(Qt::blue, Qt::SolidPattern));
    cpapcol.setFontWeight(QFont::Bold);
    oxiday.setForeground(QBrush(Qt::red, Qt::SolidPattern));
    oxiday.setFontWeight(QFont::Bold);
    bool hascpap = p_profile->FindDay(date, MT_CPAP) != nullptr;
    bool hasoxi = p_profile->FindDay(date, MT_OXIMETER) != nullptr;
    //bool hasjournal=p_profile->GetDay(date,MT_JOURNAL)!=nullptr;

    if (hascpap) {
        if (hasoxi) {
            calendar->setDateTextFormat(date, oxiday);
        } else {
            calendar->setDateTextFormat(date, cpapcol);
        }
    } else if (p_profile->GetDay(date)) {
        calendar->setDateTextFormat(date, bold);
    } else {
        calendar->setDateTextFormat(date, normal);
    }

    calendar->setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
}
void Overview::dateStart_currentPageChanged(int year, int month)
{
    QDate d(year, month, 1);
    int dom = d.daysInMonth();

    for (int i = 1; i <= dom; i++) {
        d = QDate(year, month, i);
        UpdateCalendarDay(ui->dateStart, d);
    }
}
void Overview::dateEnd_currentPageChanged(int year, int month)
{
    QDate d(year, month, 1);
    int dom = d.daysInMonth();

    for (int i = 1; i <= dom; i++) {
        d = QDate(year, month, i);
        UpdateCalendarDay(ui->dateEnd, d);
    }
}


void Overview::on_dateEnd_dateChanged(const QDate &date)
{
    qint64 d1 = qint64(QDateTime(ui->dateStart->date(), QTime(0, 10, 0), Qt::UTC).toTime_t()) * 1000L;
    qint64 d2 = qint64(QDateTime(date, QTime(23, 0, 0), Qt::UTC).toTime_t()) * 1000L;
    GraphView->SetXBounds(d1, d2);
    ui->dateStart->setMaximumDate(date);
}

void Overview::on_dateStart_dateChanged(const QDate &date)
{
    qint64 d1 = qint64(QDateTime(date, QTime(0, 10, 0), Qt::UTC).toTime_t()) * 1000L;
    qint64 d2 = qint64(QDateTime(ui->dateEnd->date(), QTime(23, 0, 0), Qt::UTC).toTime_t()) * 1000L;
    GraphView->SetXBounds(d1, d2);
    ui->dateEnd->setMinimumDate(date);
}

void Overview::on_toolButton_clicked()
{
    qint64 d1 = qint64(QDateTime(ui->dateStart->date(), QTime(0, 10, 0), Qt::UTC).toTime_t()) * 1000L;
    qint64 d2 = qint64(QDateTime(ui->dateEnd->date(), QTime(23, 00, 0), Qt::UTC).toTime_t()) * 1000L;
    GraphView->SetXBounds(d1, d2);
}

//void Overview::on_printButton_clicked()
//{
//    mainwin->PrintReport(GraphView,STR_TR_Overview); // Must be translated the same as PrintReport checks.
//}

void Overview::ResetGraphLayout()
{
    GraphView->resetLayout();
}


/*void Overview::on_printDailyButton_clicked()
{
    qint64 st,et;
    GraphView->GetXBounds(st,et);

    QDate s1=QDateTime::fromTime_t(st/1000L).date();
    QDate s2=QDateTime::fromTime_t(et/1000L).date();

    int len=p_profile->countDays(MT_UNKNOWN,s1,s2);
    if (len>7) {
        if (QMessageBox::question(this, "Woah!", "Do you really want to print "+QString::number(len)+" days worth of Daily reports,\n from "+s1.toString(Qt::SystemLocaleShortDate)+" to "+s2.toString(Qt::SystemLocaleShortDate)+"?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::No) {
            return;
        }
        if (len>14) {
            int weeks=len/7;
            if (QMessageBox::question(this, "Hold Up!", "We are talking about over "+QString::number(weeks)+" weeks of information.\n\nThis will likely take a very long time, and a heck of a lot of paper if your not printing to a PDF file.\n\nAre you really sure?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::No) {
                return;
            }
            if (len>31) {
                if (QMessageBox::question(this, "Are you serious!!??", "We are talking about printing a lot of information.\n\nIf your not printing to a PDF file, you must really hate trees.\n\nAre you really REALLY sure?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::No) {
                    return;
                }
            }
        }

        mainwin->Notify("I'm not going to nag you any more, but it would probably help if I implemented this feature.. ;)");


    } else mainwin->Notify("If this was implemented yet, You'd be able to print multiple daily reports right now.");

}*/

void Overview::on_rangeCombo_activated(int index)
{
    p_profile->general->setLastOverviewRange(index);
    ui->dateStart->setMinimumDate(p_profile->FirstDay());
    ui->dateEnd->setMaximumDate(p_profile->LastDay());

    QDate end = p_profile->LastDay();
    QDate start;

    if (index == 8) { // Custom
        ui->dateStartLabel->setEnabled(true);
        ui->dateEndLabel->setEnabled(true);
        ui->dateEnd->setEnabled(true);
        ui->dateStart->setEnabled(true);

        ui->dateStart->setMaximumDate(ui->dateEnd->date());
        ui->dateEnd->setMinimumDate(ui->dateStart->date());
        return;
    }

    ui->dateEnd->setEnabled(false);
    ui->dateStart->setEnabled(false);
    ui->dateStartLabel->setEnabled(false);
    ui->dateEndLabel->setEnabled(false);

    if (index == 0) {
        start = end.addDays(-6);
    } else if (index == 1) {
        start = end.addDays(-13);
    } else if (index == 2) {
        start = end.addMonths(-1).addDays(1);
    } else if (index == 3) {
        start = end.addMonths(-2).addDays(1);
    } else if (index == 4) {
        start = end.addMonths(-3).addDays(1);
    } else if (index == 5) {
        start = end.addMonths(-6).addDays(1);
    } else if (index == 6) {
        start = end.addYears(-1).addDays(1);
    } else if (index == 7) { // Everything
        start = p_profile->FirstDay();
    }

    if (start < p_profile->FirstDay()) { start = p_profile->FirstDay(); }

    setRange(start, end);
}
void Overview::setRange(QDate start, QDate end)
{
    ui->dateEnd->blockSignals(true);
    ui->dateStart->blockSignals(true);
    ui->dateStart->setMaximumDate(end);
    ui->dateEnd->setMinimumDate(start);
    ui->dateStart->setDate(start);
    ui->dateEnd->setDate(end);
    ui->dateEnd->blockSignals(false);
    ui->dateStart->blockSignals(false);
    this->on_toolButton_clicked();
    updateGraphCombo();

}

void Overview::on_graphCombo_activated(int index)
{
    if (index < 0) {
        return;
    }

    gGraph *g;
    QString s;
    s = ui->graphCombo->currentText();
    bool b = !ui->graphCombo->itemData(index, Qt::UserRole).toBool();
    ui->graphCombo->setItemData(index, b, Qt::UserRole);

    if (b) {
        ui->graphCombo->setItemIcon(index, *icon_on);
    } else {
        ui->graphCombo->setItemIcon(index, *icon_off);
    }

    g = GraphView->findGraphTitle(s);
    g->setVisible(b);

    updateCube();
    GraphView->updateScale();
    GraphView->redraw();
}
void Overview::updateCube()
{
    if ((GraphView->visibleGraphs() == 0)) {
        ui->toggleVisibility->setArrowType(Qt::UpArrow);
        ui->toggleVisibility->setToolTip(tr("Show all graphs"));
        ui->toggleVisibility->blockSignals(true);
        ui->toggleVisibility->setChecked(true);
        ui->toggleVisibility->blockSignals(false);

        if (ui->graphCombo->count() > 0) {
            GraphView->setEmptyText(STR_Empty_NoGraphs);

        } else {
            GraphView->setEmptyText(STR_Empty_NoData);
        }
    } else {
        ui->toggleVisibility->setArrowType(Qt::DownArrow);
        ui->toggleVisibility->setToolTip(tr("Hide all graphs"));
        ui->toggleVisibility->blockSignals(true);
        ui->toggleVisibility->setChecked(false);
        ui->toggleVisibility->blockSignals(false);
    }
}

void Overview::on_toggleVisibility_clicked(bool checked)
{
    gGraph *g;
    QString s;
    QIcon *icon = checked ? icon_off : icon_on;

    //ui->toggleVisibility->setArrowType(checked ? Qt::UpArrow : Qt::DownArrow);
    for (int i = 0; i < ui->graphCombo->count(); i++) {
        s = ui->graphCombo->itemText(i);
        ui->graphCombo->setItemIcon(i, *icon);
        ui->graphCombo->setItemData(i, !checked, Qt::UserRole);
        g = GraphView->findGraphTitle(s);
        g->setVisible(!checked);
    }

    updateCube();
    GraphView->updateScale();
    GraphView->redraw();
}
