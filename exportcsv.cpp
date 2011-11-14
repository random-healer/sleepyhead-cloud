#include <QFileDialog>
#include <QLocale>
#include <QMessageBox>
#include "SleepLib/profiles.h"
#include "SleepLib/day.h"
#include "exportcsv.h"
#include "ui_exportcsv.h"

ExportCSV::ExportCSV(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportCSV)
{
    ui->setupUi(this);
    ui->rb1_Summary->setChecked(true);
    ui->quickRangeCombo->setCurrentIndex(0);

    // Set Date controls locale to 4 digit years
    QLocale locale=QLocale::system();
    QString shortformat=locale.dateFormat(QLocale::ShortFormat);
    if (!shortformat.toLower().contains("yyyy")) {
        shortformat.replace("yy","yyyy");
    }
    ui->startDate->setDisplayFormat(shortformat);
    ui->endDate->setDisplayFormat(shortformat);

    on_quickRangeCombo_activated("Most Recent Day");
    ui->rb1_details->clearFocus();
    ui->quickRangeCombo->setFocus();
    ui->exportButton->setEnabled(false);
}

ExportCSV::~ExportCSV()
{
    delete ui;
}

void ExportCSV::on_filenameBrowseButton_clicked()
{
    QString timestamp="SleepyHead_";

    if (ui->rb1_details->isChecked()) timestamp+="Details_";
    if (ui->rb1_Sessions->isChecked()) timestamp+="Sessions_";
    if (ui->rb1_Summary->isChecked()) timestamp+="Summary_";

    timestamp+=ui->startDate->date().toString(Qt::ISODate);
    if (ui->startDate->date()!=ui->endDate->date()) timestamp+="_"+ui->endDate->date().toString(Qt::ISODate);
    timestamp+=".csv";
    QString name=QFileDialog::getSaveFileName(this,"Select file to export to",PREF.Get("{home}/")+timestamp,"CSV Files (*.csv)");
    if (name.isEmpty()) {
        ui->exportButton->setEnabled(false);
        return;
    }
    if (!name.toLower().endsWith(".csv")) {
        name+=".csv";
    }

    ui->filenameEdit->setText(name);
    ui->exportButton->setEnabled(true);
}

void ExportCSV::on_quickRangeCombo_activated(const QString &arg1)
{
    if (arg1=="Custom") {
        ui->startDate->setEnabled(true);
        ui->endDate->setEnabled(true);
        ui->startLabel->setEnabled(true);
        ui->endLabel->setEnabled(true);
    } else {
        ui->startDate->setEnabled(false);
        ui->endDate->setEnabled(false);
        ui->startLabel->setEnabled(false);
        ui->endLabel->setEnabled(false);
        if (arg1=="Everything") {
            ui->startDate->setDate(PROFILE.FirstDay());
            ui->endDate->setDate(PROFILE.LastDay());
        } else if (arg1=="Most Recent Day") {
            ui->startDate->setDate(PROFILE.LastDay());
            ui->endDate->setDate(PROFILE.LastDay());
        } else if (arg1=="Last Week") {
            ui->startDate->setDate(QDate::currentDate().addDays(-7));
            ui->endDate->setDate(QDate::currentDate());
        } else if (arg1=="Last Fortnight") {
            ui->startDate->setDate(QDate::currentDate().addDays(-14));
            ui->endDate->setDate(QDate::currentDate());
        } else if (arg1=="Last Month") {
            ui->startDate->setDate(QDate::currentDate().addMonths(-1));
            ui->endDate->setDate(QDate::currentDate());
        } else if (arg1=="Last 6 Months") {
            ui->startDate->setDate(QDate::currentDate().addMonths(-6));
            ui->endDate->setDate(QDate::currentDate());
        } else if (arg1=="Last Year") {
            ui->startDate->setDate(QDate::currentDate().addYears(-1));
            ui->endDate->setDate(QDate::currentDate());
        }
    }
}

