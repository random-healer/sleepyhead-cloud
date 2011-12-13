#include <QLabel>
#include <QColorDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QProcess>
#include <QDesktopServices>
#include <QFileDialog>
#include <QTextStream>
#include <QCalendarWidget>
#include "preferencesdialog.h"
#include "common_gui.h"


#include <Graphs/gGraphView.h>
#include <mainwindow.h>
#include "ui_preferencesdialog.h"
#include "SleepLib/machine_common.h"

extern QFont * defaultfont;
extern QFont * mediumfont;
extern QFont * bigfont;
extern MainWindow * mainwin;

MaskProfile masks[]={
    {"Unspecified",{{4,25},{8,25},{12,25},{16,25},{20,25}}},
    {"Nasal Pillows",{{4,20},{8,29},{12,37},{16,43},{20,49}}},
    {"Hybrid F/F Mask",{{4,20},{8,29},{12,37},{16,43},{20,49}}},
    {"Nasal Interface",{{4,20},{8,29},{12,37},{16,43},{20,49}}},
    {"Full-Face Mask",{{4,20},{8,29},{12,37},{16,43},{20,49}}},
};
const int num_masks=sizeof(masks)/sizeof(MaskProfile);


PreferencesDialog::PreferencesDialog(QWidget *parent,Profile * _profile) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    profile(_profile)
{
    ui->setupUi(this);
    ui->leakProfile->setRowCount(5);
    ui->leakProfile->setColumnCount(2);
    ui->leakProfile->horizontalHeader()->setStretchLastSection(true);
    ui->leakProfile->setColumnWidth(0,100);
    ui->maskTypeCombo->clear();

    ui->ahiGraphGroupbox->setEnabled(false);
    ui->customEventGroupbox->setEnabled(false);

    QString masktype="Nasal Pillows";
    //masktype=PROFILE["MaskType"].toString();
    for (int i=0;i<num_masks;i++) {
        ui->maskTypeCombo->addItem(masks[i].name);

        /*if (masktype==masks[i].name) {
            ui->maskTypeCombo->setCurrentIndex(i);
            on_maskTypeCombo_activated(i);
        }*/
    }
    QLocale locale=QLocale::system();
    QString shortformat=locale.dateFormat(QLocale::ShortFormat);
    if (!shortformat.toLower().contains("yyyy")) {
        shortformat.replace("yy","yyyy");
    }
    ui->startedUsingMask->setDisplayFormat(shortformat);
    Qt::DayOfWeek dow=firstDayOfWeekFromLocale();

    ui->startedUsingMask->calendarWidget()->setFirstDayOfWeek(dow);


    // Stop both calendar drop downs highlighting weekends in red
    QTextCharFormat format = ui->startedUsingMask->calendarWidget()->weekdayTextFormat(Qt::Saturday);
    format.setForeground(QBrush(Qt::black, Qt::SolidPattern));
    ui->startedUsingMask->calendarWidget()->setWeekdayTextFormat(Qt::Saturday, format);
    ui->startedUsingMask->calendarWidget()->setWeekdayTextFormat(Qt::Sunday, format);

    //ui->leakProfile->setColumnWidth(1,ui->leakProfile->width()/2);

    {
        QString filename=PROFILE.Get("{DataFolder}/ImportLocations.txt");
        QFile file(filename);
        file.open(QFile::ReadOnly);
        QTextStream textStream(&file);
        while (1) {
            QString line = textStream.readLine();
             if (line.isNull())
                 break;
             else if (line.isEmpty())
                 continue;
             else {
                 importLocations.append(line);
             }
        };
        file.close();
    }
    importModel=new QStringListModel(importLocations,this);
    ui->importListWidget->setModel(importModel);
    //ui->tabWidget->removeTab(3);

    Q_ASSERT(profile!=NULL);
    ui->tabWidget->setCurrentIndex(0);

    //i=ui->timeZoneCombo->findText((*profile)["TimeZone"].toString());
    //ui->timeZoneCombo->setCurrentIndex(i);

    bool  ok;
    double v;
    v=(*profile)["SPO2DropPercentage"].toDouble(&ok);
    if (!ok) v=3;
    ui->spo2Drop->setValue(v);
    v=(*profile)["SPO2DropDuration"].toDouble(&ok);
    if (!ok) v=10;
    ui->spo2DropTime->setValue(v);
    v=(*profile)["PulseChangeBPM"].toDouble(&ok);
    if (!ok) v=8;
    ui->pulseChange->setValue(v);
    v=(*profile)["PulseChangeDuration"].toDouble(&ok);
    if (!ok) v=5;
    ui->pulseChangeTime->setValue(v);
    v=(*profile)["OxiDiscardThreshold"].toDouble(&ok);
    if (!ok) v=10;
    ui->oxiDiscardThreshold->setValue(v);

    QTime t=(*profile)["DaySplitTime"].toTime();
    ui->timeEdit->setTime(t);
    int val;

    val=(*profile)["CombineCloserSessions"].toInt();
    ui->combineSlider->setValue(val);
    if (val>0) {
        ui->combineLCD->display(val);
    } else ui->combineLCD->display(tr("OFF"));

    val=(*profile)["IgnoreShorterSessions"].toInt();
    ui->IgnoreSlider->setValue(val);

    ui->applicationFont->setCurrentFont(QApplication::font());
    //ui->applicationFont->setFont(QApplication::font());
    ui->applicationFontSize->setValue(QApplication::font().pointSize());
    ui->applicationFontBold->setChecked(QApplication::font().weight()==QFont::Bold);
    ui->applicationFontItalic->setChecked(QApplication::font().italic());

    ui->graphFont->setCurrentFont(*defaultfont);
    //ui->graphFont->setFont(*defaultfont);
    ui->graphFontSize->setValue(defaultfont->pointSize());
    ui->graphFontBold->setChecked(defaultfont->weight()==QFont::Bold);
    ui->graphFontItalic->setChecked(defaultfont->italic());

    ui->titleFont->setCurrentFont(*mediumfont);
    //ui->titleFont->setFont(*mediumfont);
    ui->titleFontSize->setValue(mediumfont->pointSize());
    ui->titleFontBold->setChecked(mediumfont->weight()==QFont::Bold);
    ui->titleFontItalic->setChecked(mediumfont->italic());

    ui->bigFont->setCurrentFont(*bigfont);
    //ui->bigFont->setFont(*bigfont);
    ui->bigFontSize->setValue(bigfont->pointSize());
    ui->bigFontBold->setChecked(bigfont->weight()==QFont::Bold);
    ui->bigFontItalic->setChecked(bigfont->italic());

    //if (!(*profile).Exists("SkipEmptyDays")) (*profile)["SkipEmptyDays"]=true;
    //ui->skipEmptyDays->setChecked((*profile)["SkipEmptyDays"].toBool());

    general.clear();
    general["UseAntiAliasing"]=Preference(p_profile,"UseAntiAliasing",PT_Checkbox,"Use Anti-Aliasing","Enable Graphical smoothing. Doesn't always look pretty.",false);
    general["SquareWavePlots"]=Preference(p_profile,"SquareWavePlots",PT_Checkbox,"Square Wave Plots","Try to use Square Wave plots where possible",true);
    general["EnableGraphSnapshots"]=Preference(p_profile,"EnableGraphSnapshots",PT_Checkbox,"Event Breakdown Piechart","Shows Event Breakdown in Daily view. This may cause problems on older computers.",true);
    general["SkipLoginScreen"]=Preference(p_pref,"SkipLoginScreen",PT_Checkbox,"Skip Login Screen","Bypass the login screen at startup",false);
    general["SkipEmptyDays"]=Preference(p_profile,"SkipEmptyDays",PT_Checkbox,"Skip Empty Days","Skip over calendar days that don't have any data",true);
    general["EnableMultithreading"]=Preference(p_profile,"EnableMultithreading",PT_Checkbox,"Enable Multithreading","Try to use extra processor cores where possible",false);
    general["MemoryHog"]=Preference(p_profile,"MemoryHog",PT_Checkbox,"Cache Session Data","Keep session data in memory to improve load speed revisiting the date.",false);
    general["GraphHeight"]=Preference(p_profile,"GraphHeight",PT_Checkbox,"Graph Height","Default Graph Height",160);
    general["MaskDescription"]=Preference(p_profile,"MaskDescription",PT_Checkbox,"Mask Description","Whatever you want to record about your mask.",QString());
    general["HighResPrinting"]=Preference(p_profile,"HighResPrinting",PT_Checkbox,"High Resolution Printing","Use much slower but better quality high resolution printing.",QString());

    if (!(p_profile)->Exists("MaskStartDate")) {
        (PROFILE["MaskStartDate"]=PROFILE.FirstDay());
    }
    ui->startedUsingMask->setDate((*profile)["MaskStartDate"].toDate());

    if (!(p_profile)->Exists("ShowLeaksMode")) {
        PROFILE["ShowLeaksMode"]=0;
    }
    ui->leakModeCombo->setCurrentIndex((*profile)["ShowLeaksMode"].toInt());
    if (!(p_profile)->Exists("MaskType")) {
        PROFILE["MaskType"]=0;
    }
    int mt=(*profile)["MaskType"].toInt();
    ui->maskTypeCombo->setCurrentIndex(mt);
    on_maskTypeCombo_activated(mt);


    ui->maskDescription->setText(general["MaskDescription"].value().toString());

    ui->useAntiAliasing->setChecked(general["UseAntiAliasing"].value().toBool());
    ui->useSquareWavePlots->setChecked(general["SquareWavePlots"].value().toBool());
    ui->enableGraphSnapshots->setChecked(general["EnableGraphSnapshots"].value().toBool());
    ui->skipLoginScreen->setChecked(general["SkipLoginScreen"].value().toBool());
    ui->skipEmptyDays->setChecked(general["SkipEmptyDays"].value().toBool());
    ui->enableMultithreading->setChecked(general["EnableMultithreading"].value().toBool());
    ui->cacheSessionData->setChecked(general["MemoryHog"].value().toBool());
#ifdef Q_WS_MAC
    general["HighResPrinting"].setValue(true);
    ui->highResolutionPrinting->setChecked(true);
    ui->highResolutionPrinting->setEnabled(false);
#else
    ui->highResolutionPrinting->setChecked(general["HighResPrinting"].value().toBool());
#endif

    ui->graphHeight->setValue(general["GraphHeight"].value().toInt());

    if (!PREF.Exists("Updates_AutoCheck")) PREF["Updates_AutoCheck"]=true;
    ui->automaticallyCheckUpdates->setChecked(PREF["Updates_AutoCheck"].toBool());

    if (!PREF.Exists("Updates_CheckFrequency")) PREF["Updates_CheckFrequency"]=3;
    ui->updateCheckEvery->setValue(PREF["Updates_CheckFrequency"].toInt());
    if (PREF.Exists("Updates_LastChecked")) {
        RefreshLastChecked();
    } else ui->updateLastChecked->setText("Never");

    if (val>0) {
        ui->IgnoreLCD->display(val);
    } else ui->IgnoreLCD->display(tr("OFF"));

    ui->overlayFlagsCombo->setCurrentIndex((*profile)["AlwaysShowOverlayBars"].toInt());

    //ui->memoryHogCheckbox->setChecked((*profile)["MemoryHog"].toBool());

    //ui->intentionalLeakEdit->setValue((*profile)["IntentionalLeak"].toDouble());
    //ui->useMultithreading->setChecked((*profile)["EnableMultithreading"].toBool());

    ui->oximetryGroupBox->setChecked((*profile)["EnableOximetry"].toBool());
    ui->oximetrySync->setChecked((*profile)["SyncOximetry"].toBool());
    int ot=ui->oximetryType->findText((*profile)["OximeterType"].toString(),Qt::MatchExactly);
    if (ot<0) ot=0;
    ui->oximetryType->setCurrentIndex(ot);

    ui->eventTable->setColumnWidth(0,40);
    ui->eventTable->setColumnWidth(1,55);
    ui->eventTable->setColumnHidden(3,true);
    int row=0;
    QTableWidgetItem *item;
    QHash<QString, schema::Channel *>::iterator ci;
    for (ci=schema::channel.names.begin();ci!=schema::channel.names.end();ci++) {
        if (ci.value()->type()==schema::DATA) {
            ui->eventTable->insertRow(row);
            int id=ci.value()->id();
            ui->eventTable->setItem(row,3,new QTableWidgetItem(QString::number(id)));
            item=new QTableWidgetItem(ci.value()->description());
            ui->eventTable->setItem(row,2,item);
            QCheckBox *c=new QCheckBox(ui->eventTable);
            c->setChecked(true);
            QLabel *pb=new QLabel(ui->eventTable);
            pb->setText("foo");
            ui->eventTable->setCellWidget(row,0,c);
            ui->eventTable->setCellWidget(row,1,pb);


            QColor a=ci.value()->defaultColor();//(rand() % 255, rand() % 255, rand() % 255, 255);
            QPalette p(a,a,a,a,a,a,a);

            pb->setPalette(p);
            pb->setAutoFillBackground(true);
            pb->setBackgroundRole(QPalette::Background);
            row++;
        }
    }
/*    QLocale locale=QLocale::system();
    QString shortformat=locale.dateFormat(QLocale::ShortFormat);
    if (!shortformat.toLower().contains("yyyy")) {
        shortformat.replace("yy","yyyy");
    }*/

    graphFilterModel=new MySortFilterProxyModel(this);
    graphModel=new QStandardItemModel(this);
    graphFilterModel->setSourceModel(graphModel);
    graphFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    graphFilterModel->setFilterKeyColumn(0);
    ui->graphView->setModel(graphFilterModel);

    resetGraphModel();
//    tree->sortByColumn(0,Qt::AscendingOrder);
}


PreferencesDialog::~PreferencesDialog()
{
    disconnect(graphModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(graphModel_changed(QStandardItem*)));
    delete ui;
}

void PreferencesDialog::on_eventTable_doubleClicked(const QModelIndex &index)
{
    int row=index.row();
    int col=index.column();
    bool ok;
    int id=ui->eventTable->item(row,3)->text().toInt(&ok);
    if (col==1) {
        QWidget *w=ui->eventTable->cellWidget(row,col);
        QColorDialog a;
        QColor color=w->palette().background().color();
        a.setCurrentColor(color);
        if (a.exec()==QColorDialog::Accepted) {
            QColor c=a.currentColor();
            QPalette p(c,c,c,c,c,c,c);
            w->setPalette(p);
            m_new_colors[id]=c;
            //qDebug() << "Color accepted" << col << id;
        }
    }
}

void PreferencesDialog::Save()
{
    bool needs_restart=false;

    general["UseAntiAliasing"].setValue(ui->useAntiAliasing->isChecked());
    if (ui->useSquareWavePlots->isChecked()!=general["SquareWavePlots"].value().toBool()) {
        general["SquareWavePlots"].setValue(ui->useSquareWavePlots->isChecked());
        needs_restart=true;
    }
    general["EnableGraphSnapshots"].setValue(ui->enableGraphSnapshots->isChecked());
    general["SkipLoginScreen"].setValue(ui->skipLoginScreen->isChecked());
    general["SkipEmptyDays"].setValue(ui->skipEmptyDays->isChecked());
    general["EnableMultithreading"].setValue(ui->enableMultithreading->isChecked());
    general["MemoryHog"].setValue(ui->cacheSessionData->isChecked());
    general["MaskDescription"].setValue(ui->maskDescription->text());
    general["HighResPrinting"].setValue(ui->highResolutionPrinting->isChecked());

    (*profile)["MaskStartDate"]=ui->startedUsingMask->date();
    (*profile)["GraphHeight"]=ui->graphHeight->value();

    if (((*profile)["DaySplitTime"].toTime()!=ui->timeEdit->time()) ||
    ((*profile)["CombineCloserSessions"].toInt()!=ui->combineSlider->value()) ||
    ((*profile)["IgnoreShorterSessions"].toInt()!=ui->IgnoreSlider->value())) {
        PROFILE["TrashDayCache"]=true;
        needs_restart=true;
    } else PROFILE["TrashDayCache"]=false;

    (*profile)["CombineCloserSessions"]=ui->combineSlider->value();
    (*profile)["IgnoreShorterSessions"]=ui->IgnoreSlider->value();
    (*profile)["DaySplitTime"]=ui->timeEdit->time();

    (*profile)["AlwaysShowOverlayBars"]=ui->overlayFlagsCombo->currentIndex();
    (*profile)["ShowLeaksMode"]=ui->leakModeCombo->currentIndex();
    (*profile)["MaskType"]=ui->maskTypeCombo->currentIndex();
    //(*profile)["UseAntiAliasing"]=ui->genOpWidget->item(0)->checkState()==Qt::Checked;
    //(*profile)["MemoryHog"]=ui->memoryHogCheckbox->isChecked();
    //(*profile)["EnableGraphSnapshots"]=ui->genOpWidget->item(2)->checkState()==Qt::Checked;


    //(*profile)["IntentionalLeak"]=ui->intentionalLeakEdit->value();
    //(*profile)["EnableMultithreading"]=ui->useMultithreading->isChecked();
    (*profile)["EnableOximetry"]=ui->oximetryGroupBox->isChecked();
    (*profile)["SyncOximetry"]=ui->oximetrySync->isChecked();
    int oxigrp=ui->oximetrySync->isChecked() ? 0 : 1;
    gGraphView *gv=mainwin->getDaily()->graphView();
    gGraph *g=gv->findGraph("Pulse");
    if (g) {
        g->setGroup(oxigrp);
    }
    g=gv->findGraph("SpO2");
    if (g) {
        g->setGroup(oxigrp);
    }
    g=gv->findGraph("Plethy");
    if (g) {
        g->setGroup(oxigrp);
    }

    (*profile)["OximeterType"]=ui->oximetryType->currentText();

    (*profile)["SPO2DropPercentage"]=ui->spo2Drop->value();
    (*profile)["SPO2DropDuration"]=ui->spo2DropTime->value();
    (*profile)["PulseChangeBPM"]=ui->pulseChange->value();
    (*profile)["PulseChangeDuration"]=ui->pulseChangeTime->value();
    (*profile)["OxiDiscardThreshold"]=ui->oxiDiscardThreshold->value();

    //PREF["SkipLoginScreen"]=ui->skipLoginScreen->isChecked();

    //ui->genOpWidget->item(1)->checkState()==Qt::Checked ? true : false;
    //if ((ui->genOpWidget->item(1)->checkState()==Qt::Checked) != (*profile)["SquareWavePlots"].toBool()) {
    //needs_restart=true;
    //}
    //(*profile)["SquareWavePlots"]=ui->genOpWidget->item(1)->checkState()==Qt::Checked;

    //(*profile)["SkipEmptyDays"]=ui->skipEmptyDays->isChecked();
    PREF["Updates_AutoCheck"]=ui->automaticallyCheckUpdates->isChecked();
    PREF["Updates_CheckFrequency"]=ui->updateCheckEvery->value();

    PREF["Fonts_Application_Name"]=ui->applicationFont->currentText();
    PREF["Fonts_Application_Size"]=ui->applicationFontSize->value();
    PREF["Fonts_Application_Bold"]=ui->applicationFontBold->isChecked();
    PREF["Fonts_Application_Italic"]=ui->applicationFontItalic->isChecked();

    PREF["Fonts_Graph_Name"]=ui->graphFont->currentText();
    PREF["Fonts_Graph_Size"]=ui->graphFontSize->value();
    PREF["Fonts_Graph_Bold"]=ui->graphFontBold->isChecked();
    PREF["Fonts_Graph_Italic"]=ui->graphFontItalic->isChecked();

    PREF["Fonts_Title_Name"]=ui->titleFont->currentText();
    PREF["Fonts_Title_Size"]=ui->titleFontSize->value();
    PREF["Fonts_Title_Bold"]=ui->titleFontBold->isChecked();
    PREF["Fonts_Title_Italic"]=ui->titleFontItalic->isChecked();

    PREF["Fonts_Big_Name"]=ui->bigFont->currentText();
    PREF["Fonts_Big_Size"]=ui->bigFontSize->value();
    PREF["Fonts_Big_Bold"]=ui->bigFontBold->isChecked();
    PREF["Fonts_Big_Italic"]=ui->bigFontItalic->isChecked();

    QFont font=ui->applicationFont->currentFont();
    font.setPointSize(ui->applicationFontSize->value());
    font.setWeight(ui->applicationFontBold->isChecked()?QFont::Bold : QFont::Normal);
    font.setItalic(ui->applicationFontItalic->isChecked());
    QApplication::setFont(font);

    *defaultfont=ui->graphFont->currentFont();
    defaultfont->setPointSize(ui->graphFontSize->value());
    defaultfont->setWeight(ui->graphFontBold->isChecked()?QFont::Bold : QFont::Normal);
    defaultfont->setItalic(ui->graphFontItalic->isChecked());

    *mediumfont=ui->titleFont->currentFont();
    mediumfont->setPointSize(ui->titleFontSize->value());
    mediumfont->setWeight(ui->titleFontBold->isChecked()?QFont::Bold : QFont::Normal);
    mediumfont->setItalic(ui->titleFontItalic->isChecked());

    *bigfont=ui->bigFont->currentFont();
    bigfont->setPointSize(ui->bigFontSize->value());
    bigfont->setWeight(ui->bigFontBold->isChecked()?QFont::Bold : QFont::Normal);
    bigfont->setItalic(ui->bigFontItalic->isChecked());

    // Process color changes
    for (QHash<int,QColor>::iterator i=m_new_colors.begin();i!=m_new_colors.end();i++) {
        schema::Channel &chan=schema::channel[i.key()];
        if (!chan.isNull()) {
            qDebug() << "TODO: Change" << chan.name() << "color to" << i.value();
            chan.setDefaultColor(i.value());
        }
    }

    //qDebug() << "TODO: Save channels.xml to update channel data";

    {
        QString filename=PROFILE.Get("{DataFolder}/ImportLocations.txt");
        QFile file(filename);
        file.open(QFile::WriteOnly);
        QTextStream ts(&file);
        for (int i=0;i<importLocations.size();i++) {
            ts << importLocations[i] << endl;
            //file.write(importLocations[i].toUtf8());
        }
        file.close();
    }

    //PROFILE.Save();
    //PREF.Save();

    if (needs_restart) {
        if (QMessageBox::question(this,"Restart Required","One or more of the changes you have made will require this application to be restarted, in order for these changes to come into effect.\nWould you like do this now?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes) {
            mainwin->RestartApplication();
        }
    }
}

void PreferencesDialog::on_combineSlider_valueChanged(int position)
{
    if (position>0) {
        ui->combineLCD->display(position);
    } else ui->combineLCD->display(tr("OFF"));
}

void PreferencesDialog::on_IgnoreSlider_valueChanged(int position)
{
    if (position>0) {
        ui->IgnoreLCD->display(position);
    } else ui->IgnoreLCD->display(tr("OFF"));
}

#include "mainwindow.h"
extern MainWindow * mainwin;
void PreferencesDialog::RefreshLastChecked()
{
    ui->updateLastChecked->setText(PREF["Updates_LastChecked"].toDateTime().toString(Qt::SystemLocaleLongDate));
}

void PreferencesDialog::on_checkForUpdatesButton_clicked()
{
    mainwin->statusBar()->showMessage("Checking for Updates");
    ui->updateLastChecked->setText("Checking for Updates");
    mainwin->CheckForUpdates();
}

void PreferencesDialog::on_addImportLocation_clicked()
{
    QString dir=QFileDialog::getExistingDirectory(this,"Add this Location to the Import List","",QFileDialog::ShowDirsOnly);

    if (!dir.isEmpty()) {
        if (!importLocations.contains(dir)) {
            importLocations.append(dir);
            importModel->setStringList(importLocations);
        }
    }
}

void PreferencesDialog::on_removeImportLocation_clicked()
{
    if (ui->importListWidget->currentIndex().isValid()) {
        QString dir=ui->importListWidget->currentIndex().data().toString();
        importModel->removeRow(ui->importListWidget->currentIndex().row());
        importLocations.removeAll(dir);
    }
}


void PreferencesDialog::on_graphView_activated(const QModelIndex &index)
{
    QString a=index.data().toString();
    qDebug() << "Could do something here with" << a;
}

void PreferencesDialog::on_graphFilter_textChanged(const QString &arg1)
{
    graphFilterModel->setFilterFixedString(arg1);
}


MySortFilterProxyModel::MySortFilterProxyModel(QObject *parent)
    :QSortFilterProxyModel(parent)
{

}

bool MySortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (source_parent == qobject_cast<QStandardItemModel*>(sourceModel())->invisibleRootItem()->index()) {
         // always accept children of rootitem, since we want to filter their children
         return true;
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

void PreferencesDialog::graphModel_changed(QStandardItem * item)
{
    QModelIndex index=item->index();

    gGraphView *gv=NULL;
    bool ok;

    const QModelIndex & row=index.sibling(index.row(),0);
    bool checked=row.data(Qt::CheckStateRole)!=0;
    //QString name=row.data().toString();

    int group=row.data(Qt::UserRole+1).toInt();
    int id=row.data(Qt::UserRole+2).toInt();
    switch(group) {
        case 0: gv=mainwin->getDaily()->graphView(); break;
        case 1: gv=mainwin->getOverview()->graphView(); break;
        case 2: gv=mainwin->getOximetry()->graphView(); break;
    default: ;
    }

    if (!gv)
        return;

    gGraph *graph=(*gv)[id];
    if (!graph)
        return;

    if (graph->visible()!=checked) {
        graph->setVisible(checked);
    }

    EventDataType val;

    if (index.column()==1) {
        val=index.data().toDouble(&ok);

        if (!ok) {
            graphModel->setData(index,QString::number(graph->rec_miny,'f',1));
            ui->graphView->update();
        }  else {
            //if ((val < graph->rec_maxy) || (val==0)) {
                graph->setRecMinY(val);
            /*} else {
                graphModel->setData(index,QString::number(graph->rec_miny,'f',1));
                ui->graphView->update();
             } */
        }
    } else if (index.column()==2) {
        val=index.data().toDouble(&ok);
        if (!ok) {
            graphModel->setData(index,QString::number(graph->rec_maxy,'f',1));
            ui->graphView->update();
        }  else {
            //if ((val > graph->rec_miny) || (val==0)) {
                graph->setRecMaxY(val);
            /*} else {
                graphModel->setData(index,QString::number(graph->rec_maxy,'f',1));
                ui->graphView->update();
            }*/
        }

    }
    gv->updateScale();
//    qDebug() << name << checked;
}

void PreferencesDialog::resetGraphModel()
{

    graphModel->clear();
    QStandardItem *daily=new QStandardItem("Daily Graphs");
    QStandardItem *overview=new QStandardItem("Overview Graphs");
    daily->setEditable(false);
    overview->setEditable(false);

    graphModel->appendRow(daily);
    graphModel->appendRow(overview);

    ui->graphView->setAlternatingRowColors(true);

    // ui->graphView->setFirstColumnSpanned(0,daily->index(),true); // Crashes on windows.. Why do I need this again?
    graphModel->setColumnCount(3);
    QStringList headers;
    headers.append("Graph");
    headers.append("Min");
    headers.append("Max");
    graphModel->setHorizontalHeaderLabels(headers);
    ui->graphView->setColumnWidth(0,250);
    ui->graphView->setColumnWidth(1,50);
    ui->graphView->setColumnWidth(2,50);

    gGraphView *gv=mainwin->getDaily()->graphView();
    for (int i=0;i<gv->size();i++) {
        QList<QStandardItem *> items;
        QString title=(*gv)[i]->title();
        QStandardItem *it=new QStandardItem(title);
        it->setCheckable(true);
        it->setCheckState((*gv)[i]->visible() ? Qt::Checked : Qt::Unchecked);
        it->setEditable(false);
        it->setData(0,Qt::UserRole+1);
        it->setData(i,Qt::UserRole+2);
        items.push_back(it);

        if (title!="Event Flags") {

            it=new QStandardItem(QString::number((*gv)[i]->rec_miny,'f',1));
            it->setEditable(true);
            items.push_back(it);

            it=new QStandardItem(QString::number((*gv)[i]->rec_maxy,'f',1));
            it->setEditable(true);
            items.push_back(it);
        } else {
            it=new QStandardItem(tr("N/A")); // not applicable.
            it->setEditable(false);
            items.push_back(it);
            items.push_back(it->clone());
        }

        daily->insertRow(i,items);
    }

    gv=mainwin->getOverview()->graphView();
    for (int i=0;i<gv->size();i++) {
        QList<QStandardItem *> items;
        QStandardItem *it=new QStandardItem((*gv)[i]->title());
        it->setCheckable(true);
        it->setCheckState((*gv)[i]->visible() ? Qt::Checked : Qt::Unchecked);
        it->setEditable(false);
        items.push_back(it);
        it->setData(1,Qt::UserRole+1);
        it->setData(i,Qt::UserRole+2);

        it=new QStandardItem(QString::number((*gv)[i]->rec_miny,'f',1));
        it->setEditable(true);
        items.push_back(it);

        it=new QStandardItem(QString::number((*gv)[i]->rec_maxy,'f',1));
        it->setEditable(true);
        items.push_back(it);

        overview->insertRow(i,items);
    }
    if (mainwin->getOximetry()) {
        QStandardItem *oximetry=new QStandardItem("Oximetry Graphs");
        graphModel->appendRow(oximetry);
        oximetry->setEditable(false);
        gv=mainwin->getOximetry()->graphView();
        for (int i=0;i<gv->size();i++) {
            QList<QStandardItem *> items;
            QStandardItem *it=new QStandardItem((*gv)[i]->title());
            it->setCheckable(true);
            it->setCheckState((*gv)[i]->visible() ? Qt::Checked : Qt::Unchecked);
            it->setEditable(false);
            it->setData(2,Qt::UserRole+1);
            it->setData(i,Qt::UserRole+2);
            items.push_back(it);

            it=new QStandardItem(QString::number((*gv)[i]->rec_miny,'f',1));
            it->setEditable(true);
            items.push_back(it);

            it=new QStandardItem(QString::number((*gv)[i]->rec_maxy,'f',1));
            it->setEditable(true);
            items.push_back(it);

            oximetry->insertRow(i,items);
        }
    }
    connect(graphModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(graphModel_changed(QStandardItem*)));

    ui->graphView->expandAll();
}

void PreferencesDialog::on_resetGraphButton_clicked()
{
    if (QMessageBox::question(this,"Confirmation","Are you sure you want to reset your graph preferences to the defaults?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes) {
        gGraphView *gv[3];
        gv[0]=mainwin->getDaily()->graphView();
        gv[1]=mainwin->getOverview()->graphView();
        gv[2]=mainwin->getOximetry()->graphView();
        for (int j=0;j<3;j++) {
            if (gv[j]!=NULL) {
                for (int i=0;i<gv[j]->size();i++) {
                    gGraph *g=(*(gv[j]))[i];
                    g->setRecMaxY(0);
                    g->setRecMinY(0);
                    g->setVisible(true);
                }
                gv[j]->updateScale();
            }
        }
        resetGraphModel();
        ui->graphView->update();
    }
 }


/*void PreferencesDialog::on_genOpWidget_itemActivated(QListWidgetItem *item)
{
    item->setCheckState(item->checkState()==Qt::Checked ? Qt::Unchecked : Qt::Checked);
}  */


void PreferencesDialog::on_maskTypeCombo_activated(int index)
{
    if (index<num_masks) {
        QTableWidgetItem *item;
        for (int i=0;i<5;i++) {
            MaskProfile & mp=masks[index];

            item=ui->leakProfile->item(i,0);
            QString val=QString::number(mp.pflow[i][0],'f',2);
            if (!item) {
                item=new QTableWidgetItem(val);
                ui->leakProfile->setItem(i,0,item);
            } else item->setText(val);

            val=QString::number(mp.pflow[i][1],'f',2);
            item=ui->leakProfile->item(i,1);
            if (!item) {
                item=new QTableWidgetItem(val);
                ui->leakProfile->setItem(i,1,item);
            } else item->setText(val);
        }
    }
}