void ExportCSV::on_exportButton_clicked()
{
    QFile file(ui->filenameEdit->text());
    file.open(QFile::WriteOnly);
    QString header;
    const QString sep=",";
    const QString newline="\n";

    QStringList countlist,avglist,p90list;
    countlist.append(CPAP_Hypopnea);
    countlist.append(CPAP_Obstructive);
    countlist.append(CPAP_Apnea);
    countlist.append(CPAP_ClearAirway);
    countlist.append(CPAP_VSnore);
    countlist.append(CPAP_VSnore2);
    countlist.append(CPAP_RERA);
    countlist.append(CPAP_FlowLimit);
    countlist.append(CPAP_PressurePulse);

    avglist.append(CPAP_Pressure);
    avglist.append(CPAP_IPAP);
    avglist.append(CPAP_EPAP);

    p90list.append(CPAP_Pressure);
    p90list.append(CPAP_IPAP);
    p90list.append(CPAP_EPAP);

    if (ui->rb1_details->isChecked()) {
        header="DateTime"+sep+"Session"+sep+"Event"+sep+"Data/Duration";
    } else {
        if (ui->rb1_Summary->isChecked()) {
            header="Date"+sep+"Session Count"+sep+"Start"+sep+"End"+sep+"Total Time"+sep+"AHI";
        } else if (ui->rb1_Sessions->isChecked()) {
            header="Date"+sep+"Session"+sep+"Start"+sep+"End"+sep+"Total Time"+sep+"AHI";
        }
        for (int i=0;i<countlist.size();i++)
            header+=sep+countlist[i]+" Count";
        for (int i=0;i<avglist.size();i++)
            header+=sep+avglist[i]+" Avg";
        for (int i=0;i<p90list.size();i++)
            header+=sep+p90list[i]+" 90%";
    }
    header+=newline;
    file.write(header.toAscii());
    QDate date=ui->startDate->date();
    do {
        Day *day=PROFILE.GetDay(date,MT_CPAP);
        if (day) {
            QString data;
            if (ui->rb1_Summary->isChecked()) {
                QDateTime start=QDateTime::fromTime_t(day->first()/1000L);
                QDateTime end=QDateTime::fromTime_t(day->last()/1000L);
                data=date.toString(Qt::ISODate);
                data+=sep+QString::number(day->size(),10);
                data+=sep+start.toString(Qt::ISODate);
                data+=sep+end.toString(Qt::ISODate);
                data+=sep+QString::number(day->total_time()/1000L,'f',3);
                float ahi=day->count(CPAP_Obstructive)+day->count(CPAP_Hypopnea)+day->count(CPAP_Apnea)+day->count(CPAP_ClearAirway);
                ahi/=day->hours();
                data+=sep+QString::number(ahi,'f',3);
                for (int i=0;i<countlist.size();i++)
                    data+=sep+QString::number(day->count(countlist.at(i)));
                for (int i=0;i<avglist.size();i++)
                    data+=sep+QString::number(day->wavg(countlist.at(i)));
                for (int i=0;i<p90list.size();i++)
                    data+=sep+QString::number(day->p90(countlist.at(i)));
                data+=newline;
                file.write(data.toAscii());

            } else if (ui->rb1_Sessions->isChecked()) {
                for (int i=0;i<day->size();i++) {
                    Session *sess=(*day)[i];
                    QDateTime start=QDateTime::fromTime_t(sess->first()/1000L);
                    QDateTime end=QDateTime::fromTime_t(sess->last()/1000L);

                    data=date.toString(Qt::ISODate);
                    data+=sep+QString::number(sess->session(),10);
                    data+=sep+start.toString(Qt::ISODate);
                    data+=sep+end.toString(Qt::ISODate);
                    data+=sep+QString::number(sess->length()/1000.0,'f',3);
                    float ahi=sess->count(CPAP_Obstructive)+sess->count(CPAP_Hypopnea)+sess->count(CPAP_Apnea)+sess->count(CPAP_ClearAirway);
                    ahi/=sess->hours();
                    data+=sep+QString::number(ahi,'f',3);
                    for (int i=0;i<countlist.size();i++)
                        data+=sep+QString::number(sess->count(countlist.at(i)));
                    for (int i=0;i<avglist.size();i++)
                        data+=sep+QString::number(day->wavg(countlist.at(i)));
                    for (int i=0;i<p90list.size();i++)
                        data+=sep+QString::number(day->p90(countlist.at(i)));
                }
                data+=newline;
                file.write(data.toAscii());
            } else if (ui->rb1_details->isChecked()) {
                QStringList all=countlist;
                all.append(avglist);
                for (int i=0;i<day->size();i++) {
                    Session *sess=(*day)[i];
                    sess->OpenEvents();
                    QHash<ChannelID,QVector<EventList *> >::iterator fnd;
                    for (int j=0;j<all.size();j++) {
                        QString key=all.at(j);
                        fnd=sess->eventlist.find(key);
                        if (fnd!=sess->eventlist.end()) {
                            //header="DateTime"+sep+"Session"+sep+"Event"+sep+"Data/Duration";
                            for (int e=0;e<fnd.value().size();e++) {
                                EventList *ev=fnd.value()[e];
                                for (int q=0;q<ev->count();q++) {
                                    data=QDateTime::fromTime_t(ev->time(q)/1000L).toString(Qt::ISODate);
                                    data+=sep+QString::number(sess->session());
                                    data+=sep+key;
                                    data+=sep+QString::number(ev->data(q),'f',2);
                                    data+=newline;
                                    file.write(data.toAscii());
                                }
                            }
                        }
                    }
                    sess->TrashEvents();
                }
            }
        }
        date=date.addDays(1);
    } while (date<=ui->endDate->date());
    file.close();
    ExportCSV::accept();
}
